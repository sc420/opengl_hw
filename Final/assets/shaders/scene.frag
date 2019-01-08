#version 440

/*******************************************************************************
 * Constants
 ******************************************************************************/

const float kPi = 3.1415926535897932384626433832795f;
const float kDiscardAlphaHigh = 0.1f;
const float kEnvMapBlendRatio = 0.35f;
const float kParallaxHeightScale = 0.01f;
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
  bool use_env_map;
}
model_material;

layout(std140) uniform Lighting {
  mat4 light_trans;
  vec3 light_color;
  vec3 light_pos;
  vec3 light_intensity;
  vec3 view_pos;
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
uniform sampler2D depth_map_tex;
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

layout(location = 8) in VSDepth { vec4 light_space_pos; }
vs_depth;

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
 * Shadow Calculations
 ******************************************************************************/

float CalcShadow() {
  // Set the bias
  const float bias = 1e-4f;
  // Perform perspective divide
  vec3 proj_coords =
      vec3(vs_depth.light_space_pos) / vs_depth.light_space_pos.w;
  // Transform to [0,1] range
  proj_coords = proj_coords * 0.5f + 0.5f;
  // Get depth of current fragment from light's perspective
  const float view_depth = proj_coords.z;
  // Check whether the coordinates is further than the far plane
  if (view_depth > 1.0f) {
    return 0.0f;
  }
  // Perform PCF
  float shadow = 0.0f;
  const vec2 scale = 1.0f / textureSize(depth_map_tex, 0);
  for (float x = -1.0f; x <= 1.0f; x++) {
    for (float y = -1.0f; y <= 1.0f; y++) {
      const float light_depth =
          texture(depth_map_tex, proj_coords.xy + vec2(x, y) * scale).x;
      shadow += view_depth - bias > light_depth ? 1.0f : 0.0f;
    }
  }
  shadow /= 9.0f;
  return shadow;
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
      vec4(lighting.light_intensity.x * lighting.light_color, 1.0f);
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
      vec4(lighting.light_intensity.y * diffuse_strength * lighting.light_color,
           1.0f);
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
  const vec4 affecting_color =
      vec4(lighting.light_intensity.z * energy_conservation *
               specular_strength * lighting.light_color,
           1.0f);
  return affecting_color * tex_color;
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
 * Color Blending
 ******************************************************************************/

vec4 GetBlinnPhongShadowColor() {
  const vec4 ambient_color = GetAmbientColor();
  const vec4 diffuse_color = GetDiffuseColor();
  const vec4 specular_color = GetSpecularColor();
  const vec4 non_shadow = vec4(vec3(1.0f - CalcShadow()), 1.0f);
  const vec4 color =
      ambient_color + non_shadow * (diffuse_color + specular_color);
  // Check whether to discard the fragment if the alpha is too low
  if (ambient_color.a < kDiscardAlphaHigh) {
    discard;
  }
  return color;
}

vec4 CalcFinalColor() {
  // Calculate environment mapping blend ratio
  float env_map_blend_ratio = kEnvMapBlendRatio;
  if (!model_material.use_env_map) {
    env_map_blend_ratio = 0.0f;
  }
  // Blend Blinn-Phong color with environment mapped color
  return (1.0f - env_map_blend_ratio) * GetBlinnPhongShadowColor() +
         env_map_blend_ratio * GetEnvironmentMapColor();
}

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

void main() { fs_color = CalcFinalColor(); }
