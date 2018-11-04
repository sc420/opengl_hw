#include "assignment/program.hpp"

ProgramManager::ProgramManager(): shader_manager_(nullptr)
{
}

ProgramManager::~ProgramManager() {
  // Delete all program objects
  for (const auto &pair : hdlrs_) {
    glDeleteProgram(pair.second);
  }
}

void ProgramManager::RegisterShaderManager(const ShaderManager & shader_manager)
{
  shader_manager_ = &shader_manager;
}

void ProgramManager::CreateProgram(const std::string &program_name) {
  // Create a program object
  const GLuint program_hdlr = glCreateProgram();
  // Save the program handler
  hdlrs_[program_name] = program_hdlr;
}

void ProgramManager::AttachShader(const std::string &program_name,
                                  const std::string &shader_name) const {
  const GLuint program_hdlr = GetProgramHdlr(program_name);
  const GLuint shader_hdlr = shader_manager_->GetShaderHdlr(shader_name);
  glAttachShader(program_hdlr, shader_hdlr);
}

void ProgramManager::LinkProgram(const std::string &program_name) const  {
  const GLuint program_hdlr = GetProgramHdlr(program_name);
  glLinkProgram(program_hdlr);
  CheckProgramLinkingStatus(program_hdlr);
}

void ProgramManager::UseProgram(const std::string &program_name) const {
  const GLuint program_hdlr = GetProgramHdlr(program_name);
  glUseProgram(program_hdlr);
}

void ProgramManager::DeleteProgram(const std::string &program_name) const {
  const GLuint program_hdlr = GetProgramHdlr(program_name);
  glDeleteProgram(program_hdlr);
}

GLuint ProgramManager::GetProgramHdlr(const std::string &program_name) const {
  if (hdlrs_.count(program_name) == 0) {
    throw std::runtime_error("Could not find the program name '" + program_name + "'");
  }
  return hdlrs_.at(program_name);
}

void ProgramManager::CheckProgramLinkingStatus(const GLuint program_hdlr) const {
  GLint status = -1;
  // Get linking status
  glGetProgramiv(program_hdlr, GL_LINK_STATUS, &status);
  // Report the log if the linking is unsuccessful
  if (status == GL_FALSE) {
    // Get the length of the log
    GLint len;
    glGetProgramiv(program_hdlr, GL_INFO_LOG_LENGTH, &len);
    // Get the log
    std::string log(len, '\0');
    glGetProgramInfoLog(program_hdlr, len, &len, &log[0]);
    // Throw an error
    throw std::runtime_error(log);
  }
}
