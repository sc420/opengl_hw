#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class VertexSpecManager {
public:
  ~VertexSpecManager();

  void GenVertexArray(const std::string &name);

  void BindBuffer(const std::string &name);

  void DeleteVertexArray(const std::string &name);

  GLuint GetVertexArrayHdlr(const std::string &name);

private:
  std::map<std::string, GLuint> hdlrs_;
};
