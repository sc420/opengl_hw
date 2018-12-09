#version 440

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 vs_tex_coords;

vec4 ForceDepthTo1(const vec4 pos) { return vec4(pos.x, pos.y, pos.z, pos.z); }

void main() {
  gl_Position = vec4(in_pos.x, in_pos.y, 0.0, 1.0);
  //  gl_Position = ForceDepthTo1(gl_Position);
  vs_tex_coords = in_tex_coords;
}
