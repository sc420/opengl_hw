#version 440

uniform sampler2D screen_tex;

layout(location = 0) in vec2 vs_tex_coords;

layout(location = 0) out vec4 fs_color;

void main() { fs_color = texture(screen_tex, vs_tex_coords); }
