#version 120

varying vec4 clip_pos;                           //in

void main()
{
    float finalDepth = clip_pos.z / clip_pos.w;
    gl_FragDepth = finalDepth;
}