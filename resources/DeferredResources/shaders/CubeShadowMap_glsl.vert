#version 120
#extension GL_EXT_gpu_shader4 : require

varying vec4 position_cs;
varying vec3 normal_cs;
varying vec3 color;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	position_cs = gl_ModelViewMatrix * gl_Vertex;
	normal_cs = gl_NormalMatrix * gl_Normal;
	color = gl_Color.rgb;
}