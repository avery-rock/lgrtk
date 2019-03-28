#include <sstream>
#include <fstream>
#include <iomanip>

#include <lgr_vtk.hpp>
#include <lgr_print.hpp>
#include <lgr_range_product.hpp>
#include <lgr_input.hpp>
#include <lgr_state.hpp>

namespace lgr {

static void start_vtk_file(std::ostream& stream) {
  stream << "# vtk DataFile Version 3.0\n";
  stream << "vtk output\n";
  stream << "ASCII\n";
  stream << "DATASET UNSTRUCTURED_GRID\n";
}

template <class Index>
static void write_vtk_points(std::ostream& stream,
    device_vector<vector3<double>, Index> const& x) {
  stream << "POINTS " << int(x.size()) << " double\n";
  for (vector3<double> p : x) {
    stream << p << "\n";
  }
}

static void write_vtk_cells(std::ostream& stream, input const& in, state const& s) {
  stream << "CELLS " << int(s.elements.size()) << " " << (int(s.elements.size()) * (int(s.nodes_in_element.size()) + 1)) << "\n";
  auto const elements_to_element_nodes = s.elements * s.nodes_in_element;
  auto const element_nodes_to_nodes = s.elements_to_nodes.cbegin();
  for (auto const element_nodes : elements_to_element_nodes) {
    stream << int(element_nodes.size());
    for (auto const element_node : element_nodes) {
      node_index const node = element_nodes_to_nodes[element_node];
      stream << " " << int(node);
    }
    stream << "\n";
  }
  stream << "CELL_TYPES " << int(s.elements.size()) << "\n";
  int cell_type = -1;
  switch (in.element) {
    case BAR: cell_type = 3; break;
    case TRIANGLE: cell_type = 5; break;
    case TETRAHEDRON: cell_type = 10; break;
  }
  for (element_index i(0); i < s.elements.size(); ++i) {
    stream << cell_type << "\n";
  }
}

static void write_vtk_point_data(std::ostream& stream, state const& s) {
  stream << "POINT_DATA " << int(s.nodes.size()) << "\n";
}

template <class Index>
static void write_vtk_scalars(std::ostream& stream, char const* name,
    device_vector<double, Index> const& vec) {
  stream << "SCALARS " << name << " double 1\n";
  stream << "LOOKUP_TABLE default\n";
  for (double const val : vec) {
    stream << val << "\n";
  }
}

template <class Index>
static void write_vtk_vectors(std::ostream& stream, char const* name,
    device_vector<vector3<double>, Index> const& vec) {
  stream << "VECTORS " << name << " double\n";
  for (vector3<double> const val : vec) {
    stream << val << "\n";
  }
}

static void write_vtk_cell_data(std::ostream& stream, state const& s) {
  stream << "CELL_DATA " << int(s.elements.size()) << "\n";
}

void file_writer::operator()(
    input const& in,
    int const file_output_index,
    state const& s
    ) {
  std::stringstream filename_stream;
  filename_stream << prefix << "_" << file_output_index << ".vtk";
  auto const filename = filename_stream.str();
  std::ofstream stream(filename.c_str());
  stream << std::scientific << std::setprecision(17);
  start_vtk_file(stream);
  write_vtk_points(stream, s.x);
  write_vtk_cells(stream, in, s);
  write_vtk_point_data(stream, s);
  write_vtk_vectors(stream, "position", s.x);
  write_vtk_vectors(stream, "velocity", s.v);
  if (in.enable_nodal_pressure || in.enable_nodal_energy) {
    write_vtk_scalars(stream, "nodal_pressure", s.p_h);
  }
  if (in.enable_nodal_energy) {
    write_vtk_scalars(stream, "nodal_energy", s.e_h);
    write_vtk_scalars(stream, "nodal_density", s.rho_h);
  }
  write_vtk_cell_data(stream, s);
  write_vtk_scalars(stream, "energy", s.e);
  write_vtk_scalars(stream, "pressure", s.p);
  write_vtk_scalars(stream, "density", s.rho);
  stream.close();
}

}
