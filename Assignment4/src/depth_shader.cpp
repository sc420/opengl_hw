#include "depth_shader.hpp"

/*******************************************************************************
 * Shader Registrations
 ******************************************************************************/

void shader::DepthShader::RegisterSceneShader(SceneShader &scene_shader) {
  scene_shader_ = &scene_shader;
}

/*******************************************************************************
 * GL Initializations
 ******************************************************************************/

void shader::DepthShader::Init() {
  Shader::Init();
  InitFramebuffers();
  InitDepthTexture();
}

void shader::DepthShader::InitFramebuffers() {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string name = GetDepthFramebufferName();
  // Create framebuffers
  framebuffer_manager.GenFramebuffer(name);
  // Bind framebuffers
  framebuffer_manager.BindFramebuffer(name, GL_FRAMEBUFFER);
  // Disable color rendering
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
}

void shader::DepthShader::InitDepthTexture() {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names
  const std::string framebuffer_name = GetDepthFramebufferName();
  const std::string tex_name = GetDepthTextureName();
  const std::string unit_name = GetDepthTextureUnitName();
  // Generate texture
  texture_manager.GenTexture(tex_name);
  // Update texture
  texture_manager.BindTexture(tex_name, GL_TEXTURE_2D, unit_name);

  // texture_manager.InitTexture2D(
  //    tex_name, GL_TEXTURE_2D, 1, GL_R8, kDepthMapSize.x,
  //    kDepthMapSize.y);  // TODO: May need to use glTexImage2D

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kDepthMapSize.x,
               kDepthMapSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                     GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                     GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                     GL_REPEAT);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                     GL_REPEAT);
  // Attach textures to framebuffers
  framebuffer_manager.AttachTexture2DToFramebuffer(
      framebuffer_name, tex_name, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
      GL_TEXTURE_2D, 0);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::DepthShader::Draw(const glm::ivec2 &window_size) {
  // Get light position in the scene
  const glm::vec3 light_pos = scene_shader_->GetLightPos();
  // Use a camera at the light position
  const as::CameraTrans camera_trans(
      light_pos,
      glm::vec3(glm::radians(20.0f), glm::radians(-90.0f), glm::radians(0.0f)));
  // Get the original global transformation
  const SceneShader::GlobalTrans orig_global_trans =
      scene_shader_->GetGlobalTrans();
  // Set the new global transformation of the light
  const glm::mat4 light_proj =
      glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1e-3f, 1e3f);
  const glm::mat4 light_view = camera_trans.GetTrans();
  const glm::mat4 light_model = glm::mat4(1.0f);
  const SceneShader::GlobalTrans global_trans = {light_proj, light_view,
                                                 light_model};
  scene_shader_->UpdateGlobalTrans(global_trans);
  // Set the new viewport
  glViewport(0, 0, kDepthMapSize.x, kDepthMapSize.y);

  // Reset the original global transformations
  scene_shader_->UpdateGlobalTrans(orig_global_trans);
  // Reset the viewport
  glViewport(0, 0, window_size.x, window_size.y);
}

void shader::DepthShader::UseDepthFramebuffer() {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string name = GetDepthFramebufferName();
  // Bind framebuffers
  framebuffer_manager.BindFramebuffer(name);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::DepthShader::GetId() const { return "depth"; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::DepthShader::GetDepthFramebufferName() const {
  return GetProgramName() + "/depth";
}

std::string shader::DepthShader::GetDepthTextureName() const {
  return GetProgramName() + "/depth";
}

std::string shader::DepthShader::GetDepthTextureUnitName() const {
  return GetProgramName();
}

/*******************************************************************************
 * Constants (Private)
 ******************************************************************************/

const glm::ivec2 shader::DepthShader::kDepthMapSize = glm::ivec2(1024, 1024);
