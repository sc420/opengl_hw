#include "as/model/model.hpp"

void as::Model::LoadFile(const std::string &path, const unsigned int flags) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, flags);
  // Check errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error("Could not read the file by Assimp. Error: " +
                             std::string(importer.GetErrorString()));
  }
  // Get the directory
  const fs::path p(path);
  const std::string dir = p.parent_path().string();
  // Reset the model
  Reset();
  // Process the root node
  ProcessNode(dir, scene, scene->mRootNode);
}

const std::vector<as::Node> &as::Model::GetNodes() const { return nodes_; }

const std::vector<as::Mesh> &as::Model::GetMeshes() const { return meshes_; }

void as::Model::Reset() {
  nodes_.clear();
  meshes_.clear();
}

void as::Model::ProcessNode(const fs::path &dir, const aiScene *ai_scene,
                            const aiNode *ai_node) {
  std::queue<const aiNode *> waiting_nodes;
  std::queue<Node *> parents;
  waiting_nodes.push(ai_node);
  parents.push(nullptr);
  while (!waiting_nodes.empty()) {
    // Get the current node from the queue
    const aiNode *cur_ai_node = waiting_nodes.front();
    waiting_nodes.pop();
    // Get the current parent from the queue
    Node *parent = parents.front();
    parents.pop();
    // Create a node
    Node node(ai_node->mName.C_Str(), parent);
    // Process each mesh in the node
    for (size_t i = 0; i < cur_ai_node->mNumMeshes; i++) {
      const aiMesh *ai_mesh = ai_scene->mMeshes[cur_ai_node->mMeshes[i]];
      const Mesh mesh = ProcessMesh(dir, ai_scene, ai_mesh);
      node.AddMesh(&mesh);
      // Add the mesh to the list
      meshes_.push_back(mesh);
    }
    // Add the children nodes to the waiting nodes
    for (size_t i = 0; i < cur_ai_node->mNumChildren; i++) {
      waiting_nodes.push(cur_ai_node->mChildren[i]);
      parents.push(&node);
    }
    if (parent != nullptr) {
      // Add the current node to the children of the parent
      parent->AddChild(&node);
    }
    // Add the current node to the list
    nodes_.push_back(node);
  }
}

const as::Mesh as::Model::ProcessMesh(const fs::path &dir,
                                      const aiScene *ai_scene,
                                      const aiMesh *ai_mesh) {
  const std::vector<Vertex> vertices = ProcessMeshVertices(ai_mesh);
  const std::vector<size_t> idxs = ProcessMeshIdxs(ai_mesh);
  const Material material = ProcessMeshMaterial(dir, ai_scene, ai_mesh);
  return Mesh(ai_mesh->mName.C_Str(), vertices, idxs, material);
}

const std::vector<as::Vertex> as::Model::ProcessMeshVertices(
    const aiMesh *ai_mesh) const {
  std::vector<Vertex> vertices;
  for (size_t vtx_idx = 0; vtx_idx < ai_mesh->mNumVertices; vtx_idx++) {
    Vertex vertex;
    if (ai_mesh->HasPositions()) {
      const aiVector3D &m_vertex = ai_mesh->mVertices[vtx_idx];
      vertex.pos = glm::vec3(m_vertex.x, m_vertex.y, m_vertex.z);
    }
    if (ai_mesh->HasNormals()) {
      const aiVector3D &m_normal = ai_mesh->mNormals[vtx_idx];
      vertex.normal = glm::vec3(m_normal.x, m_normal.y, m_normal.z);
    }
    if (ai_mesh->mTextureCoords[0]) {
      const aiVector3D &m_tex_coords = ai_mesh->mTextureCoords[0][vtx_idx];
      vertex.tex_coords = glm::vec2(m_tex_coords.x, m_tex_coords.y);
    } else {
      vertex.tex_coords = glm::vec2(0.0f);
    }
    vertices.push_back(vertex);
  }
  return vertices;
}

const std::vector<size_t> as::Model::ProcessMeshIdxs(
    const aiMesh *ai_mesh) const {
  std::vector<size_t> idxs;
  for (size_t face_idx = 0; face_idx < ai_mesh->mNumFaces; face_idx++) {
    const aiFace &face = ai_mesh->mFaces[face_idx];
    // Iterate through each triangle index
    for (size_t tri_idx = 0; tri_idx < face.mNumIndices; tri_idx++) {
      idxs.push_back(face.mIndices[tri_idx]);
    }
  }
  return idxs;
}

const as::Material as::Model::ProcessMeshMaterial(const fs::path &dir,
                                                  const aiScene *ai_scene,
                                                  const aiMesh *ai_mesh) const {
  if (ai_mesh->mMaterialIndex < 0) {
    return Material();
  }
  // Get the material
  const aiMaterial *ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
  // Get the material colors
  const glm::vec4 ambient_color =
      GetMaterialProperty<glm::vec4, aiColor4D>(ai_material, "$clr.ambient");
  const glm::vec4 diffuse_color =
      GetMaterialProperty<glm::vec4, aiColor4D>(ai_material, "$clr.diffuse");
  const glm::vec4 specular_color =
      GetMaterialProperty<glm::vec4, aiColor4D>(ai_material, "$clr.specular");
  // Get the material shininess
  const float shininess =
      GetMaterialProperty<float, float>(ai_material, "$mat.shininess");
  // Get the material textures
  const std::set<Texture> textures = ProcessMaterialTextures(dir, ai_material);
  return Material(ambient_color, diffuse_color, specular_color, shininess,
                  textures);
}

const std::set<as::Texture> as::Model::ProcessMaterialTextures(
    const fs::path &dir, const aiMaterial *ai_material) const {
  std::set<Texture> ambient_textures =
      ProcessMaterialTexturesOfType(dir, ai_material, aiTextureType_AMBIENT);
  std::set<Texture> diffuse_textures =
      ProcessMaterialTexturesOfType(dir, ai_material, aiTextureType_DIFFUSE);
  std::set<Texture> specular_textures =
      ProcessMaterialTexturesOfType(dir, ai_material, aiTextureType_SPECULAR);
  // Merge all textures
  std::set<Texture> all_textures(ambient_textures);
  all_textures.insert(diffuse_textures.begin(), diffuse_textures.end());
  all_textures.insert(specular_textures.begin(), specular_textures.end());
  return all_textures;
}

std::set<as::Texture> as::Model::ProcessMaterialTexturesOfType(
    const fs::path &dir, const aiMaterial *ai_material,
    const aiTextureType ai_texture_type) const {
  std::set<Texture> textures;
  for (size_t i = 0; i < ai_material->GetTextureCount(ai_texture_type); i++) {
    // Get the relative path
    aiString path;
    ai_material->GetTexture(ai_texture_type, i, &path);
    // Build the full path
    const fs::path full_path = dir / fs::path(path.C_Str());
    // Get type name
    const Texture texture = Texture(full_path.string(), ai_texture_type);
    textures.insert(texture);
  }
  return textures;
}

glm::vec4 as::Model::ConvertAiColorToVec(const aiColor4D &ai_color) const {
  return glm::vec4(ai_color.r, ai_color.g, ai_color.b, ai_color.a);
}
