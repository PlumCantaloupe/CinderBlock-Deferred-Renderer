#version 120

uniform mat4		shadowTransMatrix;
varying vec4		q;

void main()
{
	vec4 eyeCoord = gl_ModelViewMatrix * gl_Vertex;
	q = shadowTransMatrix * eyeCoord;
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}