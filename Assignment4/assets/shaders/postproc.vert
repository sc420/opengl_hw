#version 440

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_tex_coords;

layout(location = 0) out vec2 vs_tex_coords;

void main() {
  gl_Position = vec4(vec2(in_pos), 1.0f, 1.0f);
  vs_tex_coords = in_tex_coords;
}
