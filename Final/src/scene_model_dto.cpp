#include "scene_model_dto.hpp"

dto::SceneModel::SceneModel() {}

dto::SceneModel::SceneModel(const std::string &id, const std::string &path,
                            const unsigned int flags,
                            const std::string &tex_unit_group_name,
                            const GLsizei num_mipmap_levels,
                            as::GLManagers *gl_managers) {
  id_ = id;
  LoadFile(path, flags);
  InitTextures(tex_unit_group_name, num_mipmap_levels, gl_managers);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string dto::SceneModel::GetId() const { return id_; }

std::string dto::SceneModel::GetVertexArrayGroupName() const {
  return "vertex_array/group/" + id_;
}

/*******************************************************************************
 * Model Getters
 ******************************************************************************/

const as::Model &dto::SceneModel::GetModel() const { return model_; }

glm::vec3 dto::SceneModel::GetTranslation() const { return translation_; }

glm::vec3 dto::SceneModel::GetRotation() const { return rotation_; }

glm::vec3 dto::SceneModel::GetScaling() const { return scaling_; }

glm::vec3 dto::SceneModel::GetInstancingTranslation(
    const int instance_idx) const {
  if (instancing_translations_.empty()) {
    // Direct to model transformation
    return GetTranslation();
  } else {
    return instancing_translations_[instance_idx];
  }
}

glm::vec3 dto::SceneModel::GetInstancingRotation(const int instance_idx) const {
  if (instancing_rotations_.empty()) {
    // Direct to model transformation
    return GetRotation();
  } else {
    return instancing_rotations_[instance_idx];
  }
}

glm::vec3 dto::SceneModel::GetInstancingScaling(const int instance_idx) const {
  if (instancing_scalings_.empty()) {
    // Direct to model transformation
    return GetScaling();
  } else {
    return instancing_scalings_[instance_idx];
  }
}

/*******************************************************************************
 * State Setters
 ******************************************************************************/

void dto::SceneModel::SetTranslation(const glm::vec3 &translation) {
  translation_ = translation;
}

void dto::SceneModel::SetRotation(const glm::vec3 &rotation) {
  rotation_ = rotation;
}

void dto::SceneModel::SetScaling(const glm::vec3 &scaling) {
  scaling_ = scaling;
}

void dto::SceneModel::SetInstancingTranslations(
    const std::vector<glm::vec3> &translations) {
  instancing_translations_ = translations;
}

void dto::SceneModel::SetInstancingRotations(
    const std::vector<glm::vec3> &rotations) {
  instancing_rotations_ = rotations;
}

void dto::SceneModel::SetInstancingScalings(
    const std::vector<glm::vec3> &scalings) {
  instancing_scalings_ = scalings;
}

void dto::SceneModel::SetDefaultInstancingTranslations() {
  const glm::vec3 default_translation = glm::vec3(0.0f);
  instancing_translations_.assign(GetNumInstancing(), default_translation);
}

void dto::SceneModel::SetDefaultInstancingRotations() {
  const glm::vec3 default_rotation = glm::vec3(0.0f);
  instancing_rotations_.assign(GetNumInstancing(), default_rotation);
}

void dto::SceneModel::SetDefaultInstancingScalings() {
  const glm::vec3 default_scaling = glm::vec3(1.0f);
  instancing_scalings_.assign(GetNumInstancing(), default_scaling);
}

void dto::SceneModel::SetInstancingTranslation(const int instance_idx,
                                               const glm::vec3 &translation) {
  if (instancing_translations_.empty()) {
    // Direct to model transformation
    SetTranslation(translation);
  } else {
    instancing_translations_[instance_idx] = translation;
  }
}

void dto::SceneModel::SetInstancingRotation(const int instance_idx,
                                            const glm::vec3 &rotation) {
  if (instancing_rotations_.empty()) {
    // Direct to model transformation
    SetRotation(rotation);
  } else {
    instancing_rotations_[instance_idx] = rotation;
  }
}

void dto::SceneModel::SetInstancingScaling(const int instance_idx,
                                           const glm::vec3 &scaling) {
  if (instancing_scalings_.empty()) {
    // Direct to model transformation
    SetScaling(scaling);
  } else {
    instancing_scalings_[instance_idx] = scaling;
  }
}

void dto::SceneModel::SetLightPos(const glm::vec3 &light_pos) {
  light_pos_ = light_pos;
}

void dto::SceneModel::SetLightColor(const glm::vec3 &light_color) {
  light_color_ = light_color;
}

void dto::SceneModel::SetLightIntensity(const glm::vec3 &light_intensity) {
  light_intensity_ = light_intensity;
}

void dto::SceneModel::SetUseEnvMap(const bool use_env_map) {
  use_env_map_ = use_env_map;
}

/*******************************************************************************
 * State Getters
 ******************************************************************************/

glm::mat4 dto::SceneModel::GetTrans() const {
  return GetTransformMatrix(translation_, rotation_, scaling_);
}

std::vector<glm::vec3> dto::SceneModel::GetInstancingTranslations() const {
  if (instancing_translations_.empty()) {
    return GetDefaultInstancingTransforms(glm::vec3(0.0f));
  } else {
    return instancing_translations_;
  }
}

std::vector<glm::vec3> dto::SceneModel::GetInstancingRotations() const {
  if (instancing_rotations_.empty()) {
    return GetDefaultInstancingTransforms(glm::vec3(0.0f));
  } else {
    return instancing_rotations_;
  }
}

std::vector<glm::vec3> dto::SceneModel::GetInstancingScalings() const {
  if (instancing_scalings_.empty()) {
    return GetDefaultInstancingTransforms(glm::vec3(1.0f));
  } else {
    return instancing_scalings_;
  }
}

std::vector<glm::mat4> dto::SceneModel::GetInstancingTransforms() const {
  std::vector<glm::mat4> transforms;
  const std::vector<glm::vec3> translations = GetInstancingTranslations();
  const std::vector<glm::vec3> rotations = GetInstancingRotations();
  const std::vector<glm::vec3> scalings = GetInstancingScalings();
  for (size_t i = 0; i < translations.size(); i++) {
    transforms.push_back(
        GetTransformMatrix(translations[i], rotations[i], scalings[i]));
  }
  return transforms;
}

size_t dto::SceneModel::GetNumInstancing() const {
  size_t num = 0;
  if (!instancing_translations_.empty()) {
    num = instancing_translations_.size();
  }
  if (!instancing_rotations_.empty()) {
    if (num > 0 && num != instancing_rotations_.size()) {
      throw std::runtime_error(
          "All instancing transformations should have the same size");
    }
    num = instancing_rotations_.size();
  }
  if (!instancing_scalings_.empty()) {
    if (num > 0 && num != instancing_scalings_.size()) {
      throw std::runtime_error(
          "All instancing transformations should have the same size");
    }
    num = instancing_scalings_.size();
  }

  // Force the number to be at least 1
  if (num == 0) num = 1;

  return num;
}

size_t dto::SceneModel::GetInstancingMemSize() const {
  return GetNumInstancing() * sizeof(glm::vec3);
}

glm::vec3 dto::SceneModel::GetLightPos() const { return light_pos_; }

glm::vec3 dto::SceneModel::GetLightColor() const { return light_color_; }

glm::vec3 dto::SceneModel::GetLightIntensity() const {
  return light_intensity_;
}

bool dto::SceneModel::GetUseEnvMap() const { return use_env_map_; }

/*******************************************************************************
 * Model Initialization (Private)
 ******************************************************************************/

void dto::SceneModel::LoadFile(const std::string &path,
                               const unsigned int flags) {
  model_.LoadFile(path, flags);
}

/*******************************************************************************
 * GL Initialization (Private)
 ******************************************************************************/

void dto::SceneModel::InitTextures(const std::string &tex_unit_group_name,
                                   const GLsizei num_mipmap_levels,
                                   as::GLManagers *gl_managers) {
  // Get managers
  as::TextureManager &texture_manager = gl_managers->GetTextureManager();
  // Initialize textures in each mesh
  const std::vector<as::Mesh> &meshes = model_.GetMeshes();
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
      const std::string tex_unit_name =
          GetTextureUnitName(tex_unit_group_name, texture);
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
      texture_manager.BindTexture(path, GL_TEXTURE_2D, tex_unit_name);
      // Initialize the texture
      texture_manager.InitTexture2D(path, GL_TEXTURE_2D, num_mipmap_levels,
                                    GL_BGRA, width, height);
      // Update the texture
      texture_manager.UpdateTexture2D(path, GL_TEXTURE_2D, 0, 0, 0, width,
                                      height, GL_RGBA, GL_UNSIGNED_BYTE,
                                      texels.data());
      texture_manager.GenMipmap(path, GL_TEXTURE_2D);
      texture_manager.SetTextureParamInt(
          path, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D,
                                         GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                         GL_CLAMP_TO_EDGE);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                         GL_CLAMP_TO_EDGE);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,
                                         GL_CLAMP_TO_EDGE);
    }
  }
}

