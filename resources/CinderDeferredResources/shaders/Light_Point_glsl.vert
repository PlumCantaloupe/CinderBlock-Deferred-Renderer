#version 120

uniform mat4 modelview_mat;
uniform mat4 projection_mat;

void main()
{
    vec4 position = gl_Vertex;                  //in
    
    vec4 mvPosition = modelview_mat * vec4( position.xyz, 1.0 );
    gl_Position = projection_mat * mvPosition;
}