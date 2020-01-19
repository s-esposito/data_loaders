#include "data_loaders/DataLoaderVolRef.h"

//c++
#include <algorithm>
#include <random>

//loguru
#define LOGURU_REPLACE_GLOG 1
#include <loguru.hpp>

//configuru
#define CONFIGURU_WITH_EIGEN 1
#define CONFIGURU_IMPLICIT_CONVERSIONS 1
#include <configuru.hpp>
using namespace configuru;

//boost
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


//my stuff 
#include "RandGenerator.h"

// using namespace easy_pbr::utils;


DataLoaderVolRef::DataLoaderVolRef(const std::string config_file):
    m_is_modified(false),
    m_is_running(false),
    m_frames_color_buffer(BUFFER_SIZE),
    m_frames_depth_buffer(BUFFER_SIZE),
    m_idx_sample_to_read(0),
    m_nr_resets(0),
    m_rand_gen(new RandGenerator)
{

    init_params(config_file);
    if(m_autostart){
        m_is_running=true;
        m_loader_thread=std::thread(&DataLoaderVolRef::read_data, this);  //starts the spin in another thread
    }

}

DataLoaderVolRef::~DataLoaderVolRef(){

    m_is_running=false;

    m_loader_thread.join();
}

void DataLoaderVolRef::init_params(const std::string config_file){
  
    //read all the parameters
    // Config cfg = configuru::parse_file(std::string(CMAKE_SOURCE_DIR)+"/config/"+config_file, CFG);
    std::string config_file_abs;
    if (fs::path(config_file).is_relative()){
        config_file_abs=(fs::path(PROJECT_SOURCE_DIR) / config_file).string();
    }else{
        config_file_abs=config_file;
    }
    Config cfg = configuru::parse_file(config_file_abs, CFG);

    Config loader_config=cfg["loader_vol_ref"];
    m_autostart=loader_config["autostart"];
    m_nr_samples_to_skip=loader_config["nr_samples_to_skip"];
    m_nr_samples_to_read=loader_config["nr_samples_to_read"];
    m_shuffle=loader_config["shuffle"];
    m_do_overfit=loader_config["do_overfit"];
    m_dataset_path=(std::string)loader_config["dataset_path"];

}

void DataLoaderVolRef::start(){
    CHECK(m_is_running==false) << "The loader thread is already running. Please check in the config file that autostart is not already set to true. Or just don't call start()";

    init_data_reading();

    m_is_running=true;
    m_loader_thread=std::thread(&DataLoaderVolRef::read_data, this);  //starts the spin in another thread
}

void DataLoaderVolRef::init_data_reading(){

    std::vector<fs::path> samples_filenames_all;
    

    if(!fs::is_directory(m_dataset_path)) {
        LOG(FATAL) << "No directory " << m_dataset_path;
    }
    fs::path dataset_path_full=m_dataset_path;
    for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(dataset_path_full), {})){
        //grab only the color png images
        bool found_color=entry.path().stem().string().find("color")!= std::string::npos;
        bool found_png=entry.path().stem().string().find("png")!= std::string::npos;
        // VLOG(1) << "entry is " << entry.path().stem().string() << " has found color " << found_color;
        // VLOG(1) << "entry is " << entry.path().stem().string() << " has found png " << found_png;
        if( entry.path().stem().string().find("color")!= std::string::npos && entry.path().stem().string().find("frame")!= std::string::npos  ){ 
            samples_filenames_all.push_back(entry);
        }
    }

    if(m_shuffle){
        unsigned seed = m_nr_resets;
        auto rng = std::default_random_engine(seed);
        std::shuffle(std::begin(samples_filenames_all), std::end(samples_filenames_all), rng);
    }else{
        std::sort(samples_filenames_all.begin(), samples_filenames_all.end());
    }

  



    //ADDS THE samples to the member std_vector of paths 
    //read a maximum nr of images HAVE TO DO IT HERE BECAUSE WE HAVE TO SORT THEM FIRST
    for (size_t i = 0; i <samples_filenames_all.size(); i++) {
        if( (int)i>=m_nr_samples_to_skip && ((int)m_samples_filenames.size()<m_nr_samples_to_read || m_nr_samples_to_read<0 ) ){
            m_samples_filenames.push_back(samples_filenames_all[i]);
        }
    }

    std::cout << "About to read " << m_samples_filenames.size() << " samples" <<std::endl; 


    CHECK(m_samples_filenames.size()>0) <<"We did not find any samples files to read";


    //read the intrinsics for color and depth
    m_K_color.setIdentity(); 
    std::string intrinsics_color=(m_dataset_path/"colorIntrinsics.txt").string();
    m_K_color=read_intrinsics_file(intrinsics_color);
    m_K_depth.setIdentity(); 
    std::string intrinsics_depth=(m_dataset_path/"depthIntrinsics.txt").string();
    m_K_depth=read_intrinsics_file(intrinsics_depth);



}

