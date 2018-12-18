#include "as/model/vertex.hpp"

as::Vertex::Vertex()
    : pos(glm::vec3(0.0f)),
      tex_coords(glm::vec2(0.0f)),
      normal(glm::vec3(0.0f)),
      tangent(glm::vec3(0.0f)),
      bitangent(glm::vec3(0.0f)) {}

size_t as::Vertex::GetMemSize() {
  return sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) +
         sizeof(glm::vec3) + sizeof(glm::vec3);
}
