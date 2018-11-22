#include "as/model/mesh.hpp"

as::Mesh::Mesh(const std::vector<Vertex>& vertices,
               const std::vector<size_t> idxs)
    : vertices(vertices), idxs(idxs) {}

size_t as::Mesh::GetVerticesMemSize() const {
  return Vertex::GetMemSize() * vertices.size();
}

size_t as::Mesh::GetIdxsMemSize() const { return sizeof(size_t) * idxs.size(); }
