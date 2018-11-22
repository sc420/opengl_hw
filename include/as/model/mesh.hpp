#pragma once

#include "as/common.hpp"
#include "as/model/vertex.hpp"

namespace as {
class Mesh {
 public:
  Mesh();

  Mesh(const std::vector<Vertex> &vertices, const std::vector<size_t> &idxs);

  size_t GetVerticesMemSize() const;

  size_t GetIdxsMemSize() const;

  /* Geometry data */
  std::vector<Vertex> vertices;

  std::vector<size_t> idxs;
};
}  // namespace as
