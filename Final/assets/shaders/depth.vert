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

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in vec3 in_pos;

layout(location = 4) in vec3 in_instancing_translation;
layout(location = 5) in vec3 in_instancing_rotation;
layout(location = 6) in vec3 in_instancing_scaling;

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

mat4 CalcModel() {
  return global_trans.model * model_trans.trans * CalcInstancingTrans();
}

mat4 CalcTrans() { return global_trans.proj * global_trans.view * CalcModel(); }

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

void main() { gl_Position = CalcTrans() * vec4(in_pos, 1.0f); }
