#version 120
#extension GL_EXT_gpu_shader4 : require

varying vec4 position_cs;

void main()
{
    gl_Position = ftransform();
	position_cs = gl_ModelViewMatrix * gl_Vertex;
}