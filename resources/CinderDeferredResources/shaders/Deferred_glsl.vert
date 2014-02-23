#version 120

varying vec2 uv;
varying vec3 vPos, vNormal;
varying float vDepth; //in eye space

void main(void)
{
    gl_Position = ftransform();
    uv = gl_MultiTexCoord0.st;
    
//	gl_Position = ftransform();
    gl_FrontColor = gl_Color;
//    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    vec4 tmp = gl_ModelViewMatrix * gl_Vertex;
	vPos = tmp.xyz/tmp.w;
	vNormal = gl_NormalMatrix * gl_Normal;
    vDepth = -vPos.z * 0.1;
}
