#pragma once

#include <stb/stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>

#include "as/common.hpp"

namespace as {

void LoadModelByTinyobj(const std::string &path,
                        std::vector<glm::vec3> &vertices,
                        std::vector<glm::vec3> &normals,
                        std::vector<glm::vec2> &tex_coords);

void LoadTextureByStb(const std::string &path, const GLint req_comp,
                      GLsizei &width, GLsizei &height, GLint &comp,
                      std::vector<GLubyte> &texels);

}  // namespace as
