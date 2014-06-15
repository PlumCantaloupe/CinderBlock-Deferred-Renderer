#version 120

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

varying vec2 uv;                                //out
varying vec3 vColor;                            //out
varying vec3 normalView;                        //out
varying vec4 clipPos;                           //out

void main() {
    vec4 position = gl_Vertex;                  //in
    vec3 normal = gl_Normal;                    //in
    vec4 color = gl_Color;                      //in
    gl_FrontColor = gl_Color;
    uv = gl_MultiTexCoord0.st;
    
    vColor = color.rgb;
    
    vec4 mvPosition = modelViewMatrix * vec4( position.xyz, 1.0 );
    gl_Position = projectionMatrix * mvPosition;
    
    vec3 objectNormal = normal;
    vec3 transformedNormal = normalMatrix * objectNormal;
    normalView = normalize( normalMatrix * objectNormal );
    clipPos = gl_Position;
}
