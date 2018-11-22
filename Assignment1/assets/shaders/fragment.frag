#version 440

uniform sampler2D tex_hdlr;

layout(location = 0) in vec2 vs_tex_coord;
layout(location = 1) in vec3 vs_color;

layout(location = 0) out vec4 fs_color;

vec2 GetReverseTextureCoord(const vec2 coord) {
  return vec2(coord.x, 1.0f - coord.y);
}

void main() {
  vec2 rev_tex_coord = GetReverseTextureCoord(vs_tex_coord);
  vec4 tex_color = texture(tex_hdlr, rev_tex_coord, 0);
  vec4 mixed_color = (tex_color + vec4(vs_color, 1.0f)) / 2.0f;
  fs_color = mixed_color;
}
