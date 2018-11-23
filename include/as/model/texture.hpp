#pragma once

#include "as/common.hpp"

namespace as {
class Texture {
 public:
  Texture(const std::string &path, const std::string &type);

  bool operator<(const Texture &rhs) const;

  std::string GetPath() const;

  std::string GetType() const;

 private:
  std::string path_;

  std::string type_;
};
}  // namespace as
