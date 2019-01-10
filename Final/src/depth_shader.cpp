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
  // Initialize multiple depth textures
  const DepthTextureTypes depth_tex_types[] = {DepthTextureTypes::kFromLight,
                                               DepthTextureTypes::kFromCamera};
  for (const DepthTextureTypes depth_tex_type : depth_tex_types) {
    InitDepthTexture(depth_tex_type);
  }
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

void shader::DepthShader::InitDepthTexture(
    const DepthTextureTypes depth_tex_type) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names
  const std::string framebuffer_name = GetDepthFramebufferName();
  const std::string tex_name = GetDepthTextureName(depth_tex_type);
  const std::string unit_name = GetDepthTextureUnitName(depth_tex_type);
  // Generate texture
  texture_manager.GenTexture(tex_name);
  // Update texture
  texture_manager.BindTexture(tex_name, GL_TEXTURE_2D, unit_name);
  texture_manager.InitTexture2D(tex_name, GL_TEXTURE_2D, 3,
                                GL_DEPTH_COMPONENT16, kDepthMapSize.x,
                                kDepthMapSize.y);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                     GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                     GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                     GL_CLAMP_TO_BORDER);
  texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                     GL_CLAMP_TO_BORDER);
  const float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // Attach textures to framebuffers
  framebuffer_manager.AttachTexture2DToFramebuffer(
      framebuffer_name, tex_name, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
      GL_TEXTURE_2D, 0);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::DepthShader::DrawFromLight(const glm::ivec2 &window_size) {
  // Use depth program
  UseProgram();
  // Set the new viewport
  glViewport(0, 0, kDepthMapSize.x, kDepthMapSize.y);
  // Get light space transformation
  const dto::GlobalTrans global_trans = GetLightTrans();
  // Update global transformation
  UpdateGlobalTrans(global_trans);

  // Use "from light" texture
  UseDepthTexture(DepthTextureTypes::kFromLight);

  as::ClearDepthBuffer();

  // Draw the scene models without textures
  const auto &scene_models = scene_shader_->GetSceneModels();
  for (const auto &pair : scene_models) {
    const dto::SceneModel &scene_model = pair.second;

    // Update states
    UpdateModelTrans(scene_model);
    // Draw the model
    DrawModelWithoutTextures(scene_model);
  }

  // Restore the viewport
  glViewport(0, 0, window_size.x, window_size.y);
}

void shader::DepthShader::DrawFromCamera(const glm::ivec2 &window_size,
                                         const dto::GlobalTrans &camera_trans) {
  // Use depth program
  UseProgram();
  // Set the new viewport
  glViewport(0, 0, kDepthMapSize.x, kDepthMapSize.y);
  // Update global transformation
  UpdateGlobalTrans(camera_trans);

  // Use "from light" texture
  UseDepthTexture(DepthTextureTypes::kFromCamera);

  as::ClearDepthBuffer();

  // Draw the scene models without textures
  const auto &scene_models = scene_shader_->GetSceneModels();
  for (const auto &pair : scene_models) {
    const dto::SceneModel &scene_model = pair.second;

    // Update states
    UpdateModelTrans(scene_model);
    // Draw the model
    DrawModelWithoutTextures(scene_model);
  }

  // Restore the viewport
  glViewport(0, 0, window_size.x, window_size.y);
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

void shader::DepthShader::UseDepthTexture(
    const DepthTextureTypes depth_tex_type) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string framebuffer_name = GetDepthFramebufferName();
  const std::string tex_name = GetDepthTextureName(depth_tex_type);
  // Attach textures to framebuffers
  framebuffer_manager.AttachTexture2DToFramebuffer(
      framebuffer_name, tex_name, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
      GL_TEXTURE_2D, 0);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::DepthShader::GetId() const { return "depth"; }

std::string shader::DepthShader::GetDepthTextureName(
    const DepthTextureTypes depth_tex_type) const {
  return GetProgramName() + "/texture/depth/" +
         DepthTextureTypeToName(depth_tex_type);
}

std::string shader::DepthShader::GetDepthTextureUnitName(
    const DepthTextureTypes depth_tex_type) const {
  return GetProgramName() + "/texture_unit_name/depth/" +
         DepthTextureTypeToName(depth_tex_type);
}

std::string shader::DepthShader::DepthTextureTypeToName(
    const DepthTextureTypes depth_tex_type) const {
  switch (depth_tex_type) {
    case DepthTextureTypes::kFromLight:
      return "from_light";
    case DepthTextureTypes::kFromCamera:
      return "from_camera";
    default:
      throw std::runtime_error("Unknown depth texture type");
  }
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

const glm::ivec2 shader::DepthShader::kDepthMapSize = glm::ivec2(2048, 2048);

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

/*******************************************************************************
 * GL Drawing Methods (Private)
 ******************************************************************************/

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
