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

void dto::SceneModel::SetTranslation(const glm::vec3 translation) {
  translation_ = translation;
}

void dto::SceneModel::SetScale(const glm::vec3 scale) { scale_ = scale; }

void dto::SceneModel::SetRotation(const glm::vec3 rotation) {
  rotation_ = rotation;
}

void dto::SceneModel::SetLightPos(const glm::vec3 light_pos) {
  light_pos_ = light_pos;
}

void dto::SceneModel::SetLightColor(const glm::vec3 light_color) {
  light_color_ = light_color;
}

void dto::SceneModel::SetLightIntensity(const glm::vec3 light_intensity) {
  light_intensity_ = light_intensity;
}

glm::mat4 dto::SceneModel::GetTrans() const {
  const glm::mat4 identity = glm::mat4(1.0f);
  glm::mat4 trans = glm::mat4(1.0f);

  trans = glm::scale(identity, scale_) * trans;

  // Convert quaternion to rotation matrix
  const glm::quat pitch =
      glm::angleAxis(rotation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const glm::quat yaw =
      glm::angleAxis(rotation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::quat roll =
      glm::angleAxis(rotation_.z, glm::vec3(0.0f, 0.0f, 1.0f));
  const glm::quat orientation = glm::normalize(pitch * yaw * roll);
  trans = glm::mat4_cast(orientation) * trans;

  trans = glm::translate(identity, translation_) * trans;

  return trans;
}

glm::vec3 dto::SceneModel::GetLightPos() const { return light_pos_; }

glm::vec3 dto::SceneModel::GetLightColor() const { return light_color_; }

glm::vec3 dto::SceneModel::GetLightIntensity() const {
  return light_intensity_;
}

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

/*******************************************************************************
 * Name Management (Private)
 ******************************************************************************/

std::string dto::SceneModel::GetTextureUnitName(
    const std::string &tex_unit_group_name, const as::Texture &texture) const {
  return "texture_unit_name/" + tex_unit_group_name + "/type-" +
         std::to_string(texture.GetType());
}
