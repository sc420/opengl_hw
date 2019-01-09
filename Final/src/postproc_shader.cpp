#include "postproc_shader.hpp"

shader::PostprocShader::PostprocShader() : postproc_inputs_(PostprocInputs()) {
  // TODO: debug
  postproc_inputs_.effect_idx = kEffectBloomEffect;
}

/*******************************************************************************
 * GL Initializations
 ******************************************************************************/

void shader::PostprocShader::Init() {
  CreateShaders();
  CreatePrograms();
  LoadModel();
  InitFramebuffers();
  InitVertexArrays();
  InitUniformBlocks();
}

void shader::PostprocShader::LoadModel() {
  quad_model_.LoadFile("assets/models/quad/quad.obj", 0);
}

void shader::PostprocShader::InitFramebuffers() {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();

  // Set framebuffer types to initialize
  const PostprocFramebufferTypes postproc_framebuffer_types[] = {
      PostprocFramebufferTypes::kDrawOriginal,
      PostprocFramebufferTypes::kDrawScaling,
      PostprocFramebufferTypes::kBlurScaling,
      PostprocFramebufferTypes::kCombining};

  // Create framebuffer
  for (const PostprocFramebufferTypes postproc_framebuffer_type :
       postproc_framebuffer_types) {
    // Get names
    const std::string framebuffer_name =
        GetPostprocFramebufferName(postproc_framebuffer_type);

    // Generate framebuffer
    framebuffer_manager.GenFramebuffer(framebuffer_name);
  }
}

void shader::PostprocShader::InitVertexArrays() {
  InitVertexArray(GetQuadVertexArrayGroupName(), quad_model_);
}

void shader::PostprocShader::InitUniformBlocks() {
  LinkDataToUniformBlock(GetPostprocInputsBufferName(),
                         GetPostprocInputsUniformBlockName(), postproc_inputs_);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::PostprocShader::UpdatePostprocTextures(const GLsizei width,
                                                    const GLsizei height) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();

  // Set framebuffer types to attach
  const PostprocFramebufferTypes postproc_framebuffer_types[] = {
      PostprocFramebufferTypes::kDrawOriginal,
      PostprocFramebufferTypes::kDrawScaling,
      PostprocFramebufferTypes::kBlurScaling,
      PostprocFramebufferTypes::kCombining};

  // Set texture types to update
  const PostprocTextureTypes postproc_tex_types[] = {
      PostprocTextureTypes::kOriginal, PostprocTextureTypes::kHdr};

  // Configure textures
  for (const PostprocTextureTypes postproc_tex_type : postproc_tex_types) {
    // Get names
    const std::string tex_name = GetPostprocTextureName(postproc_tex_type);
    const std::string unit_name = GetPostprocTextureUnitName(postproc_tex_type);
    // Get indexes
    const int color_attachment_idx =
        PostprocTextureTypeToNum(postproc_tex_type);

    // Check whether to delete old texture
    if (texture_manager.HasTexture(tex_name)) {
      texture_manager.DeleteTexture(tex_name);
    }
    // Generate texture
    texture_manager.GenTexture(tex_name);
    // Update texture
    texture_manager.BindTexture(tex_name, GL_TEXTURE_2D, unit_name);
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
    for (const PostprocFramebufferTypes postproc_framebuffer_type :
         postproc_framebuffer_types) {
      // Get names
      const std::string framebuffer_name =
          GetPostprocFramebufferName(postproc_framebuffer_type);

      framebuffer_manager.AttachTexture2DToFramebuffer(
          framebuffer_name, tex_name, GL_FRAMEBUFFER,
          GL_COLOR_ATTACHMENT0 + color_attachment_idx, GL_TEXTURE_2D, 0);
    }
  }

  // Tell GL we're drawing to multiple attachments
  unsigned int attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0 + 1};
  glDrawBuffers(2, attachments);

  // Set texture unit indexes
  SetTextureUnitIdxs();
}

void shader::PostprocShader::UpdatePostprocRenderbuffer(const GLsizei width,
                                                        const GLsizei height) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();

  // Set framebuffer types to attach
  const PostprocFramebufferTypes postproc_framebuffer_types[] = {
      PostprocFramebufferTypes::kDrawOriginal,
      PostprocFramebufferTypes::kDrawScaling,
      PostprocFramebufferTypes::kBlurScaling,
      PostprocFramebufferTypes::kCombining};

  for (const PostprocFramebufferTypes postproc_framebuffer_type :
       postproc_framebuffer_types) {
    // Get names
    const std::string framebuffer_name =
        GetPostprocFramebufferName(postproc_framebuffer_type);
    const std::string renderbuffer_name =
        GetPostprocDepthRenderbufferName(postproc_framebuffer_type);

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

void shader::PostprocShader::UsePostprocFramebuffer(
    const PostprocFramebufferTypes postproc_framebuffer_type) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string framebuffer_name =
      GetPostprocFramebufferName(postproc_framebuffer_type);

  // Bind the framebuffer
  framebuffer_manager.BindFramebuffer(framebuffer_name, GL_FRAMEBUFFER);
}

void shader::PostprocShader::DrawBloom() {
  //// Use the program
  // UseProgram();

  //// Update postproc inputs
  // UpdatePassIdx(0);
  // UpdatePostprocInputs();

  // DrawToTextures();
}

void shader::PostprocShader::DrawPostprocEffects() {
  // Use the program
  UseProgram();

  // Update postproc inputs
  UpdatePassIdx(1);
  UpdatePostprocInputs();

  DrawToTextures();
}

/*******************************************************************************
 * State Updaters
 ******************************************************************************/

void shader::PostprocShader::UpdateEnabled(const bool enabled) {
  postproc_inputs_.enabled = enabled;
}

