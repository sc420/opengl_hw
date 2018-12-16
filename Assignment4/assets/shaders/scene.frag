#version 440

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

uniform Lighting {
  vec4 color;
  vec4 pos;
}
lighting;

/*******************************************************************************
 * Textures
 ******************************************************************************/

uniform sampler2D tex_hdlr;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in VSTex { vec2 coords; }
vs_tex;

layout(location = 1) in VSSurface {
  vec3 pos;
  vec3 norm;
}
vs_surface;

/*******************************************************************************
 * Outputs
 ******************************************************************************/

layout(location = 0) out vec4 fs_color;

void main() {
  const vec4 tex_color = texture(tex_hdlr, vs_tex.coords);
  const vec3 norm = normalize(vs_surface.norm);
  const vec3 light_dir = normalize(vec3(lighting.pos) - vs_surface.pos);
  const float diffuse_intensity = max(dot(norm, light_dir), 0.0f);
  const vec4 diffuse_color = diffuse_intensity * lighting.color;
  fs_color = diffuse_color * tex_color;
}
