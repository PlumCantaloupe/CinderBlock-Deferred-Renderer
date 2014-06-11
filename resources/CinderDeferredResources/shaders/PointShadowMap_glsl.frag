#version 120
#extension GL_EXT_gpu_shader4 : require

uniform samplerCubeShadow shadow;
uniform vec3 light_pos;
uniform mat4 light_projection_mat;

varying vec4 pos_cs;
varying vec4 pos_ls;

void main()
{
    vec4 abs_pos = abs(pos_ls);
    float fs_z = -max(abs_pos.x, max(abs_pos.y, abs_pos.z));
    vec4 clip = light_projection_mat * vec4(0.0, 0.0, fs_z, 1.0);
    float depth = (clip.z / clip.w); // * 0.5 + 0.5;
    vec4 result = shadowCube(shadow, vec4(pos_ls.xyz, depth));
    
    float bias = 0.05;
    float dist = length( light_pos - pos_cs.xyz );
    float visibility  = ((result.z * 100.0 - dist * bias) > -bias) ? (0.0) : (1.0);
    vec4 diffuse = vec4(0.0, 0.0, 0.0, visibility * (1.0-result.z) * 0.3);
    gl_FragColor = diffuse;
}