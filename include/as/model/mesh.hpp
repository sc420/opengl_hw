#pragma once

#include "as/common.hpp"

#include "as/model/material.hpp"
#include "as/model/texture.hpp"
#include "as/model/vertex.hpp"

namespace as {
class Mesh {
 public:
  Mesh();

  Mesh(const std::string &name, const std::vector<Vertex> &vertices,
       const std::vector<size_t> &idxs, const Material &material);

  std::string GetName() const;

  std::vector<Vertex> GetVertices() const;

  std::vector<size_t> GetIdxs() const;

  Material GetMaterial() const;

  size_t GetVerticesMemSize() const;

  size_t GetIdxsMemSize() const;

 private:
  std::string name_;

  std::vector<Vertex> vertices_;

  std::vector<size_t> idxs_;

  Material material_;
};
}  // namespace as
