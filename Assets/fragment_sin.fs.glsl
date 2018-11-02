#version 410

in vec3 vv3color;

layout(location = 0) out vec4 fragColor;

void main()
{
	//TODO:
	//Please modify the fragment shader (Sin)
	fragColor = vec4(sin(gl_FragCoord.xyz / 10), 1.0);
}