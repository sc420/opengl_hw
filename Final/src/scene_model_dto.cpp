#include "scene_model_dto.hpp"

dto::SceneModel::SceneModel() : model_(as::Model()) {}

dto::SceneModel::SceneModel(const std::string &path, const unsigned int flags,
                            const std::string &tex_unit_group_name,
                            const GLsizei num_mipmap_levels,
                            as::GLManagers *gl_managers) {
  LoadFile(path, flags);
  InitTextures(tex_unit_group_name, num_mipmap_levels, gl_managers);
}

const as::Model &dto::SceneModel::GetModel() const { return model_; }

void dto::SceneModel::LoadFile(const std::string &path,
                               const unsigned int flags) {
  model_.LoadFile(path, flags);
}

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

std::string dto::SceneModel::GetTextureUnitName(
    const std::string &tex_unit_group_name, const as::Texture &texture) const {
  return "texture_unit_name/" + tex_unit_group_name + "/type-" +
         std::to_string(texture.GetType());
}
