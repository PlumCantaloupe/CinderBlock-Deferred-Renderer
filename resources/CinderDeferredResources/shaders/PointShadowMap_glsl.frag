#version 120
#extension GL_EXT_gpu_shader4 : require

uniform samplerCubeShadow shadow;
uniform vec3 light_pos;
uniform mat4 light_modelview_mat;
uniform mat4 light_projection_mat;
uniform mat4 camera_modelview_mat_inv;
uniform mat4 model_mat_inv;

varying vec4 pos_cs;

void main()
{
    vec4 pos_ls = light_modelview_mat * camera_modelview_mat_inv * model_mat_inv * pos_cs;
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