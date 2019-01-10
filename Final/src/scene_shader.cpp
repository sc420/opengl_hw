#include "scene_shader.hpp"

shader::SceneShader::SceneShader()
    : model_rotation(glm::radians(0.0f)),
      global_trans_(dto::GlobalTrans()),
      model_trans_(dto::ModelTrans()),
      model_material_(ModelMaterial()),
      lighting_(Lighting()),
      use_instantiating_(true),
      use_normal_height_(true) {}

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
 * Model Getters
 ******************************************************************************/

const std::map<std::string, dto::SceneModel>
    &shader::SceneShader::GetSceneModels() const {
  return scene_models_;
}

dto::SceneModel &shader::SceneShader::GetSceneModel(
    const std::string &scene_model_name) {
  if (scene_models_.count(scene_model_name) == 0) {
    throw std::runtime_error("Could not find scene model name '" +
                             scene_model_name + "'");
  }
  return scene_models_.at(scene_model_name);
}

/*******************************************************************************
 * GL Initializations
 ******************************************************************************/

void shader::SceneShader::Init() {
  CreateShaders();
  CreatePrograms();
  LoadModels();
  InitModels();
  InitVertexArrays();
  InitInstancingVertexArrays();
  InitUniformBlocks();
  InitLightTrans();
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

  // Bind the skybox texture
  const GLuint skybox_unit_idx = texture_manager.GetUnitIdx(skybox_tex_name);
  texture_manager.BindTexture(skybox_tex_name, GL_TEXTURE_CUBE_MAP,
                              skybox_unit_name);
  uniform_manager.SetUniform1Int(program_name, "skybox_tex", skybox_unit_idx);

  // Bind the depth texture from light view
  const std::string light_depth_tex_name = depth_shader_->GetDepthTextureName(
      shader::DepthShader::DepthTextureTypes::kFromLight);
  const std::string light_depth_unit_name =
      depth_shader_->GetDepthTextureUnitName(
          shader::DepthShader::DepthTextureTypes::kFromLight);
  texture_manager.BindTexture(light_depth_tex_name, GL_TEXTURE_2D,
                              light_depth_unit_name);
  uniform_manager.SetUniform1Int(
      program_name, "light_depth_map_tex",
      texture_manager.GetUnitIdx(light_depth_tex_name));
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::SceneShader::Draw() {
  // Use the program
  UseProgram();

  for (const auto &pair : scene_models_) {
    const dto::SceneModel &scene_model = pair.second;

    // Check whether the scene model isn't visible
    if (!scene_model.IsVisible()) {
      continue;
    }

    // Update states
    UpdateModelTrans(scene_model);
    UpdateLighting(scene_model);
    UpdateModelMaterial(scene_model);
    // Draw the model
    DrawModel(scene_model);
  }
}

/*******************************************************************************
 * State Getters
 ******************************************************************************/

dto::GlobalTrans shader::SceneShader::GetGlobalTrans() const {
  return global_trans_;
}

glm::vec3 shader::SceneShader::GetLightPos() const {
  return glm::vec3(25.0f, 50.0f, 100.0f);
}

glm::vec3 shader::SceneShader::GetLightAngles() const {
  return glm::radians(glm::vec3(330.0f, 14.0f, 0.0f));
}

glm::mat4 shader::SceneShader::GetLightProjection() const {
  return glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 1e-3f, 1e3f);
}

float shader::SceneShader::GetMinDistanceToModel(
    const glm::vec3 &pos, const std::string &scene_model_name) const {
  const dto::SceneModel &scene_model = scene_models_.at(scene_model_name);
  const as::Model &model = scene_model.GetModel();
  const std::vector<as::Mesh> &meshes = model.GetMeshes();

  // Get model global transformations
  const glm::mat4 model_trans = scene_model.GetTrans();

  float min_dist = std::numeric_limits<float>::max();

  // Check each mesh
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    const std::vector<as::Vertex> &vertices = mesh.GetVertices();

    // Check each vertex
    for (size_t vertex_idx = 0; vertex_idx < vertices.size(); vertex_idx++) {
      const as::Vertex &vertex = vertices[vertex_idx];

      // Check each instancing transformations
      const std::vector<glm::mat4> instancing_transforms =
          scene_model.GetInstancingTransforms();
      for (const glm::mat4 &instancing_transform : instancing_transforms) {
        const glm::vec4 trans_pos =
            model_trans * instancing_transform * glm::vec4(vertex.pos, 1.0f);

        min_dist = glm::min(min_dist, glm::distance(pos, glm::vec3(trans_pos)));
      }
    }
  }

  return min_dist;
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
}

