#pragma once

#include "as/common.hpp"

#include "as/model/texture.hpp"

namespace as {
class Material {
 public:
  Material();

  Material(const glm::vec4 &ambient_color, const glm::vec4 &diffuse_color,
           const glm::vec4 &specular_color, const float shininess,
           const std::set<Texture> &textures);

  bool HasAmbientTexture() const;

  bool HasDiffuseTexture() const;

  bool HasSpecularTexture() const;

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

  bool HasTextureType(const aiTextureType type) const;
};
}  // namespace as
