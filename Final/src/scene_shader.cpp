#include "scene_shader.hpp"

shader::SceneShader::SceneShader()
    : model_rotation(glm::radians(0.0f)),
      global_trans_(dto::GlobalTrans()),
      model_trans_(dto::ModelTrans()),
      model_material_(ModelMaterial()),
      lighting_(Lighting()),
      use_normal_height(true) {}

/*******************************************************************************
 * Shader Registrations
 ******************************************************************************/

void shader::SceneShader::RegisterDepthShader(const DepthShader &depth_shader) {
  depth_shader_ = &depth_shader;
}

void shader::SceneShader::RegisterSkyboxShader(
    const SkyboxShader &skybox_shader) {
  skybox_shader_ = &skybox_shader;
}

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

void shader::SceneShader::LoadModel() {
  scene_models_["scene"] =
      dto::SceneModel("assets/models/nanosuit/nanosuit.obj",
                      aiProcess_CalcTangentSpace | aiProcess_Triangulate |
                          aiProcess_GenNormals | aiProcess_FlipUVs,
                      "nanosuit", 5, gl_managers_);
  scene_models_["quad"] =
      dto::SceneModel("assets/models/quad/quad.obj",
                      aiProcess_CalcTangentSpace | aiProcess_GenNormals, "quad",
                      1, gl_managers_);
}

const as::Model &shader::SceneShader::GetSceneModel() {
  return scene_models_.at("scene").GetModel();
}

const as::Model &shader::SceneShader::GetQuadModel() {
  return scene_models_.at("quad").GetModel();
}

/*******************************************************************************
 * GL Initializations
 ******************************************************************************/

void shader::SceneShader::Init() {
  CreateShaders();
  CreatePrograms();
  LoadModel();
  InitVertexArrays();
  InitUniformBlocks();
  InitLightTrans();
}

void shader::SceneShader::InitVertexArrays() {
  InitVertexArray(GetSceneGroupName(), GetSceneModel());
  InitVertexArray(GetQuadGroupName(), GetQuadModel());
}

void shader::SceneShader::InitUniformBlocks() {
  LinkDataToUniformBlock(GetGlobalTransBufferName(),
                         GetGlobalTransUniformBlockName(), global_trans_);
  LinkDataToUniformBlock(GetModelTransBufferName(),
                         GetModelTransUniformBlockName(), model_trans_);
  LinkDataToUniformBlock(GetModelMaterialBufferName(),
                         GetModelMaterialUniformBlockName(), model_material_);
  LinkDataToUniformBlock(GetLightingBufferName(), GetLightingUniformBlockName(),
                         lighting_);
  LinkDataToUniformBlock(GetShadowBufferName(), GetShadowUniformBlockName(),
                         shadow_);
}

void shader::SceneShader::InitLightTrans() {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetLightingBufferName();
  // Get light space transformation
  const dto::GlobalTrans global_trans = depth_shader_->GetLightTrans();
  lighting_.light_trans =
      global_trans.proj * global_trans.view * global_trans.model;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::ReuseSkyboxTexture() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names
  const std::string skybox_tex_name = skybox_shader_->GetTextureName();
  const std::string skybox_unit_name = GetSkyboxTextureUnitName();
  // Reuse the skybox texture
  texture_manager.BindTexture(skybox_tex_name, GL_TEXTURE_CUBE_MAP,
                              skybox_unit_name);
}

void shader::SceneShader::BindTextures() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
  const std::string skybox_tex_name = skybox_shader_->GetTextureName();
  const std::string skybox_unit_name = GetSkyboxTextureUnitName();

  // Use the program
  UseProgram();

  // Bind the skybox texture
  const GLuint skybox_unit_idx = texture_manager.GetUnitIdx(skybox_tex_name);
  texture_manager.BindTexture(skybox_tex_name, GL_TEXTURE_CUBE_MAP,
                              skybox_unit_name);
  uniform_manager.SetUniform1Int(program_name, "skybox_tex", skybox_unit_idx);
  // Bind the depth map texture
  const std::string depth_tex_name = depth_shader_->GetDepthTextureName();
  const std::string depth_unit_name = depth_shader_->GetDepthTextureUnitName();
  const GLuint depth_unit_idx = texture_manager.GetUnitIdx(depth_tex_name);
  texture_manager.BindTexture(depth_tex_name, GL_TEXTURE_2D, depth_unit_name);
  uniform_manager.SetUniform1Int(program_name, "depth_map_tex", depth_unit_idx);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::SceneShader::Draw() {
  // Get names
  const std::string quad_group_name = GetQuadGroupName();
  const std::string scene_group_name = GetSceneGroupName();
  // Get models
  const as::Model &quad_model = GetQuadModel();
  const as::Model &scene_model = GetSceneModel();

  // Use the program
  UseProgram();

  // Draw the quad
  model_material_.use_env_map = false;
  UpdateQuadModelTrans();
  UpdateQuadLighting();
  DrawModel(quad_model, quad_group_name);
  // Draw the scene
  model_material_.use_env_map = true;
  UpdateSceneModelTrans();
  UpdateSceneLighting();
  DrawModel(scene_model, scene_group_name);
}

