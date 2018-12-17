#include "as/model/material.hpp"

as::Material::Material()
    : ambient_color_(glm::vec4(0.0f)),
      diffuse_color_(glm::vec4(0.0f)),
      specular_color_(glm::vec4(0.0f)),
      textures_(std::set<Texture>()) {}

as::Material::Material(const glm::vec4& ambient_color,
                       const glm::vec4& diffuse_color,
                       const glm::vec4& specular_color,
                       const std::set<Texture>& textures)
    : ambient_color_(ambient_color),
      diffuse_color_(diffuse_color),
      specular_color_(specular_color),
      textures_(textures) {}

glm::vec4 as::Material::GetAmbientColor() const { return ambient_color_; }

glm::vec4 as::Material::GetDiffuseColor() const { return diffuse_color_; }

glm::vec4 as::Material::GetSpecularColor() const { return specular_color_; }

std::set<as::Texture> as::Material::GetTextures() const { return textures_; }
