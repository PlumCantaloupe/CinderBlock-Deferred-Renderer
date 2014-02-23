#version 120

//just blending 3 textures together (in a weird way)

uniform sampler2D ssoaTex;
uniform sampler2D shadowsTex;
uniform sampler2D baseTex;
uniform bool useSSAO;
uniform bool useShadows;
uniform bool onlySSAO;

varying vec2 uv;

void main()
{
    vec2 uv2 = uv;
    uv2.t = 1-uv2.t;
    
    if(onlySSAO) {
        gl_FragColor = texture2D( ssoaTex, uv2);
    }
    else {
        vec4 ssaoTex	= texture2D( ssoaTex, uv2);
        vec4 shadowsTex	= texture2D( shadowsTex, uv);
        vec4 resultTex	= texture2D( baseTex, uv);
        
        if(useSSAO) {
            //blending by red value (from ssao)
            float redVal	= 1.0 - ssaoTex.r;
            resultTex	= vec4( resultTex.r - redVal, resultTex.g - redVal, resultTex.b - redVal, resultTex.a - redVal );
        }
        
        if (useShadows) {
            //blending by alpha (from shadows)
            float redVal2	= shadowsTex.a;
            resultTex	= vec4( resultTex.r - redVal2, resultTex.g - redVal2, resultTex.b - redVal2, resultTex.a - redVal2 );
        }
        
        gl_FragColor = resultTex;
    }
    
	
}