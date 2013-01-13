#version 120

uniform float diff_coeff, phong_coeff, two_sided;

varying vec3 vPos, vNormal;
varying float vDepth;

void main(void)
{
	if(gl_Color.a < 0.1) discard; //remove fragments with low alpha
	vec3 normal = normalize(vNormal);
	gl_FragData[0] = gl_Color;
	gl_FragData[1] = vec4(normal, vDepth);
    gl_FragData[2] = vec4(vPos, 1.0);
	gl_FragData[3] = vec4(diff_coeff, phong_coeff, two_sided, 1.0);
}