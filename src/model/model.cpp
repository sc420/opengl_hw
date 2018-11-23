#include "as/model/model.hpp"

void as::Model::LoadFile(const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      path, aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate);
  // Check errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cerr << "Assimp error:" << std::endl;
    std::cerr << importer.GetErrorString() << std::endl;
    throw std::runtime_error("Could not read the file by Assimp");
  }
  const std::string dir = path.substr(0, path.find_last_of('/'));
  // Reset the model
  Reset();
  // Process the root node
  ProcessNode(scene->mRootNode, scene);
}

const std::vector<as::Node> &as::Model::GetNodes() const { return nodes_; }

const std::vector<as::Mesh> &as::Model::GetMeshes() const { return meshes_; }

void as::Model::Reset() {
  nodes_.clear();
  meshes_.clear();
}

void as::Model::ProcessNode(const aiNode *ai_node, const aiScene *ai_scene) {
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
      const Mesh mesh = ProcessMesh(ai_mesh, ai_scene);
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

const as::Mesh as::Model::ProcessMesh(const aiMesh *ai_mesh,
                                      const aiScene *ai_scene) {
  const std::vector<Vertex> vertices = ProcessMeshVertices(ai_mesh);
  const std::vector<size_t> idxs = ProcessMeshIdxs(ai_mesh);
  const std::set<Texture> textures = ProcessMeshTextures(ai_mesh, ai_scene);
  return Mesh(ai_mesh->mName.C_Str(), vertices, idxs, textures);
}

std::vector<as::Vertex> as::Model::ProcessMeshVertices(
    const aiMesh *ai_mesh) const {
  std::vector<Vertex> vertices;
  for (size_t vtx_idx = 0; vtx_idx < ai_mesh->mNumVertices; vtx_idx++) {
    Vertex vertex;
    const aiVector3D &m_vertex = ai_mesh->mVertices[vtx_idx];
    const aiVector3D &m_normal = ai_mesh->mNormals[vtx_idx];
    vertex.pos = glm::vec3(m_vertex.x, m_vertex.y, m_vertex.z);
    vertex.normal = glm::vec3(m_normal.x, m_normal.y, m_normal.z);
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

std::vector<size_t> as::Model::ProcessMeshIdxs(const aiMesh *ai_mesh) const {
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

std::set<as::Texture> as::Model::ProcessMeshTextures(
    const aiMesh *ai_mesh, const aiScene *ai_scene) const {
  // if (ai_mesh->mMaterialIndex >= 0) {
  const aiMaterial *ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
  std::set<Texture> diffuse_textures =
      ProcessMaterialTextures(ai_material, aiTextureType_DIFFUSE);
  return diffuse_textures;
  //}
}

std::set<as::Texture> as::Model::ProcessMaterialTextures(
    const aiMaterial *ai_material, const aiTextureType ai_texture_type) const {
  std::set<Texture> textures;
  for (size_t i = 0; i < ai_material->GetTextureCount(ai_texture_type); i++) {
    aiString path;
    ai_material->GetTexture(ai_texture_type, i, &path);
    const std::string type = AiTextureTypeToStr(ai_texture_type);
    const Texture texture = Texture(path.C_Str(), type);
    textures.insert(texture);
  }
  return textures;
}

std::string as::Model::AiTextureTypeToStr(
    const aiTextureType ai_texture_type) const {
  switch (ai_texture_type) {
    case aiTextureType_DIFFUSE: {
      return "DIFFUSE";
    } break;
    case aiTextureType_SPECULAR: {
      return "SPECULAR";
    } break;
    case aiTextureType_AMBIENT: {
      return "AMBIENT";
    } break;
    case aiTextureType_EMISSIVE: {
      return "EMISSIVE";
    } break;
    case aiTextureType_HEIGHT: {
      return "HEIGHT";
    } break;
    case aiTextureType_NORMALS: {
      return "NORMALS";
    } break;
    case aiTextureType_SHININESS: {
      return "SHININESS";
    } break;
    case aiTextureType_OPACITY: {
      return "OPACITY";
    } break;
    case aiTextureType_DISPLACEMENT: {
      return "DISPLACEMENT";
    } break;
    case aiTextureType_LIGHTMAP: {
      return "LIGHTMAP";
    } break;
    case aiTextureType_REFLECTION: {
      return "REFLECTION";
    } break;
    case aiTextureType_UNKNOWN: {
      return "UNKNOWN";
    } break;
    default: {
      throw std::runtime_error("Unknown texture type '" +
                               std::to_string(ai_texture_type) + "'");
    }
  }
}
