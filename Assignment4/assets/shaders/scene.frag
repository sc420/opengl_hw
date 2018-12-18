#version 440

/*******************************************************************************
 * Constants
 ******************************************************************************/

const float kPi = 3.1415926535897932384626433832795;

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

uniform ModelMaterial {
  ivec4 use_ambient_tex;
  ivec4 use_diffuse_tex;
  ivec4 use_specular_tex;
  ivec4 use_normals_tex;
  vec4 ambient_color;
  vec4 diffuse_color;
  vec4 specular_color;
  vec4 shininess;
}
model_material;

uniform Lighting {
  mat4 fixed_norm_model;
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
uniform sampler2D normals_tex;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in VSTex { vec2 coords; }
vs_tex;

layout(location = 1) in VSTangentLighting {
  vec3 pos;
  vec3 norm;
  vec3 light_pos;
  vec3 view_pos;
}
vs_tangent_lighting;

/*******************************************************************************
 * Outputs
 ******************************************************************************/

layout(location = 0) out vec4 fs_color;

/*******************************************************************************
 * Blinn-Phong Model Methods
 ******************************************************************************/

vec3 GetNorm() {
  if (model_material.use_normals_tex.x > 0) {
    const vec3 norm = vec3(texture(normals_tex, vs_tex.coords));
    return normalize(norm * 2.0f - 1.0f);
  } else {
    return normalize(vs_tangent_lighting.norm);
  }
}

vec3 GetLightDir() {
  return normalize(vs_tangent_lighting.light_pos - vs_tangent_lighting.pos);
}

vec3 GetViewDir() {
  return normalize(vs_tangent_lighting.view_pos - vs_tangent_lighting.pos);
}

vec3 GetHalfwayDir() { return normalize(GetLightDir() + GetViewDir()); }

vec4 GetAmbientColor() {
  vec4 tex_color;
  if (model_material.use_ambient_tex.x > 0) {
    tex_color = texture(ambient_tex, vs_tex.coords);
  } else {
    tex_color = model_material.ambient_color;
  }
  const vec4 affecting_color =
      lighting.light_intensity.x * lighting.light_color;
  return affecting_color * tex_color;
}

vec4 GetDiffuseColor() {
  vec4 tex_color;
  if (model_material.use_diffuse_tex.x > 0) {
    tex_color = texture(diffuse_tex, vs_tex.coords);
  } else {
    tex_color = model_material.diffuse_color;
  }
  const vec3 norm = GetNorm();
  const vec3 light_dir = GetLightDir();
  const float diffuse_strength = max(dot(norm, light_dir), 0.0f);
  const vec4 affecting_color =
      lighting.light_intensity.y * diffuse_strength * lighting.light_color;
  return affecting_color * tex_color;
}

vec4 GetSpecularColor() {
  vec4 tex_color;
  if (model_material.use_specular_tex.x > 0) {
    tex_color = texture(specular_tex, vs_tex.coords);
  } else {
    tex_color = model_material.specular_color;
  }
  const vec3 norm = GetNorm();
  const vec3 halfway_dir = GetHalfwayDir();
  const float shininess = model_material.shininess.x;
  const float energy_conservation = (8.0f + shininess) / (8.0f * kPi);
  const float specular_strength =
      pow(max(dot(norm, halfway_dir), 0.0f), shininess);
  const vec4 affecting_color = lighting.light_intensity.z *
                               energy_conservation * specular_strength *
                               lighting.light_color;
  return affecting_color * tex_color;
}

vec4 GetBlinnPhongColor() {
  const vec4 ambient_color = GetAmbientColor();
  const vec4 diffuse_color = GetDiffuseColor();
  const vec4 specular_color = GetSpecularColor();
  return ambient_color + diffuse_color + specular_color;
}

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

void main() { fs_color = GetBlinnPhongColor(); }