void shader::SceneShader::UpdateSceneModelTrans(const float add_rotation) {
  // Update model state
  model_rotation += add_rotation;
  scene_models_.at("scene").SetRotation(glm::vec3(0.0f, model_rotation, 0.0f));
}

void shader::SceneShader::UpdateViewPos(const glm::vec3 &view_pos) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update lighting
  lighting_.view_pos = view_pos;
  // Update the buffer
  const std::string buffer_name = GetLightingBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateSceneModel(const dto::SceneModel &scene_model) {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string translations_buffer_name =
      GetInstancingTranslationsBufferName(scene_model);
  const std::string rotations_buffer_name =
      GetInstancingRotationsBufferName(scene_model);
  const std::string scalings_buffer_name =
      GetInstancingScalingsBufferName(scene_model);
  // Get instancing transformations
  // TODO: Should use a DTO class
  const std::vector<glm::vec3> instancing_translations =
      scene_model.GetInstancingTranslations();
  const std::vector<glm::vec3> instancing_rotations =
      scene_model.GetInstancingRotations();
  const std::vector<glm::vec3> instancing_scalings =
      scene_model.GetInstancingScalings();
  // Get memory sizes
  const size_t instancing_mem_size = scene_model.GetInstancingMemSize();

  /* Update buffers */
  buffer_manager.UpdateBuffer(translations_buffer_name, GL_ARRAY_BUFFER, 0,
                              instancing_mem_size,
                              instancing_translations.data());
  buffer_manager.UpdateBuffer(rotations_buffer_name, GL_ARRAY_BUFFER, 0,
                              instancing_mem_size, instancing_rotations.data());
  buffer_manager.UpdateBuffer(scalings_buffer_name, GL_ARRAY_BUFFER, 0,
                              instancing_mem_size, instancing_scalings.data());
}

void shader::SceneShader::TogglePcf(const bool toggle) {
  model_material_.use_pcf = toggle;
}

void shader::SceneShader::ToggleInstantiating(const bool toggle) {
  use_instantiating_ = toggle;
}

void shader::SceneShader::ToggleNormalHeight(const bool toggle) {
  use_normal_height_ = toggle;
  model_material_.use_normal = toggle;
}

void shader::SceneShader::ToggleFog(const bool toggle) {
  model_material_.use_fog = toggle;
}

