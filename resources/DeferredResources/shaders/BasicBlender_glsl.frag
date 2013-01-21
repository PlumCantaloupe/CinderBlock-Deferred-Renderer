#version 120

//just blending 3 textures together (in a weird way)

uniform sampler2D ssoaTex;
uniform sampler2D shadowsTex;
uniform sampler2D baseTex;

varying vec2 uv;

void main()
{
	vec4 ssaoTex	= texture2D( ssoaTex, uv);
    vec4 shadowsTex	= texture2D( shadowsTex, uv);
	vec4 baseTex	= texture2D( baseTex, uv);
    
    //blendng by red value (from ssao)
	float redVal	= 1.0 - ssaoTex.r;
	vec4 resultTex	= vec4( baseTex.r - redVal, baseTex.g - redVal, baseTex.b - redVal, baseTex.a - redVal );
    
    //blending by alpha (from shadows)
    float redVal2	= shadowsTex.a;
	vec4 resultTex2	= vec4( resultTex.r - redVal2, resultTex.g - redVal2, resultTex.b - redVal2, resultTex.a - redVal2 );
	
	gl_FragColor = resultTex2;
}