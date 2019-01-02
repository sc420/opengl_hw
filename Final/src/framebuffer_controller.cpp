#include "framebuffer_controller.hpp"

void ctrl::FramebufferController::RegisterGLManagers(
    as::GLManagers &gl_managers) {
  gl_managers_ = &gl_managers;
}

void ctrl::FramebufferController::UseDefaultFramebuffer() {
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  framebuffer_manager.BindDefaultFramebuffer(GL_FRAMEBUFFER);
}

void ctrl::FramebufferController::UseScreenFramebuffer(const int screen_idx) {
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  const std::string framebuffer_name = GetScreenFramebufferName(screen_idx);
  framebuffer_manager.BindFramebuffer(framebuffer_name, GL_FRAMEBUFFER);
}

std::string ctrl::FramebufferController::GetScreenFramebufferName(
    const int screen_idx) const {
  return "screen[" + std::to_string(screen_idx) + "]";
}

const int ctrl::FramebufferController::kNumFramebuffers = 3;
