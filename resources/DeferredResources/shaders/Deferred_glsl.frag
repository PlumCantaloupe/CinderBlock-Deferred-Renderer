#version 120

uniform float diff_coeff, phong_coeff, two_sided;

varying vec3 vPos, vNormal;
varying float vDepth;

vec4 pack (float depth)
{
    const vec4 bias = vec4(1.0 / 255.0,
                           1.0 / 255.0,
                           1.0 / 255.0,
                           0.0);
    
    float r = depth;
    float g = fract(r * 255.0);
    float b = fract(g * 255.0);
    float a = fract(b * 255.0);
    vec4 colour = vec4(r, g, b, a);
    
    return colour - (colour.yzww * bias);
}

void main(void)
{
	if(gl_Color.a < 0.1) discard; //remove fragments with low alpha
	vec3 normal = normalize(vNormal);
    
	gl_FragData[0] = gl_Color;
	gl_FragData[1] = vec4(normal, pack(vDepth));
    gl_FragData[2] = vec4(vPos, 1.0);
	gl_FragData[3] = vec4(diff_coeff, phong_coeff, two_sided, 1.0);
}