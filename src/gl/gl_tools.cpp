#include "as/gl/gl_tools.hpp"

/*******************************************************************************
 * Private Method Declarations
 ******************************************************************************/

void GLAPIENTRY GLMessageCallback(const GLenum source, const GLenum type,
                                  const GLuint id, const GLenum severity,
                                  const GLsizei length, const GLchar* message,
                                  const void* userParam);

/*******************************************************************************
 * Tools
 ******************************************************************************/

/**
 * Initialize GLEW.
 *
 * Reference: http://glew.sourceforge.net/basic.html
 */
void as::InitGLEW() {
  const GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    throw std::runtime_error("Could not initialize GLEW");
  }
}

/**
 * Print OpenGL context information.
 *
 * Reference: https://www.khronos.org/opengl/wiki/OpenGL_Context
 */
void as::PrintGLContextInfo(const bool print_extensions,
                            const bool print_supported_glsl_versions) {
  std::cerr << "GL version: " << glGetString(GL_VERSION) << std::endl;
  std::cerr << "GL vendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cerr << "GL renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cerr << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;
  if (print_extensions) {
    GLint num_ext;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
    std::cerr << "Number of GL extensions: " << num_ext << std::endl;
    std::cerr << "Extensions:" << std::endl;
    for (GLint i = 0; i < num_ext; i++) {
      std::cerr << glGetStringi(GL_EXTENSIONS, i) << std::endl;
    }
    std::cerr << std::endl;
  }
  if (print_supported_glsl_versions) {
    GLint num_ver;
    glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &num_ver);
    std::cerr << "Number of supported GLSL versions: " << num_ver + 1
              << std::endl;
    std::cerr << "Supported GLSL versions:" << std::endl;
    for (GLint i = 0; i < num_ver; i++) {
      std::cerr << glGetStringi(GL_SHADING_LANGUAGE_VERSION, i) << std::endl;
    }
    // Also print the current version
    std::cerr << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cerr << std::endl;
  }
}

/**
 * Enable OpenGL to catch errors and report.
 *
 * Reference: https://www.khronos.org/opengl/wiki/Debug_Output
 */
void as::EnableCatchingGLError(const bool stop_when_error) {
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(GLMessageCallback, &stop_when_error);
}

/*******************************************************************************
 * Window Position and Size
 ******************************************************************************/

void as::SetGLWindowInitCenterPos(const glm::ivec2& window_pos,
                                  const glm::ivec2& window_size) {
  glutInitWindowPosition(window_pos.x - window_size.x / 2,
                         window_pos.y - window_size.y / 2);
}

void as::SetGLWindowInitRelativeCenterPos(const glm::vec2& relative_window_pos,
                                          const glm::ivec2& window_size) {
  const int screen_width = glutGet(GLUT_SCREEN_WIDTH);
  const int screen_height = glutGet(GLUT_SCREEN_HEIGHT);
  if (screen_width <= 0 || screen_height <= 0) {
    return;
  }
  const glm::vec2 window_pos = glm::vec2(relative_window_pos.x * screen_width,
                                         relative_window_pos.y * screen_height);
  SetGLWindowInitCenterPos(window_pos, window_size);
}

void as::SetGLWindowInitSize(const glm::ivec2& window_size) {
  glutInitWindowSize(window_size.x, window_size.y);
}

bool as::LimitGLWindowSize(const int width, const int height,
                           const glm::ivec2& min_window_size,
                           const glm::ivec2& max_window_size) {
  const int new_width =
      std::min(std::max(width, min_window_size.x), max_window_size.x);
  const int new_height =
      std::min(std::max(height, min_window_size.y), max_window_size.y);
  glutReshapeWindow(new_width, new_height);
  return new_width != width || new_height != height;
}

/*******************************************************************************
 * GL Manager Container
 ******************************************************************************/

void as::GLManagers::RegisterManagers(BufferManager& buffer_manager,
                                      FramebufferManager& framebuffer_manager,
                                      ProgramManager& program_manager,
                                      ShaderManager& shader_manager,
                                      TextureManager& texture_manager,
                                      UniformManager& uniform_manager,
                                      VertexSpecManager& vertex_spec_manager) {
  buffer_manager_ = &buffer_manager;
  framebuffer_manager_ = &framebuffer_manager;
  program_manager_ = &program_manager;
  shader_manager_ = &shader_manager;
  texture_manager_ = &texture_manager;
  uniform_manager_ = &uniform_manager;
  vertex_spec_manager_ = &vertex_spec_manager;
}

void as::GLManagers::Init() {
  // Initialize managers
  texture_manager_->Init();
  // Register managers
  framebuffer_manager_->RegisterTextureManager(*texture_manager_);
  program_manager_->RegisterShaderManager(*shader_manager_);
  uniform_manager_->RegisterProgramManager(*program_manager_);
  uniform_manager_->RegisterBufferManager(*buffer_manager_);
  vertex_spec_manager_->RegisterBufferManager(*buffer_manager_);
}

as::BufferManager& as::GLManagers::GetBufferManager() const {
  return *buffer_manager_;
}

as::FramebufferManager& as::GLManagers::GetFramebufferManager() const {
  return *framebuffer_manager_;
}

as::ProgramManager& as::GLManagers::GetProgramManager() const {
  return *program_manager_;
}

as::ShaderManager& as::GLManagers::GetShaderManager() const {
  return *shader_manager_;
}

as::TextureManager& as::GLManagers::GetTextureManager() const {
  return *texture_manager_;
}

as::UniformManager& as::GLManagers::GetUniformManager() const {
  return *uniform_manager_;
}

as::VertexSpecManager& as::GLManagers::GetVertexSpecManager() const {
  return *vertex_spec_manager_;
}

/*******************************************************************************
 * Callbacks
 ******************************************************************************/

void GLAPIENTRY GLMessageCallback(const GLenum source, const GLenum type,
                                  const GLuint id, const GLenum severity,
                                  const GLsizei length, const GLchar* message,
                                  const void* user_param) {
  /* Filter based on types */
  // Ignore API_ID_RECOMPILE_FRAGMENT_SHADER
  if (type == 0x8250) {
    return;
  }
  /* Print the message */
  std::cerr << "GL Callback: "
            << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
            << " type = 0x" << std::hex << type << ", severity = 0x" << std::hex
            << severity << ", message = " << message << std::endl;
  /* Check whether to stop */
  const bool stop_when_error = (*static_cast<const bool*>(user_param));
  if (stop_when_error) {
    throw std::runtime_error("GL error has occurred, stopping now");
  }
}
