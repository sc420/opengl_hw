#pragma once

#include <tinyobjloader/tiny_obj_loader.h>

#include "as/common.hpp"

namespace as {

void LoadObjByTinyobj(const std::string &path,
                      std::vector<glm::vec3> &vertices);
}
