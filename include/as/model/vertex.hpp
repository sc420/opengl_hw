#pragma once

#include "as/common.hpp"

namespace as {
class Vertex {
 public:
  glm::vec3 pos;
  glm::vec2 tex_coords;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;

  Vertex();

  static size_t GetMemSize();
};

}  // namespace as
