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
  CreateShaders();
  CreatePrograms();
  InitFramebuffers();
  InitUniformBlocks();
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

void shader::DepthShader::InitUniformBlocks() {
  LinkDataToUniformBlock("depth/global_trans", "GlobalTrans", global_trans_);
  LinkDataToUniformBlock("depth/model_trans", "ModelTrans", model_trans_);
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
  texture_manager.InitTexture2D(tex_name, GL_TEXTURE_2D, 1,
                                GL_DEPTH_COMPONENT16, kDepthMapSize.x,
                                kDepthMapSize.y);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                     GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                     GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                     GL_CLAMP_TO_BORDER);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                     GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // Attach textures to framebuffers
  framebuffer_manager.AttachTexture2DToFramebuffer(
      framebuffer_name, tex_name, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
      GL_TEXTURE_2D, 0);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::DepthShader::Draw(const glm::ivec2 &window_size) {
  // Use depth program
  UseProgram();
  // Set the new viewport
  glViewport(0, 0, kDepthMapSize.x, kDepthMapSize.y);
  // Get light space transformation
  const dto::GlobalTrans global_trans = GetLightTrans();
  // Update global transformation
  UpdateGlobalTrans(global_trans);

  // Draw the scene
  // Get names
  const std::string quad_group_name = "scene/quad";
  const std::string scene_group_name = "scene/scene";
  // Get models
  const as::Model &quad_model = scene_shader_->GetQuadModel();
  const as::Model &scene_model = scene_shader_->GetSceneModel();

  // Draw the quad
  UpdateQuadModelTrans();
  DrawModelWithoutTextures(quad_model, quad_group_name);
  // Draw the scene
  UpdateSceneModelTrans();
  DrawModelWithoutTextures(scene_model, scene_group_name);

  // Reset the viewport
  glViewport(0, 0, window_size.x, window_size.y);
}

void shader::DepthShader::DrawModelWithoutTextures(
    const as::Model &model, const std::string &group_name) {
  // Get meshes
  const std::vector<as::Mesh> &meshes = model.GetMeshes();

  // Draw each mesh with its own texture
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Get the array indexes
    const std::vector<size_t> &idxs = mesh.GetIdxs();
    /* Draw Vertex Arrays */
    UseMesh(group_name, mesh_idx);
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, nullptr);
  }
}

dto::GlobalTrans shader::DepthShader::GetLightTrans() const {
  // TODO: Should get from scene shader
  // Get light position in the scene
  const glm::vec3 light_pos = glm::vec3(-20.0f, 0.0f, 15.0f);
  // Use a camera at the light position
  const as::CameraTrans camera_trans(
      light_pos,
      glm::vec3(glm::radians(30.0f), glm::radians(30.0f), glm::radians(0.0f)));
  // Set the new global transformation of the light
  const glm::mat4 light_proj =
      glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1e-3f, 1e3f);
  const glm::mat4 light_view = camera_trans.GetTrans();
  const glm::mat4 light_model = glm::mat4(1.0f);
  dto::GlobalTrans global_trans;
  global_trans.proj = light_proj;
  global_trans.view = light_view;
  global_trans.model = light_model;
  return global_trans;
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

std::string shader::DepthShader::GetDepthTextureName() const {
  return GetProgramName() + "/texture/depth";
}

std::string shader::DepthShader::GetDepthTextureUnitName() const {
  return GetProgramName() + "/texture_unit_name/depth";
}

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::DepthShader::GetDepthFramebufferName() const {
  return GetProgramName() + "/framebuffer/depth";
}

/*******************************************************************************
 * Constants (Private)
 ******************************************************************************/

const glm::ivec2 shader::DepthShader::kDepthMapSize = glm::ivec2(1024, 1024);

/*******************************************************************************
 * State Updaters (Private)
 ******************************************************************************/

void shader::DepthShader::UpdateGlobalTrans(
    const dto::GlobalTrans &global_trans) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = "depth/global_trans";
  // Update global transformation
  global_trans_ = global_trans;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

// HACK: Should combine
void shader::DepthShader::UpdateQuadModelTrans() {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update model transformation
  const glm::vec3 scale_factors = 100.0f * glm::vec3(0.5f, 0.35f, 0.5f);
  const glm::vec3 translate_factors = glm::vec3(-10.0f, -13.0f, -8.0f);
  glm::mat4 trans = glm::translate(glm::mat4(1.0f), translate_factors);
  trans = glm::scale(trans, scale_factors);
  trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  model_trans_.trans = trans;
  // Update the buffer
  const std::string buffer_name = "depth/model_trans";
  buffer_manager.UpdateBuffer(buffer_name);
}

// HACK: Should combine
void shader::DepthShader::UpdateSceneModelTrans() {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update model transformation
  const glm::vec3 scale_factors = glm::vec3(0.5f, 0.35f, 0.5f);
  const glm::vec3 translate_factors = glm::vec3(-10.0f, -13.0f, -8.0f);
  glm::mat4 trans = glm::translate(glm::mat4(1.0f), translate_factors);
  trans = glm::scale(trans, scale_factors);
  trans = glm::rotate(trans, scene_shader_->model_rotation,
                      glm::vec3(0.0f, 1.0f, 0.0f));
  model_trans_.trans = trans;
  // Update the buffer
  const std::string buffer_name = "depth/model_trans";
  buffer_manager.UpdateBuffer(buffer_name);
}
