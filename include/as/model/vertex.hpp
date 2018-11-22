#pragma once

#include "as/common.hpp"

namespace as {
class Vertex {
 public:
  Vertex();

  Vertex(const glm::vec3 pos, const glm::vec3 normal,
         const glm::vec2 tex_coords);

  static size_t GetMemSize();

  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

}  // namespace as
