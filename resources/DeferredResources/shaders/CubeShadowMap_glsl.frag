#version 120
#extension GL_EXT_gpu_shader4 : require

varying vec4 position_cs;
varying vec3 normal_cs;
varying vec3 color;

uniform samplerCubeShadow shadow;
uniform mat4x4 camera_view_matrix_inv;
uniform mat4x4 light_view_matrix;
uniform mat4x4 light_projection_matrix;
uniform vec3 light_position;

void main()
{
	vec4 position_ls = light_view_matrix * camera_view_matrix_inv * position_cs;
    
	vec4 abs_position = abs(position_ls);
	float fs_z = -max(abs_position.x, max(abs_position.y, abs_position.z));
	vec4 clip = light_projection_matrix * vec4(0.0, 0.0, fs_z, 1.0);
	float depth = (clip.z / clip.w) * 0.5 + 0.5;
	vec4 result = shadowCube(shadow, vec4(position_ls.xyz, depth));
    
	float bias = 0.005;
    float dist = length( light_position - position_cs.xyz );
    float visibility  = ((result.z * 100.0 - dist * bias) > -bias) ? (0.0) : (1.0);
    
	vec4 diffuse = vec4(0.0, 0.0, 0.0, visibility * (1.0-result.z) * 0.3);
    
	gl_FragColor = diffuse;
}