void shader::PostprocShader::UpdateMousePos(const glm::ivec2 &mouse_pos) {
  postproc_inputs_.mouse_pos = mouse_pos;
}

void shader::PostprocShader::UpdateEffectIdx(const int effect_idx) {
  postproc_inputs_.effect_idx = effect_idx;
}

void shader::PostprocShader::UpdatePassIdx(const int pass_idx) {
  postproc_inputs_.pass_idx = pass_idx;
}

void shader::PostprocShader::UpdateTime(const float time) {
  postproc_inputs_.time = time;
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::PostprocShader::GetId() const { return "postproc"; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::PostprocShader::GetPostprocInputsBufferName() const {
  return GetProgramName() + "/buffer/postproc_inputs";
}

std::string shader::PostprocShader::GetPostprocFramebufferName(
    const PostprocFramebufferTypes postproc_framebuffer_type) const {
  return GetProgramName() + "framebuffer" +
         PostprocFramebufferTypeToName(postproc_framebuffer_type);
}

std::string shader::PostprocShader::GetQuadVertexArrayGroupName() const {
  return GetProgramName() + "/vertex_array/group";
}

std::string shader::PostprocShader::GetPostprocTextureName(
    const PostprocTextureTypes postproc_tex_type) const {
  return GetProgramName() + "/texture/postproc/" +
         PostprocTextureTypeToName(postproc_tex_type);
}

std::string shader::PostprocShader::GetPostprocTextureUnitName(
    const PostprocTextureTypes postproc_tex_type) const {
  return GetProgramName() + "/texture_unit_name/postproc/" +
         PostprocTextureTypeToName(postproc_tex_type);
}

std::string shader::PostprocShader::GetPostprocDepthRenderbufferName(
    const PostprocFramebufferTypes postproc_framebuffer_type) const {
  return GetProgramName() + "/renderbuffer/postproc/" +
         PostprocFramebufferTypeToName(postproc_framebuffer_type);
}

std::string shader::PostprocShader::GetPostprocInputsUniformBlockName() const {
  return "PostprocInputs";
}

/*******************************************************************************
 * Type Conversions (Private)
 ******************************************************************************/

std::string shader::PostprocShader::PostprocFramebufferTypeToName(
    const PostprocFramebufferTypes postproc_framebuffer_type) const {
  switch (postproc_framebuffer_type) {
    case PostprocFramebufferTypes::kDrawOriginal:
      return "draw_original";
    case PostprocFramebufferTypes::kDrawScaling:
      return "draw_scaling";
    case PostprocFramebufferTypes::kBlurScaling:
      return "blur_scaling";
    case PostprocFramebufferTypes::kCombining:
      return "combining";
    default:
      throw std::runtime_error("Unknown postproc framebuffer type");
  }
}

int shader::PostprocShader::PostprocFramebufferTypeToNum(
    const PostprocFramebufferTypes postproc_framebuffer_type) const {
  switch (postproc_framebuffer_type) {
    case PostprocFramebufferTypes::kDrawOriginal:
      return 0;
    case PostprocFramebufferTypes::kDrawScaling:
      return 1;
    case PostprocFramebufferTypes::kBlurScaling:
      return 2;
    case PostprocFramebufferTypes::kCombining:
      return 3;
    default:
      throw std::runtime_error("Unknown postproc framebuffer type");
  }
}

std::string shader::PostprocShader::PostprocTextureTypeToName(
    const PostprocTextureTypes postproc_tex_type) const {
  switch (postproc_tex_type) {
    case PostprocTextureTypes::kOriginal:
      return "original";
    case PostprocTextureTypes::kHdr:
      return "hdr";
    default:
      throw std::runtime_error("Unknown postproc texture type");
  }
}

int shader::PostprocShader::PostprocTextureTypeToNum(
    const PostprocTextureTypes postproc_tex_type) const {
  switch (postproc_tex_type) {
    case PostprocTextureTypes::kOriginal:
      return 0;
    case PostprocTextureTypes::kHdr:
      return 1;
    default:
      throw std::runtime_error("Unknown postproc texture type");
  }
}

/*******************************************************************************
 * GL Drawing Methods (Private)
 ******************************************************************************/

void shader::PostprocShader::UpdatePostprocInputs() {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetPostprocInputsBufferName();

  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

/*******************************************************************************
 * GL Drawing Methods (Private)
 ******************************************************************************/

void shader::PostprocShader::SetTextureUnitIdxs() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
  const std::string original_tex_name =
      GetPostprocTextureName(PostprocTextureTypes::kOriginal);
  const std::string hdr_tex_name =
      GetPostprocTextureName(PostprocTextureTypes::kHdr);
  // Get the unit indexes
  const GLuint original_unit_idx =
      texture_manager.GetUnitIdx(original_tex_name);
  const GLuint hdr_unit_idx = texture_manager.GetUnitIdx(hdr_tex_name);
  // Set the texture handlers to the unit indexes
  uniform_manager.SetUniform1Int(program_name, "original_tex",
                                 original_unit_idx);
  uniform_manager.SetUniform1Int(program_name, "hdr_tex", hdr_unit_idx);
}

void shader::PostprocShader::DrawToTextures() {
  // Get names
  const std::string group_name = GetQuadVertexArrayGroupName();
  // Get the mesh
  const std::vector<as::Mesh> &meshes = quad_model_.GetMeshes();
  const as::Mesh &mesh = meshes.front();
  // Get the array indexes
  const std::vector<size_t> &idxs = mesh.GetIdxs();

  // Use the first mesh
  UseMesh(group_name, 0);
  // Draw the mesh
  glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, nullptr);
}
