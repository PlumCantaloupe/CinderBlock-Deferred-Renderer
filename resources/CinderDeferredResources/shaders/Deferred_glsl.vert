#version 120

/*
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
 */

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

varying vec3 vColor;                            //out
varying vec3 normalView;                        //out
varying vec4 clipPos;                           //out

void main() {
    //vec2 uv = gl_MultiTexCoord0.st;             //in
    vec4 position = gl_Vertex;                  //in
    vec3 normal = gl_Normal;                    //in
    vec4 color = gl_Color;                      //in
    gl_FrontColor = gl_Color;
    
    vColor = color.rgb;
    
    vec4 mvPosition = modelViewMatrix * vec4( position.xyz, 1.0 );
    gl_Position = projectionMatrix * mvPosition;
    
    vec3 objectNormal = normal;
    vec3 transformedNormal = normalMatrix * objectNormal;
    normalView = normalize( normalMatrix * objectNormal );
    clipPos = gl_Position;
}