void shader::SceneShader::ToggleMixFogWithSkybox(const bool toggle) {
  model_material_.mix_fog_with_skybox = toggle;
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::SceneShader::GetId() const { return "scene"; }

std::string shader::SceneShader::GetGlobalTransBufferName() const {
  return GetProgramName() + "/buffer/global_trans";
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

std::string shader::SceneShader::GetInstancingTranslationsBufferName(
    const dto::SceneModel &scene_model) const {
  return "buffer/instancing/translations/" + scene_model.GetId();
}

std::string shader::SceneShader::GetInstancingRotationsBufferName(
    const dto::SceneModel &scene_model) const {
  return "buffer/instancing/rotations/" + scene_model.GetId();
}

std::string shader::SceneShader::GetInstancingScalingsBufferName(
    const dto::SceneModel &scene_model) const {
  return "buffer/instancing/scalings/" + scene_model.GetId();
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

/*******************************************************************************
 * Model Initialization (Private)
 ******************************************************************************/

void shader::SceneShader::LoadModels() {
  const unsigned int flags =
      aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices |
      aiProcess_Triangulate | aiProcess_GenNormals |
      aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials |
      aiProcess_OptimizeMeshes | aiProcess_FlipUVs;

  // Scene
  scene_models_["scene"] =
      dto::SceneModel("scene", "assets/models/nanosuit/nanosuit.obj", flags,
                      "scene", 3, gl_managers_);
  // Ground
  scene_models_["ground"] = dto::SceneModel(
      "ground", "assets/models/volcano-02-low/volcano 02_subdiv_01.obj", flags,
      "ground", 3, gl_managers_);
  // Surrounding mountains
  scene_models_["surround"] =
      dto::SceneModel("surround",
                      "assets/models/MountainsGreen0070/"
                      "tube.obj",
                      flags, "surround", 3, gl_managers_);
  // Industrial building
  scene_models_["industrial_building"] = dto::SceneModel(
      "industrial_building",
      "assets/models/industrial_building_1/industrial_building_1.obj", flags,
      "industrial_building", 3, gl_managers_);
  // Oil tank
  scene_models_["oil_tank"] =
      dto::SceneModel("oil_tank", "assets/models/oil_tank/big_cistern.obj",
                      flags, "oil_tank", 3, gl_managers_);
  // Electric tower
  scene_models_["tower"] =
      dto::SceneModel("tower", "assets/models/tower/tower.obj", flags, "tower",
                      3, gl_managers_);
}

void shader::SceneShader::InitModels() {
  // Scene (Used for debugging)
  // scene_models_.at("scene").SetTranslation(glm::vec3(-10.0f, 0.0f, -8.0f));
  scene_models_.at("scene").SetTranslation(glm::vec3(0.0f, -10.0f, 0.0f));
  scene_models_.at("scene").SetRotation(glm::vec3(0.0f));
  scene_models_.at("scene").SetScaling(glm::vec3(0.5f, 0.35f, 0.5f));
  scene_models_.at("scene").SetLightPos(GetLightPos());
  scene_models_.at("scene").SetLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
  scene_models_.at("scene").SetLightIntensity(glm::vec3(0.1f, 1.0f, 1.0f));
  scene_models_.at("scene").SetUseEnvMap(true);
  // Ground
  scene_models_.at("ground").SetTranslation(glm::vec3(0.0f));
  scene_models_.at("ground").SetRotation(glm::vec3(0.0f));
  scene_models_.at("ground").SetScaling(glm::vec3(1e-4f));
  scene_models_.at("ground").SetLightPos(GetLightPos());
  scene_models_.at("ground").SetLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
  scene_models_.at("ground").SetLightIntensity(glm::vec3(0.5f, 0.7f, 1.0f));
  scene_models_.at("ground").SetUseEnvMap(false);
  // Surrounding mountains
  scene_models_.at("surround").SetTranslation(glm::vec3(0.0f));
  scene_models_.at("surround").SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
  scene_models_.at("surround").SetScaling(glm::vec3(0.4f, 0.15f, 0.4f));
  scene_models_.at("surround").SetLightPos(GetLightPos());
  scene_models_.at("surround").SetLightColor(glm::vec3(1.0f));
  scene_models_.at("surround").SetLightIntensity(glm::vec3(0.3f, 0.0f, 0.0f));
  scene_models_.at("surround").SetUseEnvMap(false);
  // Industrial building
  scene_models_.at("industrial_building")
      .SetTranslation(glm::vec3(12.8f, 0.7f, 15.8f));
  scene_models_.at("industrial_building")
      .SetRotation(glm::vec3(0.0f, 0.7f, 0.0f));
  scene_models_.at("industrial_building").SetScaling(1.4e-4f * glm::vec3(1.0f));
  scene_models_.at("industrial_building").SetLightPos(GetLightPos());
  scene_models_.at("industrial_building").SetLightColor(glm::vec3(1.0f));
  scene_models_.at("industrial_building")
      .SetLightIntensity(glm::vec3(0.5f, 0.5f, 0.5f));
  scene_models_.at("industrial_building").SetUseEnvMap(false);
  // Oil tank
  scene_models_.at("oil_tank").SetTranslation(glm::vec3(12.5f, 0.8f, 16.2f));
  scene_models_.at("oil_tank").SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
  scene_models_.at("oil_tank").SetScaling(2e-4f * glm::vec3(1.0f));
  scene_models_.at("oil_tank").SetLightPos(GetLightPos());
  scene_models_.at("oil_tank").SetLightColor(glm::vec3(1.0f));
  scene_models_.at("oil_tank").SetLightIntensity(glm::vec3(0.5f, 0.5f, 0.5f));
  scene_models_.at("oil_tank").SetUseEnvMap(false);
  // Electric tower
  scene_models_.at("tower").SetTranslation(glm::vec3(13.6f, 1.8f, -3.8f));
  scene_models_.at("tower").SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
  scene_models_.at("tower").SetScaling(5e-2f * glm::vec3(1.0f));
  scene_models_.at("tower").SetLightPos(GetLightPos());
  scene_models_.at("tower").SetLightColor(glm::vec3(1.0f));
  scene_models_.at("tower").SetLightIntensity(glm::vec3(0.5f, 0.5f, 0.5f));
  scene_models_.at("tower").SetUseEnvMap(false);

  /* Instancing */

  // Ground
  scene_models_.at("ground").SetInstancingTranslations(std::vector<glm::vec3>{
      glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1e4f * -45.0f),
      glm::vec3(0.0f, 0.0f, 1e4f * 45.0f), glm::vec3(1e4f * -45.0f, 0.0f, 0.0f),
      glm::vec3(1e4f * 45.0f, 0.0f, 0.0f),
      glm::vec3(1e4f * -45.0f, 0.0f, 1e4f * -45.0f),
      glm::vec3(1e4f * -45.0f, 0.0f, 1e4f * 45.0f),
      glm::vec3(1e4f * 45.0f, 0.0f, 1e4f * -45.0f),
      glm::vec3(1e4f * 45.0f, 0.0f, 1e4f * 45.0f)});
  // Oil tank
  scene_models_.at("oil_tank")
      .SetInstancingTranslations(std::vector<glm::vec3>{
          glm::vec3(0.0f), glm::vec3(0.6f, 0.27f, 0.5f) / 2e-4f,
          glm::vec3(1.0f, 0.22f, 0.3f) / 2e-4f});
  // Electric tower
  scene_models_.at("tower").SetInstancingTranslations(std::vector<glm::vec3>{
      glm::vec3(-24.1f, -5.4f, -373.4f), glm::vec3(-0.7f, -6.4f, -301.3f),
      glm::vec3(5.1f, -6.1f, -222.2f), glm::vec3(-4.8f, -11.7f, -148.5f),
      glm::vec3(8.0f, -10.0f, -78.7f), glm::vec3(27.8f, 1.2f, -12.5f),
      glm::vec3(28.5f, 2.2f, 67.3f), glm::vec3(23.4f, -0.8f, 139.7f),
      glm::vec3(13.9f, -8.3f, 211.9f), glm::vec3(12.0f, -11.8f, 285.7f),
      glm::vec3(15.7f, -17.0f, 365.5f), glm::vec3(-7.6f, -17.2f, 439.4f),
      glm::vec3(-58.2f, -21.3f, 486.7f)});
  scene_models_.at("tower").SetInstancingRotations(std::vector<glm::vec3>{
      glm::vec3(-0.08f, 0.46f, 0.0f), glm::vec3(0.04f, 0.2f, 0.0f),
      glm::vec3(-0.03f, 0.03f, 0.11f), glm::vec3(0.14f, -0.2f, -0.13f),
      glm::vec3(-0.23f, 0.63f, 0.09f), glm::vec3(-0.02f, 0.04f, -0.21f),
      glm::vec3(-0.04f, -0.16f, -0.12f), glm::vec3(0.13f, 0.0f, -0.11f),
      glm::vec3(0.05f, -0.18f, -0.12f), glm::vec3(0.07f, 0.26f, -0.04f),
      glm::vec3(0.0f, -0.14f, 0.03f), glm::vec3(-0.01f, -0.39f, 0.03f),
      glm::vec3(-0.25f, -1.29f, 0.04f)});
}

/*******************************************************************************
 * GL Initialization (Private)
 ******************************************************************************/

void shader::SceneShader::InitVertexArrays() {
  for (const auto &pair : scene_models_) {
    const dto::SceneModel &scene_model = pair.second;
    InitVertexArray(scene_model.GetVertexArrayGroupName(),
                    scene_model.GetModel());
  }
}

void shader::SceneShader::InitInstancingVertexArrays() {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  as::VertexSpecManager &vertex_spec_manager =
      gl_managers_->GetVertexSpecManager();

  for (const auto &pair : scene_models_) {
    const dto::SceneModel &scene_model = pair.second;

    // Get model
    const as::Model &model = scene_model.GetModel();
    // Get meshes
    const std::vector<as::Mesh> &meshes = model.GetMeshes();
    // Get instancing transformations
    // TODO: Should use a DTO class
    const std::vector<glm::vec3> instancing_translations =
        scene_model.GetInstancingTranslations();
    const std::vector<glm::vec3> instancing_rotations =
        scene_model.GetInstancingRotations();
    const std::vector<glm::vec3> instancing_scalings =
        scene_model.GetInstancingScalings();
    // Get memory sizes
    const size_t instancing_mem_size = scene_model.GetInstancingMemSize();
    // Get names
    const std::string group_name = scene_model.GetVertexArrayGroupName();
    const std::string translations_buffer_name =
        GetInstancingTranslationsBufferName(scene_model);
    const std::string rotations_buffer_name =
        GetInstancingRotationsBufferName(scene_model);
    const std::string scalings_buffer_name =
        GetInstancingScalingsBufferName(scene_model);

    /* Generate buffers */
    buffer_manager.GenBuffer(translations_buffer_name);
    buffer_manager.GenBuffer(rotations_buffer_name);
    buffer_manager.GenBuffer(scalings_buffer_name);

    /* Initialize buffers */
    buffer_manager.InitBuffer(translations_buffer_name, GL_ARRAY_BUFFER,
                              instancing_mem_size, nullptr, GL_STATIC_DRAW);
    buffer_manager.InitBuffer(rotations_buffer_name, GL_ARRAY_BUFFER,
                              instancing_mem_size, nullptr, GL_STATIC_DRAW);
    buffer_manager.InitBuffer(scalings_buffer_name, GL_ARRAY_BUFFER,
                              instancing_mem_size, nullptr, GL_STATIC_DRAW);

    /* Update buffers */
    buffer_manager.UpdateBuffer(translations_buffer_name, GL_ARRAY_BUFFER, 0,
                                instancing_mem_size,
                                instancing_translations.data());
    buffer_manager.UpdateBuffer(rotations_buffer_name, GL_ARRAY_BUFFER, 0,
                                instancing_mem_size,
                                instancing_rotations.data());
    buffer_manager.UpdateBuffer(scalings_buffer_name, GL_ARRAY_BUFFER, 0,
                                instancing_mem_size,
                                instancing_scalings.data());

    // Apply to all meshes
    for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
      const std::string va_name = GetMeshVertexArrayName(group_name, mesh_idx);

      /* Bind vertex arrays to buffers */
      vertex_spec_manager.SpecifyVertexArrayOrg(va_name, 4, 3, GL_FLOAT,
                                                GL_FALSE, 0);
      vertex_spec_manager.SpecifyVertexArrayOrg(va_name, 5, 3, GL_FLOAT,
                                                GL_FALSE, 0);
      vertex_spec_manager.SpecifyVertexArrayOrg(va_name, 6, 3, GL_FLOAT,
                                                GL_FALSE, 0);

      vertex_spec_manager.AssocVertexAttribToBindingPoint(va_name, 4, 4);
      vertex_spec_manager.AssocVertexAttribToBindingPoint(va_name, 5, 5);
      vertex_spec_manager.AssocVertexAttribToBindingPoint(va_name, 6, 6);

      vertex_spec_manager.BindBufferToBindingPoint(
          va_name, translations_buffer_name, 4, 0, sizeof(glm::vec3));
      vertex_spec_manager.BindBufferToBindingPoint(
          va_name, rotations_buffer_name, 5, 0, sizeof(glm::vec3));
      vertex_spec_manager.BindBufferToBindingPoint(
          va_name, scalings_buffer_name, 6, 0, sizeof(glm::vec3));

      /* Modify vertex array updating rates */
      glVertexAttribDivisor(4, 1);
      glVertexAttribDivisor(5, 1);
      glVertexAttribDivisor(6, 1);
    }
  }
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

/*******************************************************************************
 * State Updaters (Private)
 ******************************************************************************/

void shader::SceneShader::UpdateModelTrans(const dto::SceneModel &scene_model) {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetModelTransBufferName();

  // Update transformation
  model_trans_.trans = scene_model.GetTrans();
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateLighting(const dto::SceneModel &scene_model) {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetLightingBufferName();

  // Update lighting
  lighting_.light_pos = scene_model.GetLightPos();
  lighting_.light_color = scene_model.GetLightColor();
  lighting_.light_intensity = scene_model.GetLightIntensity();

  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateModelMaterial(
    const dto::SceneModel &scene_model) {
  // The buffer will be updated in the draw method
  model_material_.use_env_map = scene_model.GetUseEnvMap();
}

void shader::SceneShader::UpdateModelMaterial(const as::Material &material) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update material
  model_material_.use_ambient_tex = material.HasAmbientTexture();
  model_material_.use_diffuse_tex = material.HasDiffuseTexture();
  model_material_.use_specular_tex = material.HasSpecularTexture();
  if (use_normal_height_) {
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

/*******************************************************************************
 * GL Drawing Methods (Private)
 ******************************************************************************/

void shader::SceneShader::DrawModel(const dto::SceneModel &scene_model) {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
  const std::string group_name = scene_model.GetVertexArrayGroupName();
  // Get model
  const as::Model &model = scene_model.GetModel();
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
    if (use_instantiating_) {
      glDrawElementsInstanced(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT,
                              nullptr, scene_model.GetNumInstancing());
    } else {
      glDrawElementsInstanced(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT,
                              nullptr, 1);
    }
  }
}
