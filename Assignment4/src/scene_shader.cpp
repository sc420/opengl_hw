#include "scene_shader.hpp"

shader::SceneShader::SceneShader()
    : model_rotation(glm::radians(0.0f)),
      global_trans_(GlobalTrans()),
      model_trans_(ModelTrans()),
      model_material_(ModelMaterial()),
      lighting_(Lighting()) {}

/*******************************************************************************
 * Shader Registrations
 ******************************************************************************/

void shader::SceneShader::RegisterSkyboxShader(
    const SkyboxShader &skybox_shader) {
  skybox_shader_ = std::make_shared<SkyboxShader>(skybox_shader);
}

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

void shader::SceneShader::LoadModel() {
  as::Model &model = GetModel();
  model.LoadFile("assets/models/wall/wall.obj",
                 aiProcess_CalcTangentSpace | aiProcess_Triangulate |
                     aiProcess_GenNormals | aiProcess_FlipUVs);
}

/*******************************************************************************
 * GL Initializations
 ******************************************************************************/

void shader::SceneShader::Init() {
  Shader::Init();
  LoadModel();
  InitLighting();
  InitVertexArrays();
  InitUniformBlocks();
  InitTextures();
}

void shader::SceneShader::InitVertexArrays() {
  const as::Model &model = GetModel();
  InitVertexArray(model);
}

void shader::SceneShader::InitUniformBlocks() {
  LinkDataToUniformBlock(GetGlobalTransBufferName(),
                         GetGlobalTransUniformBlockName(), global_trans_);
  LinkDataToUniformBlock(GetModelTransBufferName(),
                         GetModelTransUniformBlockName(), model_trans_);
  LinkDataToUniformBlock(GetLightingBufferName(), GetLightingUniformBlockName(),
                         lighting_);
  LinkDataToUniformBlock(GetModelMaterialBufferName(),
                         GetModelMaterialUniformBlockName(), model_material_);
}

void shader::SceneShader::InitTextures() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get models
  const as::Model &model = GetModel();
  // Initialize textures in each mesh
  const std::vector<as::Mesh> &meshes = model.GetMeshes();
  for (const as::Mesh &mesh : meshes) {
    const as::Material &material = mesh.GetMaterial();
    const std::set<as::Texture> &textures = material.GetTextures();
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      // Check if the texture has been loaded
      if (texture_manager.HasTexture(path)) {
        continue;
      }
      // Get names
      const std::string &unit_name = GetTextureUnitName(texture);
      // Load the texture
      GLsizei width, height;
      int comp;
      std::vector<GLubyte> texels;
      as::LoadTextureByStb(path, 0, width, height, comp, texels);
      // Convert the texels to 4 channels to avoid GL errors
      texels = as::ConvertDataChannels(comp, 4, texels);
      // Generate the texture
      texture_manager.GenTexture(path);
      // Bind the texture
      texture_manager.BindTexture(path, GL_TEXTURE_2D, unit_name);
      // Initialize the texture
      const GLsizei num_mipmap_levels = GetNumMipmapLevels();
      texture_manager.InitTexture2D(path, GL_TEXTURE_2D, num_mipmap_levels,
                                    GL_RGBA8, width, height);
      // Update the texture
      texture_manager.UpdateTexture2D(path, GL_TEXTURE_2D, 0, 0, 0, width,
                                      height, GL_RGBA, GL_UNSIGNED_BYTE,
                                      texels.data());
      texture_manager.GenMipmap(path, GL_TEXTURE_2D);
      texture_manager.SetTextureParamInt(
          path, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D,
                                         GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
  }
}

void shader::SceneShader::InitLighting() {
  Lighting lighting;
  lighting.light_color = glm::vec3(1.0f, 1.0f, 1.0f);
  lighting.light_pos = glm::vec3(-31.75f, 26.05f, -97.72);
  lighting.light_intensity = glm::vec3(0.0f, 1.0f, 0.5f);
  lighting_ = lighting;
}

