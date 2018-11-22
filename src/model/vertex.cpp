#include "as/model/vertex.hpp"

as::Vertex::Vertex(): pos(glm::vec3(0.0f)), normal(glm::vec3(0.0f)), tex_coords(glm::vec2(0.0f))
{
}

as::Vertex::Vertex(const glm::vec3 pos, const glm::vec3 normal, const glm::vec2 tex_coords): pos(pos), normal(normal), tex_coords(tex_coords)
{
}

size_t as::Vertex::GetMemSize()
{
  return sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2);
}
