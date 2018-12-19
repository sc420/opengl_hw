#include "as/model/material.hpp"

as::Material::Material()
    : ambient_color_(glm::vec4(1.0f)),
      diffuse_color_(glm::vec4(1.0f)),
      specular_color_(glm::vec4(1.0f)),
      shininess_(1.0f) {}

as::Material::Material(const fs::path &dir, const aiScene *ai_scene,
                       const aiMesh *ai_mesh) {
  if (ai_mesh->mMaterialIndex < 0) {
    return;
  }
  // Get the material
  const aiMaterial *ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
  // Get the material colors
  ambient_color_ =
      GetMaterialProperty<glm::vec4, aiColor4D>(ai_material, "$clr.ambient");
  diffuse_color_ =
      GetMaterialProperty<glm::vec4, aiColor4D>(ai_material, "$clr.diffuse");
  specular_color_ =
      GetMaterialProperty<glm::vec4, aiColor4D>(ai_material, "$clr.specular");
  // Get the material shininess
  shininess_ = GetMaterialProperty<float, float>(ai_material, "$mat.shininess");
  // Get the material textures
  textures_ = ProcessMaterialTextures(dir, ai_material);
}

bool as::Material::HasTextureType(const aiTextureType type) const {
  for (const Texture &texture : textures_) {
    if (texture.GetType() == type) {
      return true;
    }
  }
  return false;
}

bool as::Material::HasAmbientTexture() const {
  return HasTextureType(aiTextureType_AMBIENT);
}

bool as::Material::HasDiffuseTexture() const {
  return HasTextureType(aiTextureType_DIFFUSE);
}

bool as::Material::HasSpecularTexture() const {
  return HasTextureType(aiTextureType_SPECULAR);
}

bool as::Material::HasNormalsTexture() const {
  return HasTextureType(aiTextureType_NORMALS);
}

glm::vec4 as::Material::GetAmbientColor() const { return ambient_color_; }

glm::vec4 as::Material::GetDiffuseColor() const { return diffuse_color_; }

glm::vec4 as::Material::GetSpecularColor() const { return specular_color_; }

float as::Material::GetShininess() const { return shininess_; }

std::set<as::Texture> as::Material::GetTextures() const { return textures_; }

const std::set<as::Texture> as::Material::ProcessMaterialTextures(
    const fs::path &dir, const aiMaterial *ai_material) const {
  std::set<Texture> diffuse_textures =
      ProcessMaterialTexturesOfType(dir, ai_material, aiTextureType_DIFFUSE);
  std::set<Texture> specular_textures =
      ProcessMaterialTexturesOfType(dir, ai_material, aiTextureType_SPECULAR);
  std::set<Texture> ambient_textures =
      ProcessMaterialTexturesOfType(dir, ai_material, aiTextureType_AMBIENT);
  std::set<Texture> height_textures =
      ProcessMaterialTexturesOfType(dir, ai_material, aiTextureType_HEIGHT);
  std::set<Texture> normals_textures =
      ProcessMaterialTexturesOfType(dir, ai_material, aiTextureType_NORMALS);
  // Merge all textures
  std::set<Texture> all_textures(diffuse_textures);
  all_textures.insert(specular_textures.begin(), specular_textures.end());
  all_textures.insert(ambient_textures.begin(), ambient_textures.end());
  all_textures.insert(height_textures.begin(), height_textures.end());
  all_textures.insert(normals_textures.begin(), normals_textures.end());
  return all_textures;
}

std::set<as::Texture> as::Material::ProcessMaterialTexturesOfType(
    const fs::path &dir, const aiMaterial *ai_material,
    const aiTextureType ai_texture_type) const {
  std::set<Texture> textures;
  for (size_t i = 0; i < ai_material->GetTextureCount(ai_texture_type); i++) {
    // Get the relative path
    aiString path;
    ai_material->GetTexture(ai_texture_type, i, &path);
    // Build the full path
    const fs::path full_path = dir / fs::path(path.C_Str());
    // Get type name
    const Texture texture = Texture(full_path.string(), ai_texture_type);
    textures.insert(texture);
  }
  return textures;
}
