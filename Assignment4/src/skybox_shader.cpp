#include "skybox_shader.hpp"

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

void shader::SkyboxShader::LoadModel() {
  as::Model &model = GetModel();
  model.LoadFile(
      "assets/models/san-giuseppe-bridge-low/skybox.obj",
      aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate);
}

/*******************************************************************************
 * GL Initialization Methods
 ******************************************************************************/

void shader::SkyboxShader::Init() {
  Shader::Init();
  LoadModel();
  InitVertexArrays();
  InitUniformBlocks();
  InitTextures();
}

void shader::SkyboxShader::InitVertexArrays() {
  const as::Model &model = GetModel();
  InitVertexArray(model);
}

void shader::SkyboxShader::InitUniformBlocks() {
  // Get managers
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string &program_name = GetProgramName();
  const std::string &global_mvp_buffer_name = GetGlobalTransBufferName();
  const std::string &global_mvp_uniform_block_name =
      GetGlobalTransUniformBlockName();
  const std::string &global_mvp_binding_name = global_mvp_buffer_name;
  // Bind the uniform block to the the buffer
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, global_mvp_uniform_block_name, global_mvp_binding_name);
}

void shader::SkyboxShader::InitTextures() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names
  const std::string &unit_name = GetProgramName();
  // Get models
  const as::Model &model = GetModel();
  // Set the path-to-target index map
  static const std::map<std::string, size_t> path_to_target_idx = {
      {"right.jpg", 0},  {"left.jpg", 1},  {"top.jpg", 2},
      {"bottom.jpg", 3}, {"front.jpg", 4}, {"back.jpg", 5}};
  const std::vector<as::Mesh> &meshes = model.GetMeshes();
  for (const as::Mesh &mesh : meshes) {
    const std::set<as::Texture> &textures = mesh.GetTextures();
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      // Check if the texture has been loaded
      if (texture_manager.HasTexture(path)) {
        continue;
      }
      // Get the file name
      const fs::path fs_path(path);
      const std::string file_name = fs_path.filename().string();
      // Calculate the target
      const size_t target_idx = path_to_target_idx.at(file_name);
      const GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + target_idx;
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
      texture_manager.BindTexture(path, GL_TEXTURE_CUBE_MAP, unit_name);
      // Initialize the texture
      const GLsizei num_mipmap_levels = GetNumMipmapLevels();
      texture_manager.InitTexture2D(path, GL_TEXTURE_CUBE_MAP,
                                    num_mipmap_levels, GL_RGBA8, width, height);
      // Update the texture
      texture_manager.UpdateCubeMapTexture2D(path, target, 0, 0, 0, width,
                                             height, GL_RGBA, GL_UNSIGNED_BYTE,
                                             texels.data());
      texture_manager.GenMipmap(path, GL_TEXTURE_CUBE_MAP);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_MIN_FILTER,
                                         GL_LINEAR_MIPMAP_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
  }
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::SkyboxShader::GetId() const { return "skybox"; }

/*******************************************************************************
 * Model Handlers (Protected)
 ******************************************************************************/

as::Model &shader::SkyboxShader::GetModel() { return skybox_model_; }
