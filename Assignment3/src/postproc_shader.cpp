#include "postproc_shader.hpp"

/*******************************************************************************
 * GL Initialization Methods
 ******************************************************************************/

void shader::PostprocShader::Init() {
  Shader::Init();
  LoadModel();
  InitFramebuffers();
  InitVertexArrays();
  InitUniformBlocks();
}

void shader::PostprocShader::LoadModel() {
  as::Model &model = GetModel();
  model.LoadFile("assets/models/quad/quad.obj", 0);
}

void shader::PostprocShader::InitFramebuffers() {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Create framebuffers
  // 1st framebuffer is the original screen
  // 2nd and 3rd framebuffers are ping pong screen for multi-pass filtering
  for (int screen_idx = 0; screen_idx < kNumFramebuffers; screen_idx++) {
    const std::string &name = GetScreenFramebufferName(screen_idx);
    framebuffer_manager.GenFramebuffer(name);
  }
}

void shader::PostprocShader::InitVertexArrays() {
  const as::Model &model = GetModel();
  InitVertexArray(model);
}

void shader::PostprocShader::InitUniformBlocks() {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();
  const std::string &buffer_name = GetPostprocInputsBufferName();
  const std::string &uniform_block_name = GetPostprocInputsUniformBlockName();
  const std::string &binding_name = buffer_name;
  // Initialize the buffer
  InitUniformBuffer(buffer_name, postproc_inputs_);
  // Bind the uniform block to the the buffer
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, uniform_block_name, binding_name);
  // Bind the buffer to the uniform block
  uniform_manager.BindBufferBaseToBindingPoint(buffer_name, binding_name);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::PostprocShader::UpdateScreenTextures(const GLsizei width,
                                                  const GLsizei height) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Configure textures
  for (int screen_idx = 0; screen_idx < kNumFramebuffers; screen_idx++) {
    // Get names
    const std::string framebuffer_name = GetScreenFramebufferName(screen_idx);
    const std::string tex_name = GetScreenTextureName(screen_idx);
    const std::string &tex_unit_name = tex_name;
    // Check whether to delete old texture
    if (texture_manager.HasTexture(tex_name)) {
      texture_manager.DeleteTexture(tex_name);
    }
    // Generate texture
    texture_manager.GenTexture(tex_name);
    // Update texture
    texture_manager.BindTexture(tex_name, GL_TEXTURE_2D, tex_unit_name);
    texture_manager.InitTexture2D(tex_name, GL_TEXTURE_2D, 1, GL_RGB8, width,
                                  height);
    texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                       GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                       GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                       GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                       GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Attach textures to framebuffers
    framebuffer_manager.AttachTexture2DToFramebuffer(
        framebuffer_name, tex_name, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, 0);
  }
}

void shader::PostprocShader::UpdateScreenRenderbuffers(const GLsizei width,
                                                       const GLsizei height) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Configure textures
  for (int screen_idx = 0; screen_idx < kNumFramebuffers; screen_idx++) {
    // Get names
    const std::string framebuffer_name = GetScreenFramebufferName(screen_idx);
    const std::string renderbuffer_name =
        GetScreenDepthRenderbufferName(screen_idx);
    // Check whether to delete old renderbuffer
    if (framebuffer_manager.HasRenderbuffer(renderbuffer_name)) {
      framebuffer_manager.DeleteRenderbuffer(renderbuffer_name);
    }
    // Create renderbuffers
    framebuffer_manager.GenRenderbuffer(renderbuffer_name);
    // Initialize renderbuffers
    framebuffer_manager.InitRenderbuffer(renderbuffer_name, GL_RENDERBUFFER,
                                         GL_DEPTH_COMPONENT, width, height);
    // Attach renderbuffers to framebuffers
    framebuffer_manager.AttachRenderbufferToFramebuffer(
        framebuffer_name, renderbuffer_name, GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER);
  }
}

void shader::PostprocShader::Draw() {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string &buffer_name = GetPostprocInputsBufferName();

  // Use the program
  UseProgram();
  // Set texture unit indexes
  SetTextureUnitIdxs();
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
  // Check whether to enable multi-pass filtering
  if (postproc_inputs_.effect_idx[0] == Effects::kEffectBloomEffect) {
    /* Draw to framebuffer 1 with texture 0 */
    // Update the current pass index
    UpdatePassIdx(0);
    buffer_manager.UpdateBuffer(buffer_name);
    // Draw to screen framebuffer
    UseScreenFramebuffer(1);
    DrawScreenWithTexture(0);

    /* Draw to framebuffer 2(1) with texture 1(2) */
    for (int i = 1; i < 1 + 2 * kNumMultipass; i++) {
      // Update the current pass index
      UpdatePassIdx(i);
      buffer_manager.UpdateBuffer(buffer_name);
      // Draw to screen framebuffer
      const int source_idx = 1 + (i % 2);
      const int target_idx = 1 + ((i + 1) % 2);
      UseScreenFramebuffer(target_idx);
      DrawScreenWithTexture(source_idx);
    }

    /* Draw to framebuffer 0 with texture 1 */
    // Update the current pass index
    UpdatePassIdx(1 + 2 * kNumMultipass);
    buffer_manager.UpdateBuffer(buffer_name);
    // Draw to default framebuffer
    UseDefaultFramebuffer();
    DrawScreenWithTexture(1);
  } else {
    DrawScreenWithTexture(0);
  }
}

