#version 440

uniform samplerCube skybox_tex;

layout(location = 0) in vec3 vs_tex_coords;

layout(location = 0) out vec4 fs_color;

void main() { fs_color = texture(skybox_tex, vs_tex_coords); }
