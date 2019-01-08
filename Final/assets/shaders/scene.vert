#version 440

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

layout(std140) uniform GlobalTrans {
  mat4 model;
  mat4 view;
  mat4 proj;
}
global_trans;

layout(std140) uniform ModelTrans { mat4 trans; }
model_trans;

layout(std140) uniform Lighting {
  mat4 fixed_norm_model;
  mat4 light_trans;
  vec3 light_color;
  vec3 light_pos;
  vec3 light_intensity;
  vec3 view_pos;
}
lighting;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coords;
layout(location = 2) in vec3 in_norm;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_instancing_translation;
layout(location = 5) in vec3 in_instancing_rotation;
layout(location = 6) in vec3 in_instancing_scaling;

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

layout(location = 8) out VSDepth { vec4 light_space_pos; }
vs_depth;

/*******************************************************************************
 * Quaternion Calculations
 ******************************************************************************/

vec4 NormalizeQuat(const vec4 quat) { return normalize(quat); }

// Reference:
// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
vec4 EulerAnglesToQuat(const vec3 euler_angles) {
  const float cy = cos(euler_angles.z * 0.5f);
  const float sy = sin(euler_angles.z * 0.5f);
  const float cp = cos(euler_angles.y * 0.5f);
  const float sp = sin(euler_angles.y * 0.5f);
  const float cr = cos(euler_angles.x * 0.5f);
  const float sr = sin(euler_angles.x * 0.5f);
  vec4 quat;
  quat.w = cy * cp * cr + sy * sp * sr;
  quat.x = cy * cp * sr - sy * sp * cr;
  quat.y = sy * cp * sr + cy * sp * cr;
  quat.z = sy * cp * cr - cy * sp * sr;
  return quat;
}

// Reference: https://stackoverflow.com/a/1556470
mat4 QuatToRotationMatrix(vec4 quat) {
  mat4 rotation_mat;
  rotation_mat[0] =
      vec4(1.0f - 2.0f * quat[1] * quat[1] - 2.0f * quat[2] * quat[2],
           2.0f * quat[0] * quat[1] + 2.0f * quat[2] * quat[3],
           2.0f * quat[0] * quat[2] - 2.0f * quat[1] * quat[3], 0.0f);
  rotation_mat[1] =
      vec4(2.0f * quat[0] * quat[1] - 2.0f * quat[2] * quat[3],
           1.0f - 2.0f * quat[0] * quat[0] - 2.0f * quat[2] * quat[2],
           2.0f * quat[1] * quat[2] + 2.0f * quat[0] * quat[3], 0.0f);
  rotation_mat[2] =
      vec4(2.0f * quat[0] * quat[2] + 2.0f * quat[1] * quat[3],
           2.0f * quat[1] * quat[2] - 2.0f * quat[0] * quat[3],
           1.0f - 2.0f * quat[0] * quat[0] - 2.0f * quat[1] * quat[1], 0.0f);
  rotation_mat[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  return rotation_mat;
}

/*******************************************************************************
 * Transformations
 ******************************************************************************/

mat3 CalcWorldToTangConverter() {
  const mat3 fixed_norm_model = mat3(lighting.fixed_norm_model);
  const vec3 tangent_n = normalize(fixed_norm_model * in_norm);
  const vec3 tangent_t = normalize(fixed_norm_model * in_tangent);
  const vec3 ortho_tangent_t =
      normalize(tangent_t - dot(tangent_t, tangent_n) * tangent_n);
  const vec3 ortho_tangent_b = cross(tangent_n, ortho_tangent_t);
  return transpose(mat3(ortho_tangent_t, ortho_tangent_b, tangent_n));
}

mat4 CalcInstancingTrans() {
  mat4 translation_mat;
  mat4 scaling_mat;

  translation_mat[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
  translation_mat[1] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  translation_mat[2] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
  translation_mat[3] = vec4(in_instancing_translation, 1.0f);

  scaling_mat[0] = vec4(in_instancing_scaling[0], 0.0f, 0.0f, 0.0f);
  scaling_mat[1] = vec4(0.0f, in_instancing_scaling[1], 0.0f, 0.0f);
  scaling_mat[2] = vec4(0.0f, 0.0f, in_instancing_scaling[2], 0.0f);
  scaling_mat[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

  return translation_mat *
         QuatToRotationMatrix(EulerAnglesToQuat(in_instancing_rotation)) *
         scaling_mat;
}

mat4 CalcTrans() {
  return global_trans.proj * global_trans.view * global_trans.model *
         model_trans.trans * CalcInstancingTrans();
}

mat4 CalcModel() {
  return global_trans.model * model_trans.trans * CalcInstancingTrans();
}

/*******************************************************************************
 * Lighting Calculations
 ******************************************************************************/

void OutputTangentLighting() {
  // Calculate the model
  const mat4 model = CalcModel();
  // Calculate world to tangent space converter
  const mat3 world_to_tang = CalcWorldToTangConverter();
  // Calculate tangent to world space converter
  vs_tangent_lighting.tang_to_world_conv = transpose(world_to_tang);
  // Calculate positions, normal, light position and view position in tangent
  // space
  vs_tangent_lighting.pos = world_to_tang * vec3(model * vec4(in_pos, 1.0f));
  vs_tangent_lighting.norm = world_to_tang * mat3(model) * in_norm;
  vs_tangent_lighting.light_pos = world_to_tang * lighting.light_pos;
  vs_tangent_lighting.view_pos = world_to_tang * lighting.view_pos;
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
  // Calculate light space vertex position
  vs_depth.light_space_pos =
      lighting.light_trans * model_trans.trans * vec4(in_pos, 1.0f);
}
