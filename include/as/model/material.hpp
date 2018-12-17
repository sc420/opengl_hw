#pragma once

#include "as/common.hpp"

#include "as/model/texture.hpp"

namespace as {
class Material {
 public:
  Material();

  Material(const glm::vec3 &ambient_color, const glm::vec3 &diffuse_color,
           const glm::vec3 &specular_color, const std::set<Texture> &textures);

  glm::vec3 GetAmbientColor() const;

  glm::vec3 GetDiffuseColor() const;

  glm::vec3 GetSpecularColor() const;

  std::set<Texture> GetTextures() const;

 private:
  glm::vec3 ambient_color_;
  glm::vec3 diffuse_color_;
  glm::vec3 specular_color_;
  std::set<Texture> textures_;
};
}  // namespace as
