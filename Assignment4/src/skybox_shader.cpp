#include "skybox_shader.hpp"

/*******************************************************************************
 * Shader Registrations
 ******************************************************************************/

void shader::SkyboxShader::RegisterSceneShader(
    const SceneShader &scene_shader) {
  scene_shader_ = &scene_shader;
}

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

void shader::SkyboxShader::LoadModel() {
  as::Model &model = GetModel();
  model.LoadFile("assets/models/san-giuseppe-bridge-low/skybox.obj",
                 aiProcess_FlipUVs);
}

/*******************************************************************************
 * GL Initializations
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
  const std::string program_name = GetProgramName();
  const std::string global_trans_buffer_name =
      scene_shader_->GetGlobalTransBufferName();
  const std::string global_trans_uniform_block_name =
      scene_shader_->GetGlobalTransUniformBlockName();
  const std::string global_trans_binding_name = global_trans_buffer_name;
  // Bind the uniform block to the the buffer
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, global_trans_uniform_block_name, global_trans_binding_name);
}

void shader::SkyboxShader::InitTextures() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  // Get names=
  const std::string tex_name = GetTextureName();
  const std::string unit_name = GetTextureName();
  // Get models
  const as::Model &model = GetModel();
  // Set the path-to-target index map
  static const std::map<std::string, size_t> path_to_target_idx = {
      {"right.jpg", 0},  {"left.jpg", 1},  {"top.jpg", 2},
      {"bottom.jpg", 3}, {"front.jpg", 4}, {"back.jpg", 5}};

  // Generate the texture
  texture_manager.GenTexture(tex_name);
  // Bind the texture
  texture_manager.BindTexture(tex_name, GL_TEXTURE_CUBE_MAP, unit_name);
  // Update the textures
  bool tex_initialized = false;
  const std::vector<as::Mesh> &meshes = model.GetMeshes();
  for (const as::Mesh &mesh : meshes) {
    const as::Material &material = mesh.GetMaterial();
    const std::set<as::Texture> &textures = material.GetTextures();
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
      // Convert the texels to 4 channels to avoid GL errors
      texels = as::ConvertDataChannels(comp, 4, texels);
      // Initialize the texture once
      const GLsizei num_mipmap_levels = GetNumMipmapLevels();
      if (!tex_initialized) {
        texture_manager.InitTexture2D(tex_name, GL_TEXTURE_CUBE_MAP,
                                      num_mipmap_levels, GL_RGBA8, width,
                                      height);
        tex_initialized = true;
      }
      // Update the texture
      texture_manager.UpdateCubeMapTexture2D(tex_name, target, 0, 0, 0, width,
                                             height, GL_RGBA, GL_UNSIGNED_BYTE,
                                             texels.data());
      texture_manager.GenMipmap(tex_name, GL_TEXTURE_CUBE_MAP);
      texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_MIN_FILTER,
                                         GL_LINEAR_MIPMAP_LINEAR);
      texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
  }
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::SkyboxShader::Draw() {
  // Get managers
  as::TextureManager &texture_manager = gl_managers_->GetTextureManager();
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
  const std::string tex_name = GetTextureName();
  // Get models
  const as::Model &model = GetModel();
  // Get meshes
  const std::vector<as::Mesh> &meshes = model.GetMeshes();

  // Use the program
  UseProgram();
  // Bind the texture
  texture_manager.BindTexture(tex_name);
  // Get the unit index
  const GLuint unit_idx = texture_manager.GetUnitIdx(tex_name);
  // Set the texture handler to the unit index
  uniform_manager.SetUniform1Int(program_name, "skybox_tex", unit_idx);
  // Draw each mesh with its own texture
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Get the array indexes
    const std::vector<size_t> &idxs = mesh.GetIdxs();
    /* Draw vertex arrays */
    UseMesh(mesh_idx);
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, nullptr);
  }
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::SkyboxShader::GetId() const { return "skybox"; }

std::string shader::SkyboxShader::GetTextureName() const {
  return GetProgramName() + "/skybox";
}

/*******************************************************************************
 * Model Handlers (Protected)
 ******************************************************************************/

as::Model &shader::SkyboxShader::GetModel() { return skybox_model_; }

/*******************************************************************************
 * GL Initializations (Protected)
 ******************************************************************************/

GLsizei shader::SkyboxShader::GetNumMipmapLevels() const { return 3; }
