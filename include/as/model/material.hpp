#pragma once

#include "as/common.hpp"

#include "as/model/converter.hpp"
#include "as/model/texture.hpp"

namespace fs = std::experimental::filesystem;

namespace as {
class Material {
 public:
  Material();

  Material(const fs::path &dir, const aiScene *ai_scene, const aiMesh *ai_mesh);

  bool HasTextureType(const aiTextureType type) const;

  bool HasAmbientTexture() const;

  bool HasDiffuseTexture() const;

  bool HasSpecularTexture() const;

  bool HasHeightTexture() const;

  bool HasNormalsTexture() const;

  glm::vec4 GetAmbientColor() const;

  glm::vec4 GetDiffuseColor() const;

  glm::vec4 GetSpecularColor() const;

  float GetShininess() const;

  std::set<Texture> GetTextures() const;

 private:
  glm::vec4 ambient_color_;
  glm::vec4 diffuse_color_;
  glm::vec4 specular_color_;
  float shininess_;
  std::set<Texture> textures_;

  const std::set<Texture> ProcessMaterialTextures(
      const fs::path &dir, const aiMaterial *ai_material) const;

  std::set<Texture> ProcessMaterialTexturesOfType(
      const fs::path &dir, const aiMaterial *ai_material,
      const aiTextureType ai_texture_type) const;

  template <class TReturn, class TAiValue>
  const TReturn GetMaterialProperty(const aiMaterial *ai_material,
                                    const std::string &key) const;

  template <>
  const glm::vec4 GetMaterialProperty<glm::vec4, aiColor4D>(
      const aiMaterial *ai_material, const std::string &key) const;

  template <>
  const float GetMaterialProperty<float, float>(const aiMaterial *ai_material,
                                                const std::string &key) const;
};

template <>
inline const glm::vec4 Material::GetMaterialProperty<glm::vec4, aiColor4D>(
    const aiMaterial *ai_material, const std::string &key) const {
  aiColor4D value;
  if (ai_material->Get(key.c_str(), 0, 0, value) == AI_SUCCESS) {
    return ConvertAiColorToVec(value);
  } else {
    return glm::vec4(0.0f);
  }
}

template <>
inline const float Material::GetMaterialProperty<float, float>(
    const aiMaterial *ai_material, const std::string &key) const {
  float value;
  if (ai_material->Get(key.c_str(), 0, 0, value) == AI_SUCCESS) {
    return value;
  } else {
    return 0.0f;
  }
}

}  // namespace as
