#version 120

varying vec2 uv;

void main(void)
{
	gl_Position = ftransform();
    uv = gl_MultiTexCoord0.st;
}