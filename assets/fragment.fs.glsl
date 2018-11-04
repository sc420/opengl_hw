#version 410

in vec3 vv3color;

layout(location = 0) out vec4 fragColor;

void main()
{
    const vec2 STRIDE = vec2(20, 10);
    const vec2 BRICK_SIZE = vec2(17, 7);
    const vec4 COLOR_RED = vec4(1, 0, 0, 1);
    const vec4 COLOR_WHITE = vec4(1, 1, 1, 1);

    if (mod(gl_FragCoord.x, STRIDE.x) < BRICK_SIZE.x &&
        mod(gl_FragCoord.y, STRIDE.y) < BRICK_SIZE.y) {
        fragColor = COLOR_RED;
    }
    else {
      fragColor = vec4(vv3color, 1.0); //COLOR_WHITE;
    }
}