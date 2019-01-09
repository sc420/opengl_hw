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
      PostprocFramebufferTypes::kBlurScalingHorizontal,
      PostprocFramebufferTypes::kBlurScalingVertical,
      PostprocFramebufferTypes::kCombining};

  // Create framebuffer
  for (const PostprocFramebufferTypes postproc_framebuffer_type :
       postproc_framebuffer_types) {
    for (int scaling_idx = 0; scaling_idx < kNumBloomScaling; scaling_idx++) {
      // Get names
      const std::string framebuffer_name =
          GetPostprocFramebufferName(postproc_framebuffer_type, scaling_idx);

      // Generate framebuffer
      framebuffer_manager.GenFramebuffer(framebuffer_name);
    }
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
      PostprocFramebufferTypes::kBlurScalingHorizontal,
      PostprocFramebufferTypes::kBlurScalingVertical,
      PostprocFramebufferTypes::kCombining};

  // Set texture types to update
  const PostprocTextureTypes postproc_tex_types[] = {
      PostprocTextureTypes::kOriginal1, PostprocTextureTypes::kHdr1,
      PostprocTextureTypes::kOriginal2, PostprocTextureTypes::kHdr2};

  // Configure textures
  for (const PostprocTextureTypes postproc_tex_type : postproc_tex_types) {
    const int num_bloom_scaling =
        GetPostprocTextureTypeNumBloomScaling(postproc_tex_type);

    // Configure each scaling texture
    GLsizei cur_width = width;
    GLsizei cur_height = height;
    for (int scaling_idx = 0; scaling_idx < num_bloom_scaling; scaling_idx++) {
      // Get names
      const std::string tex_name =
          GetPostprocTextureName(postproc_tex_type, scaling_idx);
      const std::string unit_name =
          GetPostprocTextureUnitName(postproc_tex_type, scaling_idx);
      // Get indexes
      const int color_attachment_idx =
          PostprocTextureTypeToNum(postproc_tex_type) % 2;

      // Check whether to delete old texture
      if (texture_manager.HasTexture(tex_name)) {
        texture_manager.DeleteTexture(tex_name);
      }
      // Generate texture
      texture_manager.GenTexture(tex_name);
      // Update texture
      texture_manager.BindTexture(tex_name, GL_TEXTURE_2D, unit_name);
      texture_manager.InitTexture2D(tex_name, GL_TEXTURE_2D, kNumMipmapLevels,
                                    GL_BGRA, cur_width, cur_height);
      texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                         GL_TEXTURE_MIN_FILTER,
                                         GL_LINEAR_MIPMAP_LINEAR);
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
            GetPostprocFramebufferName(postproc_framebuffer_type, scaling_idx);

        framebuffer_manager.AttachTexture2DToFramebuffer(
            framebuffer_name, tex_name, GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + color_attachment_idx, GL_TEXTURE_2D, 0);

        // Tell GL we're drawing to multiple attachments
        framebuffer_manager.BindFramebuffer(framebuffer_name);
        const unsigned int attachments[] = {
            GL_COLOR_ATTACHMENT0 + 0,
            GL_COLOR_ATTACHMENT0 + 1,
        };
        glDrawBuffers(2, attachments);
      }

      // Update next texture size
      cur_width /= 2;
      cur_height /= 2;
    }
  }
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
      PostprocFramebufferTypes::kBlurScalingHorizontal,
      PostprocFramebufferTypes::kBlurScalingVertical,
      PostprocFramebufferTypes::kCombining};

  for (const PostprocFramebufferTypes postproc_framebuffer_type :
       postproc_framebuffer_types) {
    // Update each scaling renderbuffer
    GLsizei cur_width = width;
    GLsizei cur_height = height;
    for (int scaling_idx = 0; scaling_idx < kNumBloomScaling; scaling_idx++) {
      // Get names
      const std::string framebuffer_name =
          GetPostprocFramebufferName(postproc_framebuffer_type, scaling_idx);
      const std::string renderbuffer_name = GetPostprocDepthRenderbufferName(
          postproc_framebuffer_type, scaling_idx);

      // Check whether to delete old renderbuffer
      if (framebuffer_manager.HasRenderbuffer(renderbuffer_name)) {
        framebuffer_manager.DeleteRenderbuffer(renderbuffer_name);
      }
      // Create renderbuffers
      framebuffer_manager.GenRenderbuffer(renderbuffer_name);
      // Initialize renderbuffers
      framebuffer_manager.InitRenderbuffer(renderbuffer_name, GL_RENDERBUFFER,
                                           GL_DEPTH_COMPONENT, cur_width,
                                           cur_height);
      // Attach renderbuffers to framebuffers
      framebuffer_manager.AttachRenderbufferToFramebuffer(
          framebuffer_name, renderbuffer_name, GL_FRAMEBUFFER,
          GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER);

      cur_width /= 2;
      cur_height /= 2;
    }
  }
}