void DataLoaderVolRef::read_data(){

    loguru::set_thread_name("loader_thread_vol_ref");


    while (m_is_running ) {

        //we finished reading so we wait here for a reset
        if(m_idx_sample_to_read>=m_samples_filenames.size()){
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            continue;
        }

        // std::cout << "size approx is " << m_queue.size_approx() << '\n';
        // std::cout << "m_idx_img_to_read is " << m_idx_img_to_read << '\n';
        if(m_frames_color_buffer.size_approx()<BUFFER_SIZE-1){ //there is enough space
            //read the frame and everything else and push it to the queue

            fs::path sample_filename=m_samples_filenames[ m_idx_sample_to_read ];
            if(!m_do_overfit){
                m_idx_sample_to_read++;
            }
            // VLOG(1) << "reading " << sample_filename;


            //read color img
            Frame frame_color;
            frame_color.rgb_8u=cv::imread(sample_filename.string());
            frame_color.rgb_8u.convertTo(frame_color.rgb_32f, CV_32FC3, 1.0/255.0);
            frame_color.width=frame_color.rgb_32f.cols;
            frame_color.height=frame_color.rgb_32f.rows;

            //read depth
            Frame frame_depth;
            std::string name = sample_filename.string().substr(0, sample_filename.string().size()-9); //removes the last 5 characters corresponding to "color"
            cv::Mat depth=cv::imread(name+"depth.png", CV_LOAD_IMAGE_ANYDEPTH);
            depth.convertTo(frame_depth.depth, CV_32FC1, 1.0/1000.0); //the depth was stored in mm but we want it in meters
            // depth.convertTo(frame_depth.depth, CV_32FC1 ); //the depth was stored in mm but we want it in meters
            frame_depth.width=frame_depth.depth.cols;
            frame_depth.height=frame_depth.depth.rows;

            //read pose file
            std::string pose_file=name+"pose.txt";
            Eigen::Affine3d tf_world_cam=read_pose_file(pose_file);
            // VLOG(1) << "pose from tf_world_cam" << pose_file << " is " << tf_world_cam.matrix();
            // frame_color.tf_cam_world=tf_world_cam.inverse().cast<float>();
            // frame_depth.tf_cam_world=tf_world_cam.inverse().cast<float>();
            // frame_color.tf_cam_world=tf_world_cam.cast<float>();
            // frame_depth.tf_cam_world=tf_world_cam.cast<float>();

            Eigen::Affine3d m_tf_worldGL_world;
            m_tf_worldGL_world.setIdentity();
            Eigen::Matrix3d worldGL_world_rot;
            worldGL_world_rot = Eigen::AngleAxisd(1.0*M_PI, Eigen::Vector3d::UnitX());
            m_tf_worldGL_world.matrix().block<3,3>(0,0)=worldGL_world_rot;
            frame_color.tf_cam_world= tf_world_cam.cast<float>().inverse() * m_tf_worldGL_world.cast<float>().inverse(); //from worldgl to world ros, from world ros to cam 
            frame_depth.tf_cam_world= tf_world_cam.cast<float>().inverse() * m_tf_worldGL_world.cast<float>().inverse(); //from worldgl to world ros, from world ros to cam 

            //assign K matrix
            frame_color.K=m_K_color.cast<float>();
            frame_depth.K=m_K_depth.cast<float>();

            // VLOG(1) << "frame color has K " << frame_color.K;



            m_frames_color_buffer.enqueue(frame_color);
            m_frames_depth_buffer.enqueue(frame_depth);

        }

    }

}

Eigen::Affine3d DataLoaderVolRef::read_pose_file(std::string pose_file){
    std::ifstream infile( pose_file );
    if(!infile.is_open()){
        LOG(FATAL) << "Could not open pose file " << pose_file;
    }
    int line_read=0;
    std::string line;
    Eigen::Affine3d pose; 
    pose.setIdentity();
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        iss >>  pose.matrix()(line_read,0)>> pose.matrix()(line_read,1)>> pose.matrix()(line_read,2)>> pose.matrix()(line_read,3);
        line_read++;
    }

    return pose;
}

Eigen::Matrix3d DataLoaderVolRef::read_intrinsics_file(std::string intrinsics_file){
    std::ifstream infile( intrinsics_file );
    if(!infile.is_open()){
        LOG(FATAL) << "Could not open intrinsics file " << intrinsics_file;
    }
    int line_read=0;
    std::string line;
    Eigen::Matrix3d K; 
    K.setIdentity();
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        iss >>  K.matrix()(line_read,0) >> K.matrix()(line_read,1) >> K.matrix()(line_read,2);
        line_read++;
        if(line_read>=3){
            break;
        }
    }

    return K;
} 


bool DataLoaderVolRef::has_data(){
    if(m_frames_color_buffer.peek()==nullptr || m_frames_depth_buffer.peek()==nullptr){
        return false;
    }else{
        return true;
    }
}


Frame DataLoaderVolRef::get_color_frame(){

    Frame frame;
    m_frames_color_buffer.try_dequeue(frame);

    return frame;
}

Frame DataLoaderVolRef::get_depth_frame(){

    Frame frame;
    m_frames_depth_buffer.try_dequeue(frame);

    return frame;
}


bool DataLoaderVolRef::is_finished(){
    //check if this loader has loaded everything
    if(m_idx_sample_to_read<(int)m_samples_filenames.size()){
        return false; //there is still more files to read
    }

    //check that there is nothing in the ring buffers
    if(m_frames_color_buffer.peek()!=nullptr || m_frames_depth_buffer.peek()!=nullptr){
        return false; //there is still something in the buffer
    }

    return true; //there is nothing more to read and nothing more in the buffer so we are finished

}


bool DataLoaderVolRef::is_finished_reading(){
    //check if this loader has loaded everything
    if(m_idx_sample_to_read<(int)m_samples_filenames.size()){
        return false; //there is still more files to read
    }

    return true; //there is nothing more to read and so we are finished reading

}

void DataLoaderVolRef::reset(){
    m_nr_resets++;
    // we shuffle again the data so as to have freshly shuffled data for the next epoch
    if(m_shuffle){
        // unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        // auto rng = std::default_random_engine(seed);
        unsigned seed = m_nr_resets;
        auto rng = std::default_random_engine(seed);
        std::shuffle(std::begin(m_samples_filenames), std::end(m_samples_filenames), rng);
    }

    m_idx_sample_to_read=0;
}

int DataLoaderVolRef::nr_samples(){
    return m_samples_filenames.size();
}

