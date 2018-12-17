#include "as/model/texture.hpp"

as::Texture::Texture(const std::string &path, const aiTextureType type)
    : path_(path), type_(type) {}

bool as::Texture::operator<(const Texture &rhs) const {
  const std::string path = GetPath();
  const std::string rhs_path = rhs.GetPath();
  if (path < rhs_path) {
    return true;
  } else if (rhs_path > path) {
    return false;
  } else {
    return GetType() < rhs.GetType();
  }
}

std::string as::Texture::GetPath() const { return path_; }

aiTextureType as::Texture::GetType() const { return type_; }