void shader::PostprocShader::UsePostprocFramebuffer(
    const PostprocFramebufferTypes postproc_framebuffer_type,
    const int scaling_idx) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string framebuffer_name =
      GetPostprocFramebufferName(postproc_framebuffer_type, scaling_idx);

  // Bind the framebuffer
  framebuffer_manager.BindFramebuffer(framebuffer_name, GL_FRAMEBUFFER);
}

void shader::PostprocShader::UseDefaultPostprocTextures() {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names
  const std::string framebuffer_name =
      GetPostprocFramebufferName(PostprocFramebufferTypes::kDrawOriginal, 0);
  const std::string original_tex_name =
      GetPostprocTextureName(PostprocTextureTypes::kOriginal2, 0);
  const std::string hdr_tex_name =
      GetPostprocTextureName(PostprocTextureTypes::kHdr2, 0);

  // Bind the texture
  texture_manager.BindTexture(original_tex_name);
  texture_manager.BindTexture(hdr_tex_name);

  // Attach the textures to the framebuffers
  framebuffer_manager.AttachTexture2DToFramebuffer(
      framebuffer_name, original_tex_name, GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0 + 0, GL_TEXTURE_2D, 0);
  framebuffer_manager.AttachTexture2DToFramebuffer(
      framebuffer_name, hdr_tex_name, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1,
      GL_TEXTURE_2D, 0);
}

void shader::PostprocShader::UsePostprocTexture(
    const PostprocFramebufferTypes postproc_framebuffer_type,
    const PostprocTextureTypes postproc_tex_type, const int scaling_idx) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names
  const std::string framebuffer_name =
      GetPostprocFramebufferName(postproc_framebuffer_type, scaling_idx);
  const std::string tex_name =
      GetPostprocTextureName(postproc_tex_type, scaling_idx);
  // Get indexes
  const int color_attachment_idx =
      (PostprocTextureTypeToNum(postproc_tex_type) % 2);

  // Bind the texture
  texture_manager.BindTexture(tex_name);

  // Attach the texture to the framebuffer
  framebuffer_manager.AttachTexture2DToFramebuffer(
      framebuffer_name, tex_name, GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0 + color_attachment_idx, GL_TEXTURE_2D, 0);
}

void shader::PostprocShader::DrawBloom(const glm::ivec2 &window_size) {
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  for (int pass_idx = 1; pass_idx <= 4; pass_idx++) {
    // Select framebuffer type
    PostprocFramebufferTypes framebuffer_type;
    if (pass_idx == 1) {
      framebuffer_type = PostprocFramebufferTypes::kDrawScaling;
    } else if (pass_idx == 2) {
      framebuffer_type = PostprocFramebufferTypes::kBlurScalingHorizontal;
    } else if (pass_idx == 3) {
      framebuffer_type = PostprocFramebufferTypes::kBlurScalingVertical;
    } else if (pass_idx == 4) {
      framebuffer_type = PostprocFramebufferTypes::kCombining;
    }

    // Set number of scaling
    int num_scaling;
    if (pass_idx == 1) {
      num_scaling = kNumBloomScaling;
    } else if (pass_idx == 2) {
      num_scaling = kNumBloomScaling;
    } else if (pass_idx == 3) {
      num_scaling = kNumBloomScaling;
    } else if (pass_idx == 4) {
      num_scaling = 1;
    }

    // Check whether to set all scaling texture unit indexes
    if (framebuffer_type == PostprocFramebufferTypes::kCombining) {
      SetScalingTextureUnitIdxs(pass_idx);
    }

    // Draw for each scaling
    glm::ivec2 cur_size = window_size;
    for (int scaling_idx = 0; scaling_idx < num_scaling; scaling_idx++) {
      // Use the framebuffer
      UsePostprocFramebuffer(framebuffer_type, scaling_idx);
      as::ClearColorBuffer();
      as::ClearDepthBuffer();

      // Update pass index
      UpdatePassIdx(pass_idx);
      // Update scaling index
      UpdateScalingIdx(scaling_idx);
      // Update postproc inputs buffer
      UpdatePostprocInputs();

      // Set texture unit indexes as inputs
      SetTextureUnitIdxs(pass_idx, scaling_idx);

      // Use the textures as outputs
      UsePostprocTexture(framebuffer_type,
                         GetPassOriginalTextureType(pass_idx, false), 0);
      UsePostprocTexture(framebuffer_type,
                         GetPassHdrTextureType(pass_idx, false), scaling_idx);

      // Update view port
      glViewport(0, 0, cur_size.x, cur_size.y);

      // Draw
      DrawToTextures();

      // Shrink window size
      cur_size /= 2;
    }
  }

  // Restore view port
  glViewport(0, 0, window_size.x, window_size.y);
}

