#include "diff_shader.hpp"

shader::DiffShader::DiffShader() { diff_.display_mode = 0; }

/*******************************************************************************
 * GL Initializations
 ******************************************************************************/

void shader::DiffShader::Init() {
  // TODO: Other shaders should explicitly initialize programs and shaders
  Shader::Init();
  LoadModel();
  InitFramebuffers();
  InitUniformBlocks();
  InitVertexArrays();
}

void shader::DiffShader::LoadModel() {
  quad_model_.LoadFile("assets/models/quad/quad.obj", 0);
}

void shader::DiffShader::InitFramebuffers() {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Create framebuffers
  const DiffTypes diff_types[] = {DiffTypes::kObj, DiffTypes::kNoObj,
                                  DiffTypes::kBg};
  for (const DiffTypes diff_type : diff_types) {
    const std::string framebuffer_name = GetDiffFramebufferName(diff_type);
    framebuffer_manager.GenFramebuffer(framebuffer_name);
  }
}

void shader::DiffShader::InitUniformBlocks() {
  LinkDataToUniformBlock(GetDiffBufferName(), GetDiffUniformBlockName(), diff_);
}

void shader::DiffShader::InitVertexArrays() {
  // TODO: Also get name like this for other shaders
  const std::string group_name = GetQuadVertexArrayGroupName();
  InitVertexArray(group_name, quad_model_);
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::DiffShader::Draw() {
  // Get names
  const std::string group_name = GetQuadVertexArrayGroupName();

  // Use the program
  UseProgram();

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

void shader::DiffShader::UseDiffFramebuffer(const DiffTypes diff_type) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string framebuffer_name = GetDiffFramebufferName(diff_type);
  // Bind framebuffer
  framebuffer_manager.BindFramebuffer(framebuffer_name, GL_FRAMEBUFFER);
}

void shader::DiffShader::UpdateObjDiffRenderbuffer(const GLsizei width,
                                                   const GLsizei height) {
  const DiffTypes diff_types[] = {DiffTypes::kObj, DiffTypes::kNoObj,
                                  DiffTypes::kBg};
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  // Get names
  const std::string obj_framebuffer_name =
      GetDiffFramebufferName(DiffTypes::kObj);
  const std::string obj_renderbuffer_name = GetObjDiffDepthRenderbufferName();
  // Check whether to delete old renderbuffer
  if (framebuffer_manager.HasRenderbuffer(obj_renderbuffer_name)) {
    framebuffer_manager.DeleteRenderbuffer(obj_renderbuffer_name);
  }
  // Create renderbuffers
  framebuffer_manager.GenRenderbuffer(obj_renderbuffer_name);
  // Initialize renderbuffers
  framebuffer_manager.InitRenderbuffer(obj_renderbuffer_name, GL_RENDERBUFFER,
                                       GL_DEPTH24_STENCIL8, width, height);
  // Attach the renderbuffer to all framebuffers
  for (const DiffTypes diff_type : diff_types) {
    const std::string framebuffer_name = GetDiffFramebufferName(diff_type);
    framebuffer_manager.AttachRenderbufferToFramebuffer(
        framebuffer_name, obj_renderbuffer_name, GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER);
  }
}

void shader::DiffShader::UpdateDiffFramebufferTextures(const GLsizei width,
                                                       const GLsizei height) {
  // Get managers
  as::FramebufferManager &framebuffer_manager =
      gl_managers_->GetFramebufferManager();
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Update all differential rendering textures
  const DiffTypes diff_types[] = {DiffTypes::kObj, DiffTypes::kNoObj,
                                  DiffTypes::kBg};
  for (const DiffTypes diff_type : diff_types) {
    // Get names
    const std::string framebuffer_name = GetDiffFramebufferName(diff_type);
    const std::string tex_name = GetDiffFramebufferTextureName(diff_type);
    const std::string tex_unit_name =
        GetDiffFramebufferTextureUnitName(diff_type);
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

  // Set texture unit indexes
  SetTextureUnitIdxs();
}

/*******************************************************************************
 * State Updaters
 ******************************************************************************/

void shader::DiffShader::ToggleDisplayMode() {
  // Get managers
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string buffer_name = GetDiffBufferName();
  // Switch display mode
  diff_.display_mode = (diff_.display_mode + 1) % 4;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::DiffShader::GetId() const { return "diff"; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::DiffShader::GetDiffFramebufferName(
    const DiffTypes diff_type) const {
  return GetProgramName() + "/framebuffer/" + DiffTypeToName(diff_type);
}

std::string shader::DiffShader::GetObjDiffDepthRenderbufferName() const {
  return GetProgramName() + "/renderbuffer/obj";
}

std::string shader::DiffShader::GetDiffFramebufferTextureName(
    const DiffTypes diff_type) const {
  return GetProgramName() + "/texture/" + DiffTypeToName(diff_type);
}

std::string shader::DiffShader::GetDiffFramebufferTextureUnitName(
    const DiffTypes diff_type) const {
  return GetProgramName() + "/texture_unit_name/" + DiffTypeToName(diff_type);
}

std::string shader::DiffShader::GetQuadVertexArrayGroupName() const {
  return GetProgramName() + "/vertex_array/group";
}

std::string shader::DiffShader::GetDiffBufferName() const {
  return GetProgramName() + "/buffer/diff";
}

std::string shader::DiffShader::GetDiffUniformBlockName() const {
  return "Diff";
}

/*******************************************************************************
 * GL Drawing Methods (Private)
 ******************************************************************************/

void shader::DiffShader::SetTextureUnitIdxs() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();

  // Use the program
  UseProgram();
  // Set all texture unit indexes
  const DiffTypes diff_types[] = {DiffTypes::kObj, DiffTypes::kNoObj,
                                  DiffTypes::kBg};
  const std::string var_names[] = {"obj_tex", "no_obj_tex", "bg_tex"};
  for (int i = 0; i < 3; i++) {
    const DiffTypes diff_type = diff_types[i];
    const std::string var_name = var_names[i];
    // Get names
    const std::string framebuffer_name = GetDiffFramebufferName(diff_type);
    const std::string tex_name = GetDiffFramebufferTextureName(diff_type);
    // Get the unit index
    const GLuint unit_idx = texture_manager.GetUnitIdx(tex_name);
    // Set the texture handlers to the unit indexes
    uniform_manager.SetUniform1Int(program_name, var_name, unit_idx);
  }
}

/*******************************************************************************
 * Name Management (Private)
 ******************************************************************************/

std::string shader::DiffShader::DiffTypeToName(
    const DiffTypes diff_type) const {
  switch (diff_type) {
    case DiffTypes::kObj: {
      return "obj";
    } break;
    case DiffTypes::kNoObj: {
      return "no_obj";
    } break;
    case DiffTypes::kBg: {
      return "bg";
    } break;
    default: { throw std::runtime_error("Unknown diff type"); }
  }
}
