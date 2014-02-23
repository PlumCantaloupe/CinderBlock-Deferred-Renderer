#version 120

varying vec4 sPos;

void main(void)
{
	gl_Position = ftransform();
    sPos = gl_Position;
}