#pragma once

#include <TinyOBJ/tiny_obj_loader.h>

#include "as/common.hpp"

namespace as {

void LoadObjByTinyobj(const std::string &path,
                      std::vector<glm::vec3> &vertices);
}
