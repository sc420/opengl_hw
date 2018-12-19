#version 440

/*******************************************************************************
 * Constants
 ******************************************************************************/

const float kPi = 3.1415926535897932384626433832795;
const float kEnvMapBlendRatio = 0.0f;  // TODO: 0.35f
const float kParallaxHeightScale = 0.05f;

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

uniform ModelMaterial {
  ivec4 use_ambient_tex;
  ivec4 use_diffuse_tex;
  ivec4 use_specular_tex;
  ivec4 use_height_tex;
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
uniform sampler2D height_tex;
uniform sampler2D normals_tex;
uniform samplerCube skybox_tex;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in VSTex { vec2 coords; }
vs_tex;

layout(location = 1) in VSTangentLighting {
  mat3 tang_to_world_mat;
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
 * Geometry Calculations
 ******************************************************************************/

// Reference: https://stackoverflow.com/a/26697650
float CheckInsideBox(vec2 coords, vec2 bottom_left, vec2 top_right) {
  const vec2 axis_results = step(bottom_left, coords) - step(top_right, coords);
  return axis_results.x * axis_results.y;
}

/*******************************************************************************
 * Tangent Space Direction Calculations
 ******************************************************************************/

vec3 GetTangentNorm() {
  if (model_material.use_normals_tex.x > 0) {
    const vec3 norm = vec3(texture(normals_tex, vs_tex.coords));
    return normalize(norm * 2.0f - 1.0f);
  } else {
    return normalize(vs_tangent_lighting.norm);
  }
}

vec3 GetTangentLightDir() {
  return normalize(vs_tangent_lighting.light_pos - vs_tangent_lighting.pos);
}

vec3 GetTangentViewDir() {
  return normalize(vs_tangent_lighting.view_pos - vs_tangent_lighting.pos);
}

vec3 GetTangentHalfwayDir() {
  return normalize(GetTangentLightDir() + GetTangentViewDir());
}

/*******************************************************************************
 * Parallax Mapping
 ******************************************************************************/

vec2 CalcParallaxMappingTexCoords(sampler2D tex) {
  const vec2 tex_coords = vs_tex.coords;
  if (model_material.use_height_tex.x <= 0) {
    return tex_coords;
  }
  const vec3 view_dir = GetTangentViewDir();

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 32.0;
  float numLayers =
      mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), view_dir)));
  // calculate the size of each layer
  float layerDepth = 1.0f / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0f;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = view_dir.xy * kParallaxHeightScale;
  vec2 deltaTexCoords = P / numLayers;

  vec2 currentTexCoords = tex_coords;
  float currentDepthMapValue = texture(height_tex, currentTexCoords).x;

  while (currentLayerDepth < currentDepthMapValue) {
    // shift texture coordinates along direction of P
    currentTexCoords -= deltaTexCoords;
    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(height_tex, currentTexCoords).x;
    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

  vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

  // get depth after and before collision for linear interpolation
  float afterDepth = currentDepthMapValue - currentLayerDepth;
  float beforeDepth =
      texture(height_tex, prevTexCoords).x - currentLayerDepth + layerDepth;

  // interpolation of texture coordinates
  float weight = afterDepth / (afterDepth - beforeDepth);
  vec2 finalTexCoords =
      prevTexCoords * weight + currentTexCoords * (1.0 - weight);

  return finalTexCoords;
}

vec4 GetParallaxMappingColor(sampler2D tex) {
  // Calculate parallax mapped texture coordinates
  const vec2 parallax_tex_coords = CalcParallaxMappingTexCoords(tex);
  // Check whether the coordinates are out of range
  const vec2 bottom_left = vec2(0.0f);
  const vec2 top_right = vec2(1.0f);
  if (CheckInsideBox(parallax_tex_coords, bottom_left, top_right) <= 0.0f) {
    discard;
  }
  return texture(tex, parallax_tex_coords);
}

/*******************************************************************************
 * Blinn-Phong Model Methods
 ******************************************************************************/

vec4 GetAmbientColor() {
  vec4 tex_color;
  if (model_material.use_ambient_tex.x > 0) {
    tex_color = GetParallaxMappingColor(ambient_tex);
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
    tex_color = GetParallaxMappingColor(diffuse_tex);
  } else {
    tex_color = model_material.diffuse_color;
  }
  const vec3 norm = GetTangentNorm();
  const vec3 light_dir = GetTangentLightDir();
  const float diffuse_strength = max(dot(norm, light_dir), 0.0f);
  const vec4 affecting_color =
      lighting.light_intensity.y * diffuse_strength * lighting.light_color;
  return affecting_color * tex_color;
}

vec4 GetSpecularColor() {
  vec4 tex_color;
  if (model_material.use_specular_tex.x > 0) {
    tex_color = GetParallaxMappingColor(specular_tex);
  } else {
    tex_color = model_material.specular_color;
  }
  const vec3 norm = GetTangentNorm();
  const vec3 halfway_dir = GetTangentHalfwayDir();
  const float shininess = model_material.shininess.x;
  const float energy_conservation = (8.0f + shininess) / (8.0f * kPi);
  const float specular_strength =
      pow(max(dot(norm, halfway_dir), 0.0f), shininess);
  const vec4 affecting_color = 0.0f * lighting.light_intensity.z *
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
 * Environment Mapping
 ******************************************************************************/

vec4 GetEnvironmentMapColor() {
  const vec3 view_dir = GetTangentViewDir();
  const vec3 reflect_dir = reflect(-view_dir, GetTangentNorm());
  const vec3 world_reflect_dir =
      vs_tangent_lighting.tang_to_world_mat * reflect_dir;
  return texture(skybox_tex, world_reflect_dir);
}

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

void main() {
  fs_color = (1.0f - kEnvMapBlendRatio) * GetBlinnPhongColor() +
             kEnvMapBlendRatio * GetEnvironmentMapColor();
}
