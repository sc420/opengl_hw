#include "assignment/program.hpp"

ProgramManager::~ProgramManager() {
  // Delete all program objects
  for (const auto &pair : hdlrs_) {
    glDeleteProgram(pair.second);
  }
}

void ProgramManager::CreateProgram(const std::string &name) {
  // Create a program object
  const GLuint hdlr = glCreateProgram();
  // Save the program handler
  hdlrs_[name] = hdlr;
}

void ProgramManager::AttachShader(const std::string &name,
                                  const GLuint shader_hdlr) {
  const GLuint program_hdlr = GetProgramHdlr(name);
  glAttachShader(program_hdlr, shader_hdlr);
}

void ProgramManager::LinkProgram(const std::string &name) {
  const GLuint hdlr = GetProgramHdlr(name);
  glLinkProgram(hdlr);
  CheckProgramLinkingStatus(hdlr);
}

void ProgramManager::UseProgram(const std::string &name) {
  const GLuint hdlr = GetProgramHdlr(name);
  glUseProgram(hdlr);
}

void ProgramManager::DeleteProgram(const std::string & name)
{
  const GLuint hdlr = GetProgramHdlr(name);
  glDeleteProgram(hdlr);
}

GLuint ProgramManager::GetProgramHdlr(const std::string &name) {
  if (hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the program name '" + name + "'");
  }
  return hdlrs_.at(name);
}

void ProgramManager::CheckProgramLinkingStatus(const GLuint hdlr) {
  GLint status = -1;
  // Get linking status
  glGetProgramiv(hdlr, GL_LINK_STATUS, &status);
  // Report the log if the linking is unsuccessful
  if (status == GL_FALSE) {
    // Get the length of the log
    GLint len;
    glGetProgramiv(hdlr, GL_INFO_LOG_LENGTH, &len);
    // Get the log
    std::string log(len, '\0');
    glGetProgramInfoLog(hdlr, len, &len, &log[0]);
    // Throw an error
    throw std::runtime_error(log);
  }
}
