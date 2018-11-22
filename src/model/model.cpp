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
  std::queue<const Node *> parents;
  waiting_nodes.push(ai_node);
  parents.push(nullptr);
  while (!waiting_nodes.empty()) {
    // Get the current node from the queue
    const aiNode *cur_ai_node = waiting_nodes.front();
    waiting_nodes.pop();
    // Get the current parent from the queue
    const Node *parent = parents.front();
    parents.pop();
    // Create a node
    Node node(ai_node->mName.C_Str(), parent);
    // Process each mesh in the node
    for (size_t i = 0; i < cur_ai_node->mNumMeshes; i++) {
      const aiMesh *ai_mesh = ai_scene->mMeshes[cur_ai_node->mMeshes[i]];
      Mesh mesh = ProcessMesh(ai_mesh, ai_scene);
      node.AddMesh(&mesh);
      // Add the mesh to the list
      meshes_.push_back(mesh);
    }
    // Add the children nodes to the waiting nodes
    for (size_t i = 0; i < cur_ai_node->mNumChildren; i++) {
      waiting_nodes.push(cur_ai_node->mChildren[i]);
      parents.push(&node);
    }
    // Add the current node to the list
    nodes_.push_back(node);
  }
}

as::Mesh as::Model::ProcessMesh(const aiMesh *ai_mesh,
                                const aiScene *ai_scene) {
  std::vector<Vertex> vertices;
  std::vector<size_t> idxs;
  // Iterate through each vertex
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
  // Iterate through each face
  for (size_t face_idx = 0; face_idx < ai_mesh->mNumFaces; face_idx++) {
    const aiFace &face = ai_mesh->mFaces[face_idx];
    // Iterate through each triangle index
    for (size_t tri_idx = 0; tri_idx < face.mNumIndices; tri_idx++) {
      idxs.push_back(face.mIndices[tri_idx]);
    }
  }
  return Mesh(vertices, idxs);
}
