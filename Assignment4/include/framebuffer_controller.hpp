#pragma once

#include "as/common.hpp"
#include "as/gl/gl_tools.hpp"

namespace ctrl {
class FramebufferController {
 public:
  void RegisterGLManagers(as::GLManagers& gl_managers);

  void UseDefaultFramebuffer();

  void UseScreenFramebuffer(const int screen_idx);

  std::string GetScreenFramebufferName(const int screen_idx) const;

 private:
  as::GLManagers* gl_managers_;
};

}  // namespace ctrl
