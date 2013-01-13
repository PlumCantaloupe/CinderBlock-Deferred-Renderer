//would like to better use deferred rendered for this ... WIP

#version 120
#extension GL_EXT_gpu_shader4 : require

varying vec4 position_cs;
varying vec3 normal_cs;
varying vec3 color;

uniform samplerCubeShadow shadow;
//
//uniform sampler2D positionMap;
//uniform sampler2D normalMap;
//uniform sampler2D colorMap;
//

uniform mat4x4 camera_view_matrix_inv;
uniform mat4x4 light_view_matrix;
uniform mat4x4 light_projection_matrix;
uniform vec3 light_position;

//
varying vec2  uv;
//

void main()
{
	vec4 position_ls = light_view_matrix * camera_view_matrix_inv * position_cs;
    //
//    vec4 position_cs = texture2D(positionMap, uv);
//    vec4 position_ls = light_view_matrix * camera_view_matrix_inv * position_cs;
//    vec3 normal_cs = texture2D(normalMap, uv).rgb;
//    vec3 color = texture2D(colorMap, uv).rgb;
    //
    
	vec4 abs_position = abs(position_ls);
	float fs_z = -max(abs_position.x, max(abs_position.y, abs_position.z));
	vec4 clip = light_projection_matrix * vec4(0.0, 0.0, fs_z, 1.0);
	float depth = (clip.z / clip.w) * 0.5 + 0.5;
	vec4 result = shadowCube(shadow, vec4(position_ls.xyz, depth));
    
	vec3 lvector = light_position - position_cs.xyz;
	float ldistance = length(lvector);
	float lintensity = max(dot(normal_cs, normalize(lvector)), 0.0) * 10.0;
	lintensity /= ldistance * ldistance;
	lintensity /= lintensity + 0.5;
    
//	vec3 diffuse = lintensity * result.xyz * color;
    vec4 diffuse = vec4(0.0, 0.0, 0.0, (1.0-result.r) * 0.2);
    
	gl_FragColor = diffuse; //vec4(diffuse,1);
}