void shader::SceneShader::DrawScene() {
  // Get names
  const std::string scene_group_name = GetSceneGroupName();
  // Get models
  const as::Model &scene_model = GetSceneModel();

  // Use the program
  UseProgram();

  // Update shadow
  UpdateShadow(true, true);

  // Draw the scene
  model_material_.use_env_map = true;
  UpdateSceneModelTrans();
  UpdateSceneLighting();
  DrawModel(scene_model, scene_group_name);
}

void shader::SceneShader::DrawQuad(const bool draw_shadow) {
  // Get names
  const std::string quad_group_name = GetQuadGroupName();
  // Get models
  const as::Model &quad_model = GetQuadModel();

  // Use the program
  UseProgram();

  // Update shadow
  UpdateShadow(false, draw_shadow);

  // Draw the quad
  model_material_.use_env_map = false;
  UpdateQuadModelTrans();
  UpdateQuadLighting();
  DrawModel(quad_model, quad_group_name);
}

void shader::SceneShader::UpdateQuadLighting() {
  lighting_.light_color = glm::vec3(1.0f, 1.0f, 1.0f);
  lighting_.light_pos = GetLightPos();
  // We have to use a value higher than 1.0 to counteract the diffuse strength
  // calculation with normal and light direction
  lighting_.light_intensity = glm::vec3(0.0f, 3.0f, 0.0f);

  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetLightingBufferName();
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateSceneLighting() {
  lighting_.light_color = glm::vec3(1.0f, 1.0f, 1.0f);
  lighting_.light_pos = GetLightPos();
  lighting_.light_intensity = glm::vec3(0.1f, 1.0f, 1.0f);

  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetLightingBufferName();
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

/*******************************************************************************
 * State Getters
 ******************************************************************************/

dto::GlobalTrans shader::SceneShader::GetGlobalTrans() const {
  return global_trans_;
}

glm::mat4 shader::SceneShader::GetQuadModelTrans() {
  const glm::vec3 scale_factors = 100.0f * glm::vec3(0.5f, 0.35f, 0.5f);
  const glm::vec3 translate_factors = glm::vec3(-10.0f, -13.0f, -8.0f);
  glm::mat4 trans = glm::translate(glm::mat4(1.0f), translate_factors);
  trans = glm::scale(trans, scale_factors);
  trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  return trans;
}

glm::mat4 shader::SceneShader::GetSceneModelTrans() {
  const glm::vec3 scale_factors = glm::vec3(0.5f, 0.35f, 0.5f);
  const glm::vec3 translate_factors = glm::vec3(-10.0f, -13.0f, -8.0f);
  glm::mat4 trans = glm::translate(glm::mat4(1.0f), translate_factors);
  trans = glm::scale(trans, scale_factors);
  trans = glm::rotate(trans, model_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
  return trans;
}

glm::vec3 shader::SceneShader::GetLightPos() const {
  return glm::vec3(-20.0f, 0.0f, 15.0f);
}

/*******************************************************************************
 * State Updaters
 ******************************************************************************/

void shader::SceneShader::UpdateGlobalTrans(
    const dto::GlobalTrans &global_trans) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetGlobalTransBufferName();
  // Update global transformation
  global_trans_ = global_trans;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
  // Update also the fixed normal model
  UpdateFixedNormModel();
}

void shader::SceneShader::UpdateQuadModelTrans() {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update model transformation
  model_trans_.trans = GetQuadModelTrans();
  // Update the buffer
  const std::string buffer_name = GetModelTransBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
  // Update also the fixed normal model
  UpdateFixedNormModel();
}

void shader::SceneShader::UpdateSceneModelTrans(const float add_rotation) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update model state
  model_rotation += add_rotation;
  // Update model transformation
  model_trans_.trans = GetSceneModelTrans();
  // Update the buffer
  const std::string buffer_name = GetModelTransBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
  // Update also the fixed normal model
  UpdateFixedNormModel();
}

void shader::SceneShader::UpdateModelMaterial(const as::Material &material) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update material
  model_material_.use_ambient_tex = material.HasAmbientTexture();
  model_material_.use_diffuse_tex = material.HasDiffuseTexture();
  model_material_.use_specular_tex = material.HasSpecularTexture();
  if (use_normal_height) {
    model_material_.use_height_tex = material.HasHeightTexture();
    model_material_.use_normals_tex = material.HasNormalsTexture();
  } else {
    model_material_.use_height_tex = false;
    model_material_.use_normals_tex = false;
  }
  model_material_.ambient_color = material.GetAmbientColor();
  model_material_.diffuse_color = material.GetDiffuseColor();
  model_material_.specular_color = material.GetSpecularColor();
  model_material_.shininess = material.GetShininess();
  // Update the buffer
  const std::string buffer_name = GetModelMaterialBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateViewPos(const glm::vec3 &view_pos) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update lighting
  lighting_.view_pos = view_pos;
  // Update the buffer
  const std::string buffer_name = GetLightingBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateShadow(const bool dim_orig_color,
                                       const bool draw_shadow) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update shadow
  shadow_.dim_orig_color = dim_orig_color;
  shadow_.draw_shadow = draw_shadow;
  // Update the buffer
  const std::string buffer_name = GetShadowBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::ToggleNormalHeight(const bool toggle) {
  use_normal_height = toggle;
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::SceneShader::GetId() const { return "scene"; }

std::string shader::SceneShader::GetSceneGroupName() const {
  return GetProgramName() + "/scene";
}

std::string shader::SceneShader::GetQuadGroupName() const {
  return GetProgramName() + "/quad";
}

std::string shader::SceneShader::GetGlobalTransBufferName() const {
  return GetProgramName() + "buffer/global_trans";
}

std::string shader::SceneShader::GetGlobalTransUniformBlockName() const {
  return "GlobalTrans";
}

/*******************************************************************************
 * GL Initializations (Protected)
 ******************************************************************************/

GLsizei shader::SceneShader::GetNumMipmapLevels() const { return 5; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::SceneShader::GetModelTransBufferName() const {
  return GetProgramName() + "model_trans";
}

std::string shader::SceneShader::GetModelMaterialBufferName() const {
  return GetProgramName() + "model_material";
}

std::string shader::SceneShader::GetLightingBufferName() const {
  return GetProgramName() + "lighting";
}

std::string shader::SceneShader::GetShadowBufferName() const {
  return GetProgramName() + "shadow";
}

std::string shader::SceneShader::GetSkyboxTextureUnitName() const {
  return GetProgramName() + "/skybox";
}

std::string shader::SceneShader::GetModelTransUniformBlockName() const {
  return "ModelTrans";
}

std::string shader::SceneShader::GetModelMaterialUniformBlockName() const {
  return "ModelMaterial";
}

std::string shader::SceneShader::GetLightingUniformBlockName() const {
  return "Lighting";
}

std::string shader::SceneShader::GetShadowUniformBlockName() const {
  return "Shadow";
}

/*******************************************************************************
 * State Updaters (Private)
 ******************************************************************************/

void shader::SceneShader::UpdateFixedNormModel() {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update fixed normal model
  const glm::mat4 model = global_trans_.model * model_trans_.trans;
  lighting_.fixed_norm_model =
      glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
  // Update the buffer
  const std::string buffer_name = GetLightingBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

/*******************************************************************************
 * GL Drawing Methods (Private)
 ******************************************************************************/

void shader::SceneShader::DrawModel(const as::Model &model,
                                    const std::string &group_name) {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
  // Get meshes
  const std::vector<as::Mesh> &meshes = model.GetMeshes();

  // Draw each mesh with its own texture
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Get the array indexes
    const std::vector<size_t> &idxs = mesh.GetIdxs();
    // Get the material
    const as::Material &material = mesh.GetMaterial();
    // Get the textures
    const std::set<as::Texture> &textures = material.GetTextures();
    /* Update Material Colors */
    UpdateModelMaterial(material);
    /* Update Textures */
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      const aiTextureType type = texture.GetType();
      // Bind the texture
      texture_manager.BindTexture(path);
      // Get the unit index
      const GLuint unit_idx = texture_manager.GetUnitIdx(path);
      // Set the texture handler to the unit index
      switch (type) {
        case aiTextureType_AMBIENT: {
          uniform_manager.SetUniform1Int(program_name, "ambient_tex", unit_idx);
        } break;
        case aiTextureType_DIFFUSE: {
          uniform_manager.SetUniform1Int(program_name, "diffuse_tex", unit_idx);
        } break;
        case aiTextureType_SPECULAR: {
          uniform_manager.SetUniform1Int(program_name, "specular_tex",
                                         unit_idx);
        } break;
        case aiTextureType_HEIGHT: {
          uniform_manager.SetUniform1Int(program_name, "height_tex", unit_idx);
        } break;
        case aiTextureType_NORMALS: {
          uniform_manager.SetUniform1Int(program_name, "normals_tex", unit_idx);
        } break;
        default: {
          throw std::runtime_error("Unknown texture type '" +
                                   std ::to_string(type) + "'");
        }
      }
    }
    /* Draw Vertex Arrays */
    UseMesh(group_name, mesh_idx);
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, nullptr);
  }
}