/*******************************************************************************
 * GL Drawing Methods (Private)
 ******************************************************************************/

glm::mat4 dto::SceneModel::GetTransformMatrix(const glm::vec3 &translation,
                                              const glm::vec3 &rotation,
                                              const glm::vec3 &scaling) const {
  const glm::mat4 identity = glm::mat4(1.0f);
  glm::mat4 trans = glm::mat4(1.0f);

  trans = glm::scale(identity, scaling) * trans;

  // Convert quaternion to rotation matrix
  const glm::quat quaternion = glm::quat(rotation);
  trans = glm::mat4_cast(quaternion) * trans;

  trans = glm::translate(identity, translation) * trans;

  return trans;
}

/*******************************************************************************
 * State Getters (Private)
 ******************************************************************************/

std::vector<glm::vec3> dto::SceneModel::GetDefaultInstancingTransforms(
    const glm::vec3 &default_transform) const {
  return std::vector<glm::vec3>(GetNumInstancing(), default_transform);
}

/*******************************************************************************
 * Name Management (Private)
 ******************************************************************************/

std::string dto::SceneModel::GetTextureUnitName(
    const std::string &tex_unit_group_name, const as::Texture &texture) const {
  return "texture_unit_name/" + tex_unit_group_name + "/type-" +
         std::to_string(texture.GetType());
}
