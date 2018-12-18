#version 440

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

uniform GlobalTrans {
  mat4 model;
  mat4 view;
  mat4 proj;
}
global_trans;

uniform ModelTrans { mat4 trans; }
model_trans;

uniform Lighting {
  mat4 fixed_norm_model;
  vec4 light_color;
  vec4 light_pos;
  vec4 light_intensity;
  vec4 view_pos;
}
lighting;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coords;
layout(location = 2) in vec3 in_norm;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;

/*******************************************************************************
 * Outputs
 ******************************************************************************/

layout(location = 0) out VSTex { vec2 coords; }
vs_tex;

layout(location = 1) out VSTangentLighting {
  mat3 tang_to_world_conv;
  vec3 pos;
  vec3 norm;
  vec3 light_pos;
  vec3 view_pos;
}
vs_tangent_lighting;

/*******************************************************************************
 * Transformations
 ******************************************************************************/

mat4 CalcTrans() {
  return global_trans.proj * global_trans.view * global_trans.model *
         model_trans.trans;
}

mat4 CalcModel() { return global_trans.model * model_trans.trans; }

/*******************************************************************************
 * Lighting Calculations
 ******************************************************************************/

void OutputTangentLighting() {
  // Calculate the model
  const mat4 model = CalcModel();
  // Calculate world to tangent space converter
  const mat3 fixed_norm_model = mat3(lighting.fixed_norm_model);
  const vec3 tangent_t = normalize(fixed_norm_model * in_tangent);
  const vec3 tangent_b = normalize(fixed_norm_model * in_bitangent);
  const vec3 tangent_n = normalize(fixed_norm_model * in_norm);
  const mat3 world_to_tang_conv =
      transpose(mat3(tangent_t, tangent_b, tangent_n));
  // Calculate tangent to world space converter
  vs_tangent_lighting.tang_to_world_conv = transpose(world_to_tang_conv);
  // Calculate positions, normal, light position and view position in tangent
  // space
  vs_tangent_lighting.pos =
      world_to_tang_conv * vec3(model * vec4(in_pos, 1.0f));
  vs_tangent_lighting.norm =
      world_to_tang_conv * vec3(model * vec4(in_norm, 1.0f));
  vs_tangent_lighting.light_pos = world_to_tang_conv * vec3(lighting.light_pos);
  vs_tangent_lighting.view_pos = world_to_tang_conv * vec3(lighting.view_pos);
}

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

void main() {
  // Calculate vertex position
  gl_Position = CalcTrans() * vec4(in_pos, 1.0f);
  // Pass texture coordinates
  vs_tex.coords = in_tex_coords;
  // Calculate tangent lighting
  OutputTangentLighting();
}
