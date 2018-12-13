#include "scene_shader.hpp"

/*******************************************************************************
 * GL Initialization Methods
 ******************************************************************************/

void shader::SceneShader::Init() {
  Shader::Init();
  LoadModel();
  InitVertexArrays();
  InitUniformBlocks();
  InitTextures();
}

void shader::SceneShader::LoadModel() {
  scene_model_.LoadFile(
      "assets/models/crytek-sponza/sponza.obj",
      aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate);
}

void shader::SceneShader::InitVertexArrays() { InitVertexArray(scene_model_); }

void shader::SceneShader::InitUniformBlocks() {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();
  const std::string &global_mvp_buffer_name = GetGlobalMvpBufferName();
  const std::string &model_trans_buffer_name = GetModelTransBufferName();
  const std::string &global_mvp_uniform_block_name =
      GetGlobalMvpUniformBlockName();
  const std::string &model_trans_uniform_block_name =
      GetModelTransUniformBlockName();
  const std::string &global_mvp_binding_name = global_mvp_buffer_name;
  const std::string &model_trans_binding_name = model_trans_buffer_name;
  // Initialize the buffers
  InitUniformBuffer(global_mvp_buffer_name, global_mvp_);
  InitUniformBuffer(model_trans_buffer_name, model_trans_);
  // Bind the uniform blocks to the the buffer
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, global_mvp_uniform_block_name, global_mvp_binding_name);
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, model_trans_uniform_block_name, model_trans_binding_name);
  // Bind the buffers to the uniform blocks
  uniform_manager.BindBufferBaseToBindingPoint(global_mvp_buffer_name,
                                               global_mvp_binding_name);
  uniform_manager.BindBufferBaseToBindingPoint(model_trans_buffer_name,
                                               model_trans_binding_name);
}

void shader::SceneShader::InitTextures() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Initialize textures in each mesh
  const std::vector<as::Mesh> &meshes = scene_model_.GetMeshes();
  for (const as::Mesh &mesh : meshes) {
    const std::set<as::Texture> &textures = mesh.GetTextures();
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      // Check if the texture has been loaded
      if (texture_manager.HasTexture(path)) {
        continue;
      }
      // Decide the unit name
      const std::string unit_name = path;
      // Load the texture
      GLsizei width, height;
      int comp;
      std::vector<GLubyte> texels;
      as::LoadTextureByStb(path, 0, width, height, comp, texels);
      // Convert the texels from 3 channels to 4 channels to avoid GL errors
      texels = as::ConvertDataChannels3To4(texels);
      // Generate the texture
      texture_manager.GenTexture(path);
      // Bind the texture
      texture_manager.BindTexture(path, GL_TEXTURE_2D, unit_name);
      // Initialize the texture
      texture_manager.InitTexture2D(path, GL_TEXTURE_2D, kNumMipmapLevels,
                                    GL_RGBA8, width, height);
      // Update the texture
      texture_manager.UpdateTexture2D(path, GL_TEXTURE_2D, 0, 0, 0, width,
                                      height, GL_RGBA, GL_UNSIGNED_BYTE,
                                      texels.data());
      texture_manager.GenMipmap(path, GL_TEXTURE_2D);
      texture_manager.SetTextureParamInt(
          path, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D,
                                         GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
  }
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::SceneShader::Draw() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();

  // Use the program
  UseProgram();

  // Draw each mesh with its own texture
  const std::vector<as::Mesh> &meshes = scene_model_.GetMeshes();
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Get names
    const std::string scene_va_name = GetMeshVertexArrayName(mesh_idx);
    const std::string scene_buffer_name =
        GetMeshVertexArrayBufferName(mesh_idx);
    const std::string scene_idxs_buffer_name =
        GetMeshVertexArrayIdxsBufferName(mesh_idx);
    // Get the array indexes
    const std::vector<size_t> &idxs = mesh.GetIdxs();
    // Get the textures
    const std::set<as::Texture> &textures = mesh.GetTextures();
    /* Update textures */
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      // Bind the texture
      texture_manager.BindTexture(path);
      // Get the unit index
      const GLuint unit_idx = texture_manager.GetUnitIdx(path);
      // Set the texture handler to the unit index
      uniform_manager.SetUniform1Int(program_name, "tex_hdlr", unit_idx);
    }
    /* Draw vertex arrays */
    UseMesh(mesh_idx);
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, 0);
  }
}

/*******************************************************************************
 * State Updating Methods
 ******************************************************************************/

void shader::SceneShader::UpdateGlobalMvp(const GlobalMvp &global_mvp) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string &buffer_name = GetGlobalMvpBufferName();
  // Update global MVP
  global_mvp_ = global_mvp;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateModelTrans(const ModelTrans &model_trans) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update model trans
  model_trans_ = model_trans;
  // Update the buffer
  const std::string &buffer_name = GetModelTransBufferName();
  buffer_manager.UpdateBuffer(buffer_name);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::SceneShader::GetId() const { return "scene"; }

/*******************************************************************************
 * Constants (Protected)
 ******************************************************************************/

const GLsizei shader::SceneShader::kNumMipmapLevels = 5;

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::SceneShader::GetGlobalMvpBufferName() const {
  return "global_mvp";
}

std::string shader::SceneShader::GetModelTransBufferName() const {
  return "model_trans";
}

std::string shader::SceneShader::GetGlobalMvpUniformBlockName() const {
  return "GlobalMvp";
}

std::string shader::SceneShader::GetModelTransUniformBlockName() const {
  return "ModelTrans";
}
