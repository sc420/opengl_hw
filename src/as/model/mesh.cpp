#include "as/model/mesh.hpp"

as::Mesh::Mesh() {}

as::Mesh::Mesh(const std::string& name, const std::vector<Vertex>& vertices,
               const std::vector<size_t>& idxs, const Material& material)
    : name_(name), vertices_(vertices), idxs_(idxs), material_(material) {}

std::string as::Mesh::GetName() const { return name_; }

std::vector<as::Vertex> as::Mesh::GetVertices() const { return vertices_; }

std::vector<size_t> as::Mesh::GetIdxs() const { return idxs_; }

as::Material as::Mesh::GetMaterial() const { return material_; }

size_t as::Mesh::GetVerticesMemSize() const {
  return Vertex::GetMemSize() * vertices_.size();
}

size_t as::Mesh::GetIdxsMemSize() const {
  return sizeof(size_t) * idxs_.size();
}
