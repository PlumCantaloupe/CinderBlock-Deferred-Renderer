/*
#version 120
#extension GL_EXT_gpu_shader4 : require

varying vec4 position_cs;

void main()
{
    gl_Position = ftransform();
	position_cs = gl_ModelViewMatrix * gl_Vertex;
}
 */

/*
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
 */


uniform mat4 modelview_mat;
uniform mat4 projection_mat;

varying vec4 pos_cs;                       //out

void main()
{
    vec4 pos = gl_Vertex;                  //in

    pos_cs = modelview_mat * vec4( pos.xyz, 1.0 );
    gl_Position = projection_mat * pos_cs;
}