void shader::PostprocShader::DrawPostprocEffects() {
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Set pass index
  const int pass_idx = 5;

  // Use the program
  UseProgram();

  // Update pass index
  UpdatePassIdx(pass_idx);
  // Update postproc inputs
  UpdatePostprocInputs();

  // Set texture unit indexes
  SetScalingTextureUnitIdxs(pass_idx);
  // Draw
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

void shader::PostprocShader::UpdateScalingIdx(const int scaling_idx) {
  postproc_inputs_.scaling_idx = scaling_idx;
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
    const PostprocFramebufferTypes postproc_framebuffer_type,
    const int scaling_idx) const {
  return GetProgramName() + "framebuffer" +
         PostprocFramebufferTypeToName(postproc_framebuffer_type) +
         "/scaling-" + std::to_string(scaling_idx);
}

std::string shader::PostprocShader::GetQuadVertexArrayGroupName() const {
  return GetProgramName() + "/vertex_array/group";
}

std::string shader::PostprocShader::GetPostprocTextureName(
    const PostprocTextureTypes postproc_tex_type, const int scaling_idx) const {
  return GetProgramName() + "/texture/postproc/" +
         PostprocTextureTypeToName(postproc_tex_type) + "/scaling-" +
         std::to_string(scaling_idx);
}

std::string shader::PostprocShader::GetPostprocTextureUnitName(
    const PostprocTextureTypes postproc_tex_type, const int scaling_idx) const {
  return GetProgramName() + "/texture_unit_name/postproc/" +
         PostprocTextureTypeToName(postproc_tex_type) + "/scaling-" +
         std::to_string(scaling_idx);
}

std::string shader::PostprocShader::GetPostprocDepthRenderbufferName(
    const PostprocFramebufferTypes postproc_framebuffer_type,
    const int scaling_idx) const {
  return GetProgramName() + "/renderbuffer/postproc/" +
         PostprocFramebufferTypeToName(postproc_framebuffer_type) +
         "/scaling-" + std::to_string(scaling_idx);
}

std::string shader::PostprocShader::GetPostprocInputsUniformBlockName() const {
  return "PostprocInputs";
}

/*******************************************************************************
 * Constants (Private)
 ******************************************************************************/

const GLsizei shader::PostprocShader::kNumMipmapLevels = 3;
const int shader::PostprocShader::kNumBloomScaling = 3;

/*******************************************************************************
 * State Getters (Private)
 ******************************************************************************/

shader::PostprocShader::PostprocTextureTypes
shader::PostprocShader::GetPassOriginalTextureType(const int pass_idx,
                                                   const bool read) const {
  if ((pass_idx % 2 == 0) == read) {
    return PostprocTextureTypes::kOriginal1;
  } else {
    return PostprocTextureTypes::kOriginal2;
  }
}

shader::PostprocShader::PostprocTextureTypes
shader::PostprocShader::GetPassHdrTextureType(const int pass_idx,
                                              const bool read) const {
  if ((pass_idx % 2 == 0) == read) {
    return PostprocTextureTypes::kHdr1;
  } else {
    return PostprocTextureTypes::kHdr2;
  }
}

/*******************************************************************************
 * State Updaters (Private)
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

void shader::PostprocShader::SetTextureUnitIdxs(const int pass_idx,
                                                const int scaling_idx) {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
  const std::string original_tex_name =
      GetPostprocTextureName(GetPassOriginalTextureType(pass_idx, true), 0);
  const std::string hdr_tex_name = GetPostprocTextureName(
      GetPassHdrTextureType(pass_idx, true), scaling_idx);
  // Get the unit indexes
  const GLuint original_unit_idx =
      texture_manager.GetUnitIdx(original_tex_name);
  const GLuint hdr_unit_idx = texture_manager.GetUnitIdx(hdr_tex_name);

  // Set the texture handlers to the unit indexes
  uniform_manager.SetUniform1Int(program_name, "original_tex",
                                 original_unit_idx);
  uniform_manager.SetUniform1Int(program_name, "hdr_tex", hdr_unit_idx);
}

void shader::PostprocShader::SetScalingTextureUnitIdxs(const int pass_idx) {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
  const std::string original_tex_name =
      GetPostprocTextureName(GetPassOriginalTextureType(pass_idx, true), 0);
  // Get unit indexes
  const GLuint original_unit_idx =
      texture_manager.GetUnitIdx(original_tex_name);

  // Set the texture handlers to the unit indexes
  uniform_manager.SetUniform1Int(program_name, "original_tex",
                                 original_unit_idx);

  for (int scaling_idx = 0; scaling_idx < kNumBloomScaling; scaling_idx++) {
    // Get names
    const std::string hdr_tex_name = GetPostprocTextureName(
        GetPassHdrTextureType(pass_idx, true), scaling_idx);
    // Get unit indexes
    const GLuint hdr_unit_idx = texture_manager.GetUnitIdx(hdr_tex_name);

    // Set the texture handlers to the unit indexes
    uniform_manager.SetUniform1Int(
        program_name, "scaled_hdr_tex" + std::to_string(scaling_idx + 1),
        hdr_unit_idx);
  }
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
    case PostprocFramebufferTypes::kBlurScalingHorizontal:
      return "blur_scaling_horizontal";
    case PostprocFramebufferTypes::kBlurScalingVertical:
      return "blur_scaling_vertical";
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
    case PostprocFramebufferTypes::kBlurScalingHorizontal:
      return 2;
    case PostprocFramebufferTypes::kBlurScalingVertical:
      return 3;
    case PostprocFramebufferTypes::kCombining:
      return 4;
    default:
      throw std::runtime_error("Unknown postproc framebuffer type");
  }
}

int shader::PostprocShader::GetPostprocTextureTypeNumBloomScaling(
    const PostprocTextureTypes postproc_tex_type) const {
  switch (postproc_tex_type) {
    case PostprocTextureTypes::kOriginal1:
    case PostprocTextureTypes::kOriginal2:
      return 1;
    case PostprocTextureTypes::kHdr1:
    case PostprocTextureTypes::kHdr2:
      return kNumBloomScaling;
    default:
      throw std::runtime_error("Unknown postproc texture type");
  }
}

std::string shader::PostprocShader::PostprocTextureTypeToName(
    const PostprocTextureTypes postproc_tex_type) const {
  switch (postproc_tex_type) {
    case PostprocTextureTypes::kOriginal1:
      return "read_original";
    case PostprocTextureTypes::kHdr1:
      return "read_hdr";
    case PostprocTextureTypes::kOriginal2:
      return "write_original";
    case PostprocTextureTypes::kHdr2:
      return "write_hdr";
    default:
      throw std::runtime_error("Unknown postproc texture type");
  }
}

int shader::PostprocShader::PostprocTextureTypeToNum(
    const PostprocTextureTypes postproc_tex_type) const {
  switch (postproc_tex_type) {
    case PostprocTextureTypes::kOriginal1:
      return 0;
    case PostprocTextureTypes::kHdr1:
      return 1;
    case PostprocTextureTypes::kOriginal2:
      return 2;
    case PostprocTextureTypes::kHdr2:
      return 3;
    default:
      throw std::runtime_error("Unknown postproc texture type");
  }
}
