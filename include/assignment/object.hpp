#pragma once

#include <vector>
#include <string>

#include <TinyOBJ/tiny_obj_loader.h>

#include "assignment/common.hpp"

void TinyobjLoadObj(const std::string &path, std::vector<glm::vec3> &vertices) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str());

  if (!err.empty()) { // `err` may contain warning message.
    std::cerr << err << std::endl;
  }
  if (!ret) {
    exit(1);
  }

  vertices.clear();

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    //std::cerr << "shape " << s << std::endl;

    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      //std::cerr << "face " << f << std::endl;

      int fv = shapes[s].mesh.num_face_vertices[f];

      // Loop over vertices in the face.
      for (int v = 0; v < fv; v++) {
        //std::cerr << "vertice " << v << std::endl;

        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
        tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
        tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
        tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
        tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
        tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
        tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
        tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];

        //std::cerr << "vxyz = (" << vx << ", " << vy << ", " << vz <<
        //  "), nxyz = (" << nx << ", " << ny << ", " << nz <<
        //  "), txy = (" << tx << ", " << ty << ")" << std::endl;

        vertices.push_back(glm::vec3(vx, vy, vz));

        // Optional: vertex colors
        // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
        // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
        // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
      }
      index_offset += fv;

      // per-face material
      shapes[s].mesh.material_ids[f];
    }
  }
}