void shader::SceneShader::SetSkyboxTexture() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names
  const std::string &skybox_tex_name = skybox_shader_->GetTextureName();
  const std::string &skybox_unit_name = GetSkyboxTextureUnitName();
  // Reuse the skybox texture
  texture_manager.BindTexture(skybox_tex_name, GL_TEXTURE_CUBE_MAP,
                              skybox_unit_name);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::SceneShader::Draw() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();
  const std::string &skybox_tex_name = skybox_shader_->GetTextureName();
  const std::string &skybox_unit_name = GetSkyboxTextureUnitName();
  // Get models
  as::Model &model = GetModel();
  // Get meshes
  const std::vector<as::Mesh> &meshes = model.GetMeshes();

  // Use the program
  UseProgram();
  // Draw each mesh with its own texture
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Get names
    const std::string scene_va_name = GetMeshVertexArrayName(mesh_idx);
    const std::string scene_buffer_name =
        GetMeshVertexArrayBufferName(mesh_idx);
    const std::string scene_idxs_buffer_name =
        GetMeshVertexArrayIdxsBufferName(mesh_idx);
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
      /* Draw Vertex Arrays */
      UseMesh(mesh_idx);
      glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, nullptr);
    }
  }

  // Bind the skybox texture
  const GLuint skybox_unit_idx = texture_manager.GetUnitIdx(skybox_tex_name);
  texture_manager.BindTexture(skybox_tex_name, GL_TEXTURE_CUBE_MAP,
                              skybox_unit_name);
  uniform_manager.SetUniform1Int(GetProgramName(), "skybox_tex",
                                 skybox_unit_idx);
}

/*******************************************************************************
 * State Updaters
 ******************************************************************************/

void shader::SceneShader::UpdateGlobalTrans(const GlobalTrans &global_trans) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string &buffer_name = GetGlobalTransBufferName();
  // Update global transformation
  global_trans_ = global_trans;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
  // Update also the fixed normal model
  UpdateFixedNormModel();
}

void shader::SceneShader::UpdateModelTrans(const float add_rotation) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update model state
  model_rotation += add_rotation;
  // Update model transformation
  const glm::vec3 scale_factors = glm::vec3(0.5f, 0.35f, 0.5f);
  const glm::vec3 translate_factors = glm::vec3(-10.0f, -13.0f, -8.0f);
  glm::mat4 trans = glm::scale(glm::mat4(1.0f), scale_factors);
  trans = glm::translate(trans, translate_factors);
  trans = glm::rotate(trans, model_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
  model_trans_.trans = trans;
  // Update the buffer
  const std::string &buffer_name = GetModelTransBufferName();
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
  model_material_.use_height_tex = material.HasHeightTexture();
  model_material_.use_normals_tex = material.HasNormalsTexture();
  model_material_.ambient_color = material.GetAmbientColor();
  model_material_.diffuse_color = material.GetDiffuseColor();
  model_material_.specular_color = material.GetSpecularColor();
  model_material_.shininess = material.GetShininess();
  // Update the buffer
  const std::string &buffer_name = GetModelMaterialBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateViewPos(const glm::vec3 &view_pos) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update lighting
  lighting_.view_pos = view_pos;
  // Update the buffer
  const std::string &buffer_name = GetLightingBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::SceneShader::GetId() const { return "scene"; }

std::string shader::SceneShader::GetGlobalTransBufferName() const {
  return "global_trans";
}

std::string shader::SceneShader::GetGlobalTransUniformBlockName() const {
  return "GlobalTrans";
}

/*******************************************************************************
 * Model Handlers (Protected)
 ******************************************************************************/

as::Model &shader::SceneShader::GetModel() { return scene_model_; }

/*******************************************************************************
 * GL Initializations (Protected)
 ******************************************************************************/

GLsizei shader::SceneShader::GetNumMipmapLevels() const { return 5; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::SceneShader::GetModelTransBufferName() const {
  return "model_trans";
}

std::string shader::SceneShader::GetModelMaterialBufferName() const {
  return "model_material";
}

std::string shader::SceneShader::GetLightingBufferName() const {
  return "lighting";
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

std::string shader::SceneShader::GetTextureUnitName(
    const as::Texture &texture) const {
  return GetProgramName() + "/type[" + std::to_string(texture.GetType()) + "]";
}

std::string shader::SceneShader::GetSkyboxTextureUnitName() const {
  return GetProgramName() + "/skybox";
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
  const std::string &buffer_name = GetLightingBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}
