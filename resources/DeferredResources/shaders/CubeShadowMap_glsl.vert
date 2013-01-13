#version 120
#extension GL_EXT_gpu_shader4 : require

varying vec4 position_cs;
varying vec3 normal_cs;
varying vec3 color;

varying vec2  uv;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	position_cs = gl_ModelViewMatrix * gl_Vertex;
	normal_cs = gl_NormalMatrix * gl_Normal;
	color = gl_Color.rgb;
    
//    gl_Position = ftransform();
//	gl_Position = sign( gl_Position );
//    
//    //Texture coordinate for screen aligned (in correct range):
//	uv = (vec2( gl_Position.x, gl_Position.y ) + vec2( 1.0 ) ) * 0.5;
}