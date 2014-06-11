#version 120

uniform sampler2DShadow		depthTexture;
varying vec4				q;

void main()
{
	vec3 coord  = 0.5 * (q.xyz / q.w + 1.0);
	vec4 result = shadow2D( depthTexture, coord );
    
    float bias = 0.05;
    float visibility = 0.0;
    if ( result.z  <  coord.z-bias ) {
        visibility = 0.3;
    }
    
	gl_FragColor = vec4( 0.0, 0.0, 0.0, visibility );
}