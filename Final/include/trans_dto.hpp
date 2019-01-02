#pragma once

#include "as/common.hpp"

namespace dto {
struct GlobalTrans {
  glm::mat4 model;  // 64*0=0, +64->64
  glm::mat4 view;   // 64*1=64, +64->128
  glm::mat4 proj;   // 64*2=128, +64->192
};

struct ModelTrans {
  glm::mat4 trans;  // 64*0=0, +64->64
};
}  // namespace dto
