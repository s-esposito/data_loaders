#include <thread>
#include <unordered_map>
#include <vector>

//ros
#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_ros/point_cloud.h>

//eigen 
#include <Eigen/Core>
#include<Eigen/StdVector>

//readerwriterqueue
#include "readerwriterqueue/readerwriterqueue.h"

//boost
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "data_loaders/core/MeshCore.h"

#define BUFFER_SIZE 5 //clouds are stored in a queue until they are acessed, the queue stores a maximum of X items

class LabelMngr;
class RandGenerator;
class DataTransformer;

class DataLoaderSemanticKitti
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    DataLoaderSemanticKitti(const std::string config_file);
    ~DataLoaderSemanticKitti();
    void start(); //starts the thread that reads the data from disk. This gets called automatically if we have autostart=true
    MeshCore get_cloud();
    bool has_data();
    bool is_finished(); //returns true when we have finished reading AND processing everything
    bool is_finished_reading(); //returns true when we have finished reading everything but maybe not processing
    void reset(); //starts reading from the beggining
    int nr_samples(); //returns the number of samples/examples that this loader will iterate over
    void set_mode_train(); //set the loader so that it starts reading form the training set
    void set_mode_test();
    void set_mode_validation();
    void set_sequence(const std::string sequence);
    // void set_adaptive_subsampling(const bool adaptive_subsampling);


    int add(const int a, const int b );
private:

    void init_params(const std::string config_file);
    void init_data_reading(); //after the parameters this uses the params to initiate all the structures needed for the susequent read_data
    std::vector<Eigen::Affine3d,  Eigen::aligned_allocator<Eigen::Affine3d>  >read_pose_file(std::string m_pose_file);
    void read_data();
    Eigen::Affine3d get_pose_for_scan_nr_and_sequence(const int scan_nr, const std::string sequence);
    void create_transformation_matrices();
    // void apply_transform(Eigen::MatrixXd& V, const Eigen::Affine3d& trans);
    // void compute_normals(Eigen::MatrixXd& NV, const Eigen::MatrixXd& V);

    //objects 
    std::shared_ptr<RandGenerator> m_rand_gen;
    std::shared_ptr<DataTransformer> m_transformer;

    //params
    bool m_autostart;
    bool m_is_running;// if the loop of loading is running, it is used to break the loop when the user ctrl-c
    std::string m_mode; // train or test or val
    fs::path m_dataset_path; 
    fs::path m_sequence; 
    int m_nr_clouds_to_skip;
    int m_nr_clouds_to_read;
    float m_cap_distance;
    bool m_shuffle_points; //When splatting in a permutohedral lattice it's better to have adyancent point in 3D be in different parts in memoru to aboid hashing conflicts
    bool m_do_pose;
    bool m_normalize; //normalizes the point cloud between [-1,1]
    bool m_shuffle;
    bool m_do_overfit; // return all the time just one of the clouds, specifically the first one
    // bool m_do_adaptive_subsampling; //randomly drops points from the cloud, dropping with more probability the ones that are closes and with less the ones further
    std::thread m_loader_thread;
    uint32_t m_idx_cloud_to_read;
    int m_nr_resets;
    // std::string m_pose_file;
    // std::string m_pose_file_format;


    //internal
    bool m_is_modified; //indicate that a cloud was finished processind and you are ready to get it 
    int m_nr_sequences;
    std::vector<fs::path> m_npz_filenames;
    moodycamel::ReaderWriterQueue<MeshCore> m_clouds_buffer;
    // std::vector<Eigen::Affine3d,  Eigen::aligned_allocator<Eigen::Affine3d>  >m_worldROS_cam_vec; //actually the semantic kitti expressed the clouds in the left camera coordinate so it should be m_worldRos_cam_vec 
    std::unordered_map< std::string,  std::vector<Eigen::Affine3d,  Eigen::aligned_allocator<Eigen::Affine3d>  > > m_poses_per_sequence; //each sequence is identified by a string like "00, 01 etc". Each has a vector of poses
    Eigen::Affine3d m_tf_cam_velodyne;
    Eigen::Affine3d m_tf_worldGL_worldROS;

    //label mngr to link to all the meshes that will have a semantic information
    std::shared_ptr<LabelMngr> m_label_mngr;

};