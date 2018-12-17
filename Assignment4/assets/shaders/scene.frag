#version 440

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

uniform ModelMaterial {
  vec4 ambient_color;
  vec4 diffuse_color;
  vec4 specular_color;
}
model_material;

uniform Lighting {
  vec4 light_color;
  vec4 light_pos;
  vec4 light_intensity;
  vec4 view_pos;
}
lighting;

/*******************************************************************************
 * Textures
 ******************************************************************************/

uniform sampler2D ambient_tex;
uniform sampler2D diffuse_tex;
uniform sampler2D specular_tex;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in VSTex { vec2 coords; }
vs_tex;

layout(location = 1) in VSSurface {
  vec3 frag_pos;
  vec3 frag_norm;
}
vs_surface;

/*******************************************************************************
 * Outputs
 ******************************************************************************/

layout(location = 0) out vec4 fs_color;

void main() {
  // Get texture color
  const vec4 tex_ambient_color = texture(ambient_tex, vs_tex.coords);
  const vec4 tex_diffuse_color = texture(diffuse_tex, vs_tex.coords);
  const vec4 tex_specular_color = texture(specular_tex, vs_tex.coords);
  // Get directional vectors
  const vec3 norm = normalize(vs_surface.frag_norm);
  const vec3 light_dir =
      normalize(vec3(lighting.light_pos) - vs_surface.frag_pos);
  const vec3 reflect_dir = reflect(-light_dir, norm);
  const vec3 view_dir =
      normalize(vec3(lighting.view_pos) - vs_surface.frag_pos);
  // Calculate ambient color
  const vec4 ambient_color = lighting.light_intensity.x * lighting.light_color;
  // Calculate diffuse color
  const float diffuse_strength = max(dot(norm, light_dir), 0.0f);
  const vec4 diffuse_color =
      lighting.light_intensity.y * diffuse_strength * lighting.light_color;
  // Calculate specular color
  const float specular_strength =
      pow(max(dot(view_dir, reflect_dir), 0.0f), 8.0f);
  const vec4 specular_color =
      lighting.light_intensity.z * specular_strength * lighting.light_color;
  // Calculate lighting color
  const vec4 lighting_color = ambient_color +
                              diffuse_color * tex_diffuse_color +
                              specular_color * tex_specular_color;

  fs_color = tex_specular_color;
}
