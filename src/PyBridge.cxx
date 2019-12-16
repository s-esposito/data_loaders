#include "data_loaders/PyBridge.h"

// #include <torch/extension.h>
// #include "torch/torch.h"
// #include "torch/csrc/utils/pybind.h"

//my stuff 
#include "data_loaders/DataLoaderShapeNetPartSeg.h"
#include "easy_pbr/Mesh.h"
#include "easy_pbr/LabelMngr.h"


// https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html
// PYBIND11_MAKE_OPAQUE(std::vector<int>); //to be able to pass vectors by reference to functions and have things like push back actually work 
// PYBIND11_MAKE_OPAQUE(std::vector<float>, std::allocator<float> >);

namespace py = pybind11;




PYBIND11_MODULE(dataloaders, m) {

    // py::module::import("easypbr");
    // py::object mesh_py = (py::object) py::module::import("easypbr").attr("Mesh");
    // py::print(Mesh.attr("path"));
    // py::object mesh=py::module::import("easypbr").attr("Mesh");;
 
    //DataLoader ShapeNetPartSeg
    py::class_<DataLoaderShapeNetPartSeg> (m, "DataLoaderShapeNetPartSeg")
    .def(py::init<const std::string>())
    .def("start", &DataLoaderShapeNetPartSeg::start )
    .def("get_cloud", &DataLoaderShapeNetPartSeg::get_cloud )
    .def("has_data", &DataLoaderShapeNetPartSeg::has_data ) 
    .def("is_finished", &DataLoaderShapeNetPartSeg::is_finished ) 
    .def("is_finished_reading", &DataLoaderShapeNetPartSeg::is_finished_reading ) 
    .def("reset", &DataLoaderShapeNetPartSeg::reset ) 
    .def("nr_samples", &DataLoaderShapeNetPartSeg::nr_samples ) 
    .def("set_mode_train", &DataLoaderShapeNetPartSeg::set_mode_train ) 
    .def("set_mode_test", &DataLoaderShapeNetPartSeg::set_mode_test ) 
    .def("set_mode_validation", &DataLoaderShapeNetPartSeg::set_mode_validation ) 
    .def("get_object_name", &DataLoaderShapeNetPartSeg::get_object_name ) 
    .def("set_object_name", &DataLoaderShapeNetPartSeg::set_object_name ) 
    // .def("get_cloud", [mesh_py](DataLoaderShapeNetPartSeg& loader ) { 
    //     MeshSharedPtr mesh=loader.get_cloud();
    //     return mesh;
    //     // return static_cast<Mesh *>(mesh);
    //     // return py::cast(mesh);
    //     // mesh_py=py::cast(mesh);
    //     // return mesh_py;
    // } )
    ;

    //  py::class_<Mesh, std::shared_ptr<Mesh>> (m, "Mesh")
    // .def(py::init<>())
    // .def(py::init<std::string>())
    // .def("load_from_file", &Mesh::load_from_file )
    // .def("save_to_file", &Mesh::save_to_file )
    // .def("clone", &Mesh::clone )
    // .def("add", &Mesh::add )
    // .def("is_empty", &Mesh::is_empty )
    // .def("create_box_ndc", &Mesh::create_box_ndc )
    // .def("create_floor", &Mesh::create_floor )
    // .def_readwrite("id", &Mesh::id)
    // .def_readwrite("name", &Mesh::name)
    // .def_readwrite("m_width", &Mesh::m_width)
    // .def_readwrite("m_height", &Mesh::m_height)
    // .def_readwrite("m_vis", &Mesh::m_vis)
    // .def_readwrite("m_force_vis_update", &Mesh::m_force_vis_update)
    // .def_readwrite("V", &Mesh::V)
    // .def_readwrite("F", &Mesh::F)
    // .def_readwrite("C", &Mesh::C)
    // .def_readwrite("E", &Mesh::E)
    // .def_readwrite("D", &Mesh::D)
    // .def_readwrite("NF", &Mesh::NF)
    // .def_readwrite("NV", &Mesh::NV)
    // .def_readwrite("UV", &Mesh::UV)
    // .def_readwrite("V_tangent_u", &Mesh::V_tangent_u)
    // .def_readwrite("V_lenght_v", &Mesh::V_length_v)
    // .def_readwrite("L_pred", &Mesh::L_pred)
    // .def_readwrite("L_gt", &Mesh::L_gt)
    // .def_readwrite("I", &Mesh::I)
    // .def_readwrite("m_label_mngr", &Mesh::m_label_mngr )
    // .def_readwrite("m_min_max_y_for_plotting", &Mesh::m_min_max_y_for_plotting )
    // .def("translate_model_matrix", &Mesh::translate_model_matrix )
    // .def("rotate_model_matrix", &Mesh::rotate_model_matrix )
    // .def("rotate_model_matrix_local", &Mesh::rotate_model_matrix_local )
    // // .def("rotate_x_axis", &Mesh::rotate_x_axis )
    // // .def("rotate_y_axis", &Mesh::rotate_y_axis )
    // .def("random_subsample", &Mesh::random_subsample )
    // // .def("move_in_x", &Mesh::move_in_x )
    // // .def("move_in_y", &Mesh::move_in_y )
    // // .def("move_in_z", &Mesh::move_in_z )
    // .def("add_child", &Mesh::add_child )
    // ;


}