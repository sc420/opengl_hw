#version 440

void main() {
  gl_FragDepth = gl_FragCoord.z;

  //  gl_FragColor = vec4(0.5f);
  //  gl_FragDepth = 0.5f;
}
