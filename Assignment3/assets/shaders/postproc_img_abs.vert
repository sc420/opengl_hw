#version 440

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 vs_tex_coords;

void main() {
  gl_Position = vec4(in_pos.x, in_pos.y, 0.0, 1.0);
  vs_tex_coords = in_tex_coords;
}
