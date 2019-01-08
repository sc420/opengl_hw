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
 * Model Getters
 ******************************************************************************/

const std::map<std::string, dto::SceneModel>
    &shader::SceneShader::GetSceneModels() const {
  return scene_models_;
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
  // Use the program
  UseProgram();

  for (const auto &pair : scene_models_) {
    const dto::SceneModel &scene_model = pair.second;
    // Get names
    const std::string group_name = scene_model.GetVertexArrayGroupName();

    // Update states
    UpdateModelTrans(scene_model);
    UpdateLighting(scene_model);
    UpdateModelMaterial(scene_model);
    // Draw the model
    UseInstancingTransform(scene_model);
    DrawModel(scene_model, group_name);
  }
}

/*******************************************************************************
 * State Getters
 ******************************************************************************/

dto::GlobalTrans shader::SceneShader::GetGlobalTrans() const {
  return global_trans_;
}

glm::vec3 shader::SceneShader::GetLightPos() const {
  return glm::vec3(-20.0f, 30.0f, 30.0f);
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

void shader::SceneShader::ToggleNormalHeight(const bool toggle) {
  use_normal_height = toggle;
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
  // Scene
  scene_models_["scene"] = dto::SceneModel(
      "scene", "assets/models/nanosuit/nanosuit.obj",
      aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices |
          aiProcess_Triangulate | aiProcess_GenNormals |
          aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials |
          aiProcess_OptimizeMeshes | aiProcess_FlipUVs,
      "scene", 5, gl_managers_);
  // Ground
  scene_models_["ground"] = dto::SceneModel(
      "ground", "assets/models/volcano-02-low/volcano 02_subdiv_01.obj",
      aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices |
          aiProcess_Triangulate | aiProcess_GenNormals |
          aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials |
          aiProcess_OptimizeMeshes | aiProcess_FlipUVs,
      "ground", 1, gl_managers_);
}

void shader::SceneShader::InitModels() {
  // Scene
  scene_models_.at("scene").SetTranslation(glm::vec3(-10.0f, 0.0f, -8.0f));
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
  scene_models_.at("ground").SetLightIntensity(glm::vec3(0.5f, 0.5f, 1.0f));
  scene_models_.at("ground").SetUseEnvMap(false);

  std::vector<glm::vec3> instancing_translations = {
      glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, 1.0f)};
  scene_models_.at("ground").SetInstancingTranslations(instancing_translations);

  std::vector<glm::vec3> instancing_rotations = {
      glm::vec3(0.0f), glm::vec3(glm::radians(90.0f), 0.0f, 0.0f),
      glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
      glm::vec3(0.0f, 0.0f, glm::radians(90.0f))};

  scene_models_.at("scene").SetInstancingTranslations(instancing_translations);
  scene_models_.at("scene").SetInstancingRotations(instancing_rotations);
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
    const std::string translatins_buffer_name =
        "buffer/instancing/translations/" + scene_model.GetId();
    const std::string rotations_buffer_name =
        "buffer/instancing/rotations/" + scene_model.GetId();
    const std::string scalings_buffer_name =
        "buffer/instancing/scalings/" + scene_model.GetId();

    /* Generate buffers */
    buffer_manager.GenBuffer(translatins_buffer_name);
    buffer_manager.GenBuffer(rotations_buffer_name);
    buffer_manager.GenBuffer(scalings_buffer_name);

    /* Initialize buffers */
    buffer_manager.InitBuffer(translatins_buffer_name, GL_ARRAY_BUFFER,
                              instancing_mem_size, nullptr, GL_STATIC_DRAW);
    buffer_manager.InitBuffer(rotations_buffer_name, GL_ARRAY_BUFFER,
                              instancing_mem_size, nullptr, GL_STATIC_DRAW);
    buffer_manager.InitBuffer(scalings_buffer_name, GL_ARRAY_BUFFER,
                              instancing_mem_size, nullptr, GL_STATIC_DRAW);

    /* Update buffers */
    buffer_manager.UpdateBuffer(translatins_buffer_name, GL_ARRAY_BUFFER, 0,
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
          va_name, translatins_buffer_name, 4, 0, sizeof(glm::vec3));
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
 * GL Drawing Methods (Private)
 ******************************************************************************/

void shader::SceneShader::UseInstancingTransform(
    const dto::SceneModel &scene_model) {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string translatins_buffer_name =
      "buffer/instancing/translations/" + scene_model.GetId();
  const std::string rotations_buffer_name =
      "buffer/instancing/rotations/" + scene_model.GetId();
  const std::string scalings_buffer_name =
      "buffer/instancing/scalings/" + scene_model.GetId();

  // Use the buffers
  buffer_manager.BindBuffer(translatins_buffer_name);
  buffer_manager.BindBuffer(rotations_buffer_name);
  buffer_manager.BindBuffer(scalings_buffer_name);
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

  // Update also the fixed normal model
  UpdateFixedNormModel();
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

void shader::SceneShader::DrawModel(const dto::SceneModel &scene_model,
                                    const std::string &group_name) {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
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
    glDrawElementsInstanced(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, nullptr,
                            scene_model.GetNumInstancing());
  }
}
