#version 120

varying vec4 sPos;

void main(void)
{
	vec4 pos = ftransform();
	gl_Position = pos;
	sPos = pos;
}