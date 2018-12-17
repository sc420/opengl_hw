#pragma once

#include "as/common.hpp"

namespace as {
class Texture {
 public:
  Texture(const std::string &path, const aiTextureType type);

  bool operator<(const Texture &rhs) const;

  std::string GetPath() const;

  aiTextureType GetType() const;

 private:
  std::string path_;

  aiTextureType type_;
};
}  // namespace as
