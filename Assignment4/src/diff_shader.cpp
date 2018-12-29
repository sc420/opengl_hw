#include "diff_shader.hpp"

/*******************************************************************************
 * GL Initializations
 ******************************************************************************/

void shader::DiffShader::Init() {
  // TODO: Other shaders should explicitly initialize programs and shaders
  InitFramebuffers();
}

void shader::DiffShader::InitFramebuffers() {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Create framebuffers
  DiffTypes diff_types[] = {DiffTypes::kObj, DiffTypes::kNoObj, DiffTypes::kBg};
  for (const DiffTypes diff_type : diff_types) {
    const std::string framebuffer_name = GetDiffFramebufferName(diff_type);
    framebuffer_manager.GenFramebuffer(framebuffer_name);
  }
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::DiffShader::UseDiffFramebuffer(const DiffTypes diff_type) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string framebuffer_name = GetDiffFramebufferName(diff_type);
  // Bind framebuffer
  framebuffer_manager.BindFramebuffer(framebuffer_name, GL_FRAMEBUFFER);
}

void shader::DiffShader::UpdateObjDiffRenderbuffers(const GLsizei width,
                                                    const GLsizei height) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string obj_framebuffer_name =
      GetDiffFramebufferName(DiffTypes::kObj);
  const std::string obj_renderbuffer_name = GetObjDiffDepthRenderbufferName();
  // Check whether to delete old renderbuffer
  if (framebuffer_manager.HasRenderbuffer(obj_renderbuffer_name)) {
    framebuffer_manager.DeleteRenderbuffer(obj_renderbuffer_name);
  }
  // Create renderbuffers
  framebuffer_manager.GenRenderbuffer(obj_renderbuffer_name);
  // Initialize renderbuffers
  framebuffer_manager.InitRenderbuffer(obj_renderbuffer_name, GL_RENDERBUFFER,
                                       GL_DEPTH24_STENCIL8, width, height);
  // Attach renderbuffers to framebuffers
  framebuffer_manager.AttachRenderbufferToFramebuffer(
      obj_framebuffer_name, obj_renderbuffer_name, GL_FRAMEBUFFER,
      GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::DiffShader::GetId() const { return "diff"; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::DiffShader::GetDiffFramebufferName(
    const DiffTypes diff_type) const {
  std::string suffix;
  switch (diff_type) {
    case DiffTypes::kObj: {
      suffix = "obj";
    } break;
    case DiffTypes::kNoObj: {
      suffix = "no_obj";
    } break;
    case DiffTypes::kBg: {
      suffix = "bg";
    } break;
    default: { throw std::runtime_error("Unknown diff type"); }
  }
  return GetProgramName() + "/framebuffer/" + suffix;
}

std::string shader::DiffShader::GetObjDiffDepthRenderbufferName() const {
  return GetProgramName() + "/renderbuffer/obj";
}
