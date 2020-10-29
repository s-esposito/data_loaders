#include <thread>
#include <unordered_map>
#include <vector>
#include <atomic>



//eigen 
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

//readerwriterqueue
// #include "readerwriterqueue/readerwriterqueue.h"

//boost
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>



namespace radu { namespace utils{
    class RandGenerator;
}}

namespace easy_pbr{
    class Frame;
}
// class DataTransformer;


class DataLoaderShapeNetImg
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    DataLoaderShapeNetImg(const std::string config_file);
    ~DataLoaderShapeNetImg();
    std::shared_ptr<easy_pbr::Frame> get_random_frame();
    void start_reading_next_scene(); //switch to another scene from this object and start reading it
    bool finished_reading_scene(); //returns true when we have finished reading everything for that one scene of the corresponding object and we can safely use get_random_frame
    void reset(); //starts reading from the beggining
    int nr_scenes(); //returns the number of scenes for the object that we selected
    std::string get_object_name();
    void set_object_name(const std::string object_name);




private:

    void init_params(const std::string config_file);
    void init_data_reading(); //after the parameters this uses the params to initiate all the structures needed for the susequent read_data
    void read_scene(const std::string scene_path); //a path to the scene which contains all the  images and the pose and so on
    std::unordered_map<std::string, std::string> create_mapping_classnr2classname(); //create the mapping between the weird nr of a class to the actual class name
    

    //objects
    std::shared_ptr<radu::utils::RandGenerator> m_rand_gen;
    // std::shared_ptr<DataTransformer> m_transformer;

    //params
    // bool m_autostart;
    std::atomic<bool> m_is_running;// if the loop of loading is running, it is used to break the loop when the user ctrl-c
    // std::string m_mode; // train or test or val
    bool m_shuffle;
    bool m_do_overfit; // return all the time just images from the the first scene of that specified object class
    std::string m_restrict_to_object;  //makes it load clouds only from a specific object
    boost::filesystem::path m_dataset_path;  //get the path where all the off files are 
    std::thread m_loader_thread;
    int m_nr_resets;
    int m_idx_scene_to_read;


    //internal
    std::vector<boost::filesystem::path> m_scene_folders; //contains all the folders of the scenes for this objects
    std::vector< std::shared_ptr<easy_pbr::Frame> > m_frames_for_scene;

};