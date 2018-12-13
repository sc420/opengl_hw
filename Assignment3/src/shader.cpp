#include "shader.hpp"

namespace fs = std::experimental::filesystem;

/*******************************************************************************
 * GL Initialization Methods
 ******************************************************************************/

void shader::Shader::RegisterGLManagers(as::GLManagers& gl_managers) {
  gl_managers_ = &gl_managers;
}

void shader::Shader::Init() {
  CreateShaders();
  CreatePrograms();
}

void shader::Shader::InitVertexArray(const as::Model& model) {
  as::BufferManager& buffer_manager = gl_managers_->GetBufferManager();
  as::VertexSpecManager& vertex_spec_manager =
      gl_managers_->GetVertexSpecManager();
  const std::vector<as::Mesh>& meshes = model.GetMeshes();
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh& mesh = meshes.at(mesh_idx);
    // Get names
    const std::string& va_name = GetMeshVertexArrayName(mesh_idx);
    const std::string& buffer_name = GetMeshVertexArrayBufferName(mesh_idx);
    const std::string& idxs_buffer_name =
        GetMeshVertexArrayIdxsBufferName(mesh_idx);
    // Get mesh data
    const std::vector<as::Vertex>& vertices = mesh.GetVertices();
    const std::vector<size_t>& idxs = mesh.GetIdxs();
    // Get memory size of mesh data
    const size_t vertices_mem_sz = mesh.GetVerticesMemSize();
    const size_t idxs_mem_sz = mesh.GetIdxsMemSize();
    /* Generate buffers */
    // VA
    buffer_manager.GenBuffer(buffer_name);
    // VA indexes
    buffer_manager.GenBuffer(idxs_buffer_name);
    /* Initialize buffers */
    // VA
    buffer_manager.InitBuffer(buffer_name, GL_ARRAY_BUFFER, vertices_mem_sz,
                              NULL, GL_STATIC_DRAW);
    // VA indexes
    buffer_manager.InitBuffer(idxs_buffer_name, GL_ELEMENT_ARRAY_BUFFER,
                              idxs_mem_sz, NULL, GL_STATIC_DRAW);
    /* Update buffers */
    // VA
    buffer_manager.UpdateBuffer(buffer_name, GL_ARRAY_BUFFER, 0,
                                vertices_mem_sz, vertices.data());
    // VA indexes
    buffer_manager.UpdateBuffer(idxs_buffer_name, GL_ELEMENT_ARRAY_BUFFER, 0,
                                idxs_mem_sz, idxs.data());
    /* Create vertex arrays */
    // VA
    vertex_spec_manager.GenVertexArray(va_name);
    /* Bind vertex arrays to buffers */
    // VA
    vertex_spec_manager.SpecifyVertexArrayOrg(va_name, 0, 3, GL_FLOAT, GL_FALSE,
                                              0);
    vertex_spec_manager.SpecifyVertexArrayOrg(va_name, 1, 3, GL_FLOAT, GL_FALSE,
                                              0);
    vertex_spec_manager.SpecifyVertexArrayOrg(va_name, 2, 2, GL_FLOAT, GL_FALSE,
                                              0);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(va_name, 0, 0);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(va_name, 1, 1);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(va_name, 2, 2);
    vertex_spec_manager.BindBufferToBindingPoint(
        va_name, buffer_name, 0, offsetof(as::Vertex, pos), sizeof(as::Vertex));
    vertex_spec_manager.BindBufferToBindingPoint(va_name, buffer_name, 1,
                                                 offsetof(as::Vertex, normal),
                                                 sizeof(as::Vertex));
    vertex_spec_manager.BindBufferToBindingPoint(
        va_name, buffer_name, 2, offsetof(as::Vertex, tex_coords),
        sizeof(as::Vertex));
  }
}

/*******************************************************************************
 * GL Drawing Methods
 ******************************************************************************/

void shader::Shader::UseProgram() const {
  // Get managers
  const as::ProgramManager& program_manager = gl_managers_->GetProgramManager();
  // Get names
  const std::string& program_name = GetProgramName();
  // Use the program
  program_manager.UseProgram(program_name);
}

void shader::Shader::UseMesh(const size_t mesh_idx) const {
  // Get managers
  as::BufferManager& buffer_manager = gl_managers_->GetBufferManager();
  const as::VertexSpecManager& vertex_spec_manager =
      gl_managers_->GetVertexSpecManager();
  // Get names
  const std::string& va_name = GetMeshVertexArrayName(mesh_idx);
  const std::string& buffer_name = GetMeshVertexArrayBufferName(mesh_idx);
  const std::string& idxs_buffer_name =
      GetMeshVertexArrayIdxsBufferName(mesh_idx);
  // Use the vertex array
  vertex_spec_manager.BindVertexArray(va_name);
  // Use the buffers
  buffer_manager.BindBuffer(buffer_name);
  buffer_manager.BindBuffer(idxs_buffer_name);
}

/*******************************************************************************
 * Name Management
 ******************************************************************************/

std::string shader::Shader::GetProgramName() const { return GetId(); }

std::string shader::Shader::GetMeshVertexArrayName(
    const size_t mesh_idx) const {
  return GetProgramName() + "/mesh[" + std::to_string(mesh_idx) + "]";
}

std::string shader::Shader::GetMeshVertexArrayBufferName(
    const size_t mesh_idx) const {
  return GetProgramName() + "/va[" + std::to_string(mesh_idx) + "]";
}

std::string shader::Shader::GetMeshVertexArrayIdxsBufferName(
    const size_t mesh_idx) const {
  return GetProgramName() + "/va_idxs[" + std::to_string(mesh_idx) + "]";
}

/*******************************************************************************
 * GL Initialization Methods (Private)
 ******************************************************************************/

void shader::Shader::CreateShaders() {
  as::ShaderManager& shader_manager = gl_managers_->GetShaderManager();
  const std::string& vertex_path = GetShaderPath(ShaderTypes::kVertex);
  const std::string& fragment_path = GetShaderPath(ShaderTypes::kFragment);
  shader_manager.CreateShader(vertex_path, GL_VERTEX_SHADER, vertex_path);
  shader_manager.CreateShader(fragment_path, GL_FRAGMENT_SHADER, fragment_path);
}

void shader::Shader::CreatePrograms() {
  as::ProgramManager& program_manager = gl_managers_->GetProgramManager();
  const std::string& program_name = GetProgramName();
  const std::string& vertex_path = GetShaderPath(ShaderTypes::kVertex);
  const std::string& fragment_path = GetShaderPath(ShaderTypes::kFragment);
  program_manager.CreateProgram(program_name);
  program_manager.AttachShader(program_name, vertex_path);
  program_manager.AttachShader(program_name, fragment_path);
  program_manager.LinkProgram(program_name);
}

/*******************************************************************************
 * Path Management (Private)
 ******************************************************************************/

std::string shader::Shader::GetShaderPath(
    const ShaderTypes& shader_type) const {
  const std::string& id = GetId();
  std::string ext;
  switch (shader_type) {
    case ShaderTypes::kVertex: {
      ext = ".vert";
    } break;
    case ShaderTypes::kFragment: {
      ext = ".frag";
    } break;
    default: { throw std::runtime_error("Unknown shader type"); }
  }
  fs::path path("assets/shaders");
  path = path / (id + ext);
  return path.string();
}
