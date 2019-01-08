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
  LinkDataToUniformBlock(GetGlobalTransBufferName(),
                         GetGlobalTransUniformBlockName(), global_trans_);
  LinkDataToUniformBlock(GetModelTransBufferName(),
                         GetModelTransUniformBlockName(), model_trans_);
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

  // Draw the scene models without textures
  const auto &scene_models = scene_shader_->GetSceneModels();
  for (const auto &pair : scene_models) {
    const dto::SceneModel &scene_model = pair.second;

    // Update states
    UpdateModelTrans(scene_model);
    // Draw the model
    DrawModelWithoutTextures(scene_model);
  }

  // Reset the viewport
  glViewport(0, 0, window_size.x, window_size.y);
}

void shader::DepthShader::DrawModelWithoutTextures(
    const dto::SceneModel &scene_model) {
  // Get model
  const as::Model &model = scene_model.GetModel();
  // Get meshes
  const std::vector<as::Mesh> &meshes = model.GetMeshes();
  // Get names
  const std::string group_name = scene_model.GetVertexArrayGroupName();

  // Draw each mesh with its own texture
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Get the array indexes
    const std::vector<size_t> &idxs = mesh.GetIdxs();
    /* Draw Vertex Arrays */
    UseMesh(group_name, mesh_idx);
    glDrawElementsInstanced(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, nullptr,
                            scene_model.GetNumInstancing());
  }
}

dto::GlobalTrans shader::DepthShader::GetLightTrans() const {
  // Use a camera at the light position
  const as::CameraTrans camera_trans(scene_shader_->GetLightPos(),
                                     scene_shader_->GetLightAngles());
  // Set the light space
  const glm::mat4 light_proj = scene_shader_->GetLightProjection();
  const glm::mat4 light_view = camera_trans.GetTrans();
  const glm::mat4 light_model = glm::mat4(1.0f);
  // Set the new global transformation to the light space
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

std::string shader::DepthShader::GetGlobalTransBufferName() const {
  return GetProgramName() + "/buffer/global_trans";
}

std::string shader::DepthShader::GetModelTransBufferName() const {
  return GetProgramName() + "/buffer/model_trans";
}

std::string shader::DepthShader::GetGlobalTransUniformBlockName() const {
  return "GlobalTrans";
}

std::string shader::DepthShader::GetModelTransUniformBlockName() const {
  return "ModelTrans";
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
  const std::string buffer_name = GetGlobalTransBufferName();
  // Update global transformation
  global_trans_ = global_trans;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::DepthShader::UpdateModelTrans(const dto::SceneModel &scene_model) {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetModelTransBufferName();

  // Update model transformation
  model_trans_.trans = scene_model.GetTrans();

  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}
