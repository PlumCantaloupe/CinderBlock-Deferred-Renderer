#version 120

uniform sampler2DShadow		depthTexture;
varying vec4				q;

void main()
{
	vec3 coord  = 0.5 * (q.xyz / q.w + 1.0);
	float shadow = shadow2D( depthTexture, coord ).r;
	//gl_FragColor = vec4( shadow, shadow, shadow, 0.5 );
    gl_FragColor = vec4( 0.0, 0.0, 0.0, 0.0 );
}