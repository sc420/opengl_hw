#include "scene_shader.hpp"

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

shader::SceneShader::SceneShader()
    : global_trans_(GlobalTrans()),
      model_trans_(ModelTrans()),
      lighting_(Lighting()) {}

void shader::SceneShader::LoadModel() {
  as::Model &model = GetModel();
  model.LoadFile(
      "assets/models/nanosuit/nanosuit.obj",
      aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate);
}

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

void shader::SceneShader::InitVertexArrays() {
  const as::Model &model = GetModel();
  InitVertexArray(model);
}

void shader::SceneShader::InitUniformBlocks() {
  LinkDataToUniformBlock(GetGlobalTransBufferName(),
                         GetGlobalTransUniformBlockName(), global_trans_);
  LinkDataToUniformBlock(GetModelTransBufferName(),
                         GetModelTransUniformBlockName(), model_trans_);
}

void shader::SceneShader::InitTextures() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names
  const std::string &unit_name = GetProgramName();
  // Get models
  const as::Model &model = GetModel();
  // Initialize textures in each mesh
  const std::vector<as::Mesh> &meshes = model.GetMeshes();
  for (const as::Mesh &mesh : meshes) {
    const std::set<as::Texture> &textures = mesh.GetTextures();
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      // Check if the texture has been loaded
      if (texture_manager.HasTexture(path)) {
        continue;
      }
      // Load the texture
      GLsizei width, height;
      int comp;
      std::vector<GLubyte> texels;
      as::LoadTextureByStb(path, 0, width, height, comp, texels);
      // Convert the texels from 3 channels to 4 channels to avoid GL errors
      if (comp == 3) {
        texels = as::ConvertDataChannels3To4(texels);
      }
      // Generate the texture
      texture_manager.GenTexture(path);
      // Bind the texture
      texture_manager.BindTexture(path, GL_TEXTURE_2D, unit_name);
      // Initialize the texture
      const GLsizei num_mipmap_levels = GetNumMipmapLevels();
      texture_manager.InitTexture2D(path, GL_TEXTURE_2D, num_mipmap_levels,
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
  // Get models
  as::Model &model = GetModel();
  // Get meshes
  const std::vector<as::Mesh> &meshes = model.GetMeshes();

  // Use the program
  UseProgram();
  // Draw each mesh with its own texture
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
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, nullptr);
  }
}

/*******************************************************************************
 * State Updating Methods
 ******************************************************************************/

void shader::SceneShader::UpdateGlobalTrans(const GlobalTrans &global_trans) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Get names
  const std::string &buffer_name = GetGlobalTransBufferName();
  // Update global transformation
  global_trans_ = global_trans;
  // Update the buffer
  buffer_manager.UpdateBuffer(buffer_name);
}

void shader::SceneShader::UpdateModelTrans(const ModelTrans &model_trans) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  // Update model transformation
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
 * Model Handlers (Protected)
 ******************************************************************************/

as::Model &shader::SceneShader::GetModel() { return scene_model_; }

/*******************************************************************************
 * GL Initialization Methods (Protected)
 ******************************************************************************/

GLsizei shader::SceneShader::GetNumMipmapLevels() const { return 1; }

/*******************************************************************************
 * Name Management (Protected)
 ******************************************************************************/

std::string shader::SceneShader::GetGlobalTransBufferName() const {
  return "global_trans";
}

std::string shader::SceneShader::GetModelTransBufferName() const {
  return "model_trans";
}

std::string shader::SceneShader::GetGlobalTransUniformBlockName() const {
  return "GlobalTrans";
}

std::string shader::SceneShader::GetModelTransUniformBlockName() const {
  return "ModelTrans";
}