void shader::PostprocShader::UseDefaultFramebuffer() {
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  framebuffer_manager.BindDefaultFramebuffer(GL_FRAMEBUFFER);
}

void shader::PostprocShader::UseScreenFramebuffer(const int screen_idx) {
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  const std::string &framebuffer_name = GetScreenFramebufferName(screen_idx);
  framebuffer_manager.BindFramebuffer(framebuffer_name);
}

/*******************************************************************************
 * State Updating Methods
 ******************************************************************************/

void shader::PostprocShader::UpdateEnabled(const bool enabled) {
  postproc_inputs_.enabled[0] = enabled;
}

void shader::PostprocShader::UpdateMousePos(const glm::ivec2 &mouse_pos) {
  postproc_inputs_.mouse_pos = mouse_pos;
}

void shader::PostprocShader::UpdateWindowSize(const glm::ivec2 &window_size) {
  postproc_inputs_.window_size = window_size;
}

void shader::PostprocShader::UpdateEffectIdx(const int effect_idx) {
  postproc_inputs_.effect_idx[0] = effect_idx;
}

void shader::PostprocShader::UpdatePassIdx(const int pass_idx) {
  postproc_inputs_.pass_idx[0] = pass_idx;
}

void shader::PostprocShader::UpdateTime(const int time) {
  postproc_inputs_.time[0] = time;
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::PostprocShader::GetId() const { return "postproc"; }

/*******************************************************************************
 * Model Handlers (Protected)
 ******************************************************************************/

as::Model &shader::PostprocShader::GetModel() { return screen_quad_model_; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::PostprocShader::GetPostprocInputsBufferName() const {
  return "postproc_inputs";
}

std::string shader::PostprocShader::GetPostprocInputsUniformBlockName() const {
  return "PostprocInputs";
}

std::string shader::PostprocShader::GetScreenFramebufferName(
    const int screen_idx) const {
  return "screen[" + std::to_string(screen_idx) + "]";
}

std::string shader::PostprocShader::GetScreenTextureName(
    const int screen_idx) const {
  return "screen[" + std::to_string(screen_idx) + "]";
}

std::string shader::PostprocShader::GetScreenDepthRenderbufferName(
    const int screen_idx) const {
  return "screen[" + std::to_string(screen_idx) + "]";
}

/*******************************************************************************
 * Constants (Private)
 ******************************************************************************/

const int shader::PostprocShader::kNumFramebuffers = 3;
const int shader::PostprocShader::kNumMultipass = 10;

/*******************************************************************************
 * GL Drawing Methods (Private)
 ******************************************************************************/

void shader::PostprocShader::SetTextureUnitIdxs() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();
  const std::string &screen_tex1_name = GetScreenTextureName(0);
  const std::string &screen_tex2_name = GetScreenTextureName(1);
  const std::string &screen_tex3_name = GetScreenTextureName(2);
  // Get the unit indexes
  const GLuint unit_idx1 = texture_manager.GetUnitIdx(screen_tex1_name);
  const GLuint unit_idx2 = texture_manager.GetUnitIdx(screen_tex2_name);
  const GLuint unit_idx3 = texture_manager.GetUnitIdx(screen_tex3_name);
  // Set the texture handlers to the unit indexes
  uniform_manager.SetUniform1Int(program_name, "screen_tex", unit_idx1);
  uniform_manager.SetUniform1Int(program_name, "multipass_tex1", unit_idx2);
  uniform_manager.SetUniform1Int(program_name, "multipass_tex2", unit_idx3);
}

void shader::PostprocShader::DrawScreenWithTexture(const int tex_idx) {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &tex_name = GetScreenTextureName(tex_idx);
  // Get models
  const as::Model &model = GetModel();
  // Get the mesh
  const std::vector<as::Mesh> &meshes = model.GetMeshes();
  const as::Mesh &mesh = meshes.front();
  // Get the array indexes
  const std::vector<size_t> &idxs = mesh.GetIdxs();
  // Use the first mesh
  UseMesh(0);
  // Bind the texture
  texture_manager.BindTexture(tex_name);
  // Draw the mesh
  glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, 0);
}
