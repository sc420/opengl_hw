#pragma once

#include <string>
#include <vector>

#include <TinyOBJ/tiny_obj_loader.h>

#include "assignment/common.hpp"

void TinyobjLoadObj(const std::string &path, std::vector<glm::vec3> &vertices);
