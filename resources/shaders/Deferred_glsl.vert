#version 120

varying vec3 vPos, vNormal;
varying float vDepth; //in eye space

void main(void)
{
	gl_Position = ftransform();
	gl_FrontColor = gl_Color;
    
    vec4 tmp = gl_ModelViewMatrix * gl_Vertex;
	vPos = tmp.xyz/tmp.w;
    
	vNormal = gl_NormalMatrix * gl_Normal;
    vDepth = -vPos.z/10.0;
}

