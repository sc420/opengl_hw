#version 440

uniform sampler2D tex_hdlr;

in vec3 vs_color;

layout(location = 0) out vec4 fs_color;

void main() {
  vec2 pos = gl_FragCoord.xy;
  pos.y = 600.0f - pos.y;
  fs_color = texelFetch(tex_hdlr, ivec2(pos / 600.0f * 1667.0f), 0);
}
