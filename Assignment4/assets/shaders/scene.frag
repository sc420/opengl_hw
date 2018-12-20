#version 440

/*******************************************************************************
 * Constants
 ******************************************************************************/

const float kPi = 3.1415926535897932384626433832795;
const float kEnvMapBlendRatio = 0.35f;
const float kParallaxHeightScale = 0.05f;
const float kParallaxMapMinNumLayers = 4.0f;
const float kParallaxMapMaxNumLayers = 8.0f;

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

layout(std140) uniform ModelMaterial {
  bool use_ambient_tex;
  bool use_diffuse_tex;
  bool use_specular_tex;
  bool use_height_tex;
  bool use_normals_tex;
  vec4 ambient_color;
  vec4 diffuse_color;
  vec4 specular_color;
  float shininess;
}
model_material;

layout(std140) uniform Lighting {
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
  if (model_material.use_normals_tex) {
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
  if (!model_material.use_height_tex) {
    return tex_coords;
  }
  const vec3 view_dir = GetTangentViewDir();
  // The more perpendicular of a view direction, the more number of layers
  const float num_layers =
      mix(kParallaxMapMaxNumLayers, kParallaxMapMinNumLayers, abs(view_dir.z));
  // Calculate the height of each layer
  const float layer_height = 1.0f / num_layers;
  // Calculate the amount to shift the texture coordinates per layer
  const vec2 ofs = (-view_dir.xy) / num_layers * kParallaxHeightScale;
  // Find the layer which is deeper than the depth from texture
  float cur_layer_depth = 0.0f;
  vec2 cur_tex_coords = tex_coords;
  float cur_depth = texture(height_tex, cur_tex_coords).x;
  while (cur_layer_depth < cur_depth) {
    // Shift texture coordinates along the opposite of viewing direction
    cur_tex_coords += ofs;
    // Get depth at current texture coordinates
    cur_depth = texture(height_tex, cur_tex_coords).x;
    // Get depth of the next layer
    cur_layer_depth += layer_height;
  }
  // Calculate the previous texture coordinates
  const vec2 prev_tex_coords = cur_tex_coords - ofs;
  // Get errors after and before collision for linear interpolation
  const float prev_layer_depth = cur_layer_depth - layer_height;
  const float cur_error = cur_depth - cur_layer_depth;
  const float prev_error =
      prev_layer_depth - texture(height_tex, prev_tex_coords).x;
  // Calculate interpolation of texture coordinates
  const float weight = cur_error / (cur_error + prev_error);
  return prev_tex_coords * weight + cur_tex_coords * (1.0f - weight);
}

vec4 GetParallaxMappingColor(sampler2D tex) {
  // Calculate parallax mapped texture coordinates
  const vec2 parallax_tex_coords = CalcParallaxMappingTexCoords(tex);
  // Check whether the coordinates are out of range
  //  const vec2 bottom_left = vec2(0.0f);
  //  const vec2 top_right = vec2(1.0f);
  //  if (CheckInsideBox(parallax_tex_coords, bottom_left, top_right) <= 0.0f) {
  //    discard;
  //  }
  return texture(tex, parallax_tex_coords);
}

/*******************************************************************************
 * Blinn-Phong Model Methods
 ******************************************************************************/

vec4 GetAmbientColor() {
  vec4 tex_color;
  if (model_material.use_ambient_tex) {
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
  if (model_material.use_diffuse_tex) {
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
  if (model_material.use_specular_tex) {
    tex_color = GetParallaxMappingColor(specular_tex);
  } else {
    tex_color = model_material.specular_color;
  }
  const vec3 norm = GetTangentNorm();
  const vec3 halfway_dir = GetTangentHalfwayDir();
  const float shininess = model_material.shininess;
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

  fs_color = GetEnvironmentMapColor();  // TODO: debug
}
