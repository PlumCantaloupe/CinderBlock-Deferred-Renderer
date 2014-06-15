#version 120

/*
uniform float diff_coeff, phong_coeff, two_sided;
uniform float useTexture;
uniform sampler2D tex0;

varying vec2 uv;
varying vec3 vPos, vNormal;
varying float vDepth;

void main(void)
{
	if(gl_Color.a < 0.1) discard; //remove fragments with low alpha
	vec3 normal = normalize(vNormal);
    
    //get texture if one binded otherwise use color as defined by useTexture float (range: 0.0 - 1.0)
    vec4 texColor = texture2D(tex0, uv) * useTexture;
    vec4 vertColor = gl_Color * (1.0 - useTexture);
    
	gl_FragData[0] = texColor + vertColor;
    //    gl_FragData[0] = gl_Color;
	gl_FragData[1] = vec4(normal, vDepth);
    gl_FragData[2] = vec4(vPos, 1.0);
	gl_FragData[3] = vec4(diff_coeff, phong_coeff, two_sided, 1.0);
}
*/

uniform vec3 diffuse;
uniform vec3 specular;
uniform vec3 emissive;
uniform float shininess;
uniform float additiveSpecular;

uniform float useDiffuseTex;
uniform sampler2D texDiffuse;

varying vec2 uv;                                //in
varying vec3 vColor;                            //in
varying vec3 normalView;                        //in
varying vec4 clipPos;                           //in

const float unit = 255.0/256.0;
float vec3_to_float( vec3 data )
{
    float compressed = fract( data.x * unit ) + floor( data.y * unit * 255.0 ) + floor( data.z * unit * 255.0 ) * 255.0;
    return compressed;
}

void main()
{
    vec4 finalDiffuse;
    vec4 finalNormalDepth;
    
    const float opacity = 1.0; //no transparency
    finalDiffuse = (vec4( diffuse, opacity ) * (1.0 - useDiffuseTex)) + (texture2D(texDiffuse, uv) * useDiffuseTex);

    const float compressionScale = 0.999; //255.0/256.0;
    vec3 diffuseMapColor;
    diffuseMapColor = vec3( 1.0 );

    //diffuse color
    finalDiffuse.x = vec3_to_float( compressionScale * finalDiffuse.xyz );

    //specular color
    if ( additiveSpecular < 0.0 ) {
        finalDiffuse.y = vec3_to_float( compressionScale * specular );
    }
    else {
        finalDiffuse.y = vec3_to_float( compressionScale * specular * diffuseMapColor );
    }
    finalDiffuse.y *= additiveSpecular;
    finalDiffuse.z = shininess; //shininess
    finalDiffuse.w = vec3_to_float( compressionScale * emissive * diffuseMapColor * vColor ); //emissive color
    
    vec3 normal = normalize( normalView ) * 0.5 + 0.5;
    finalNormalDepth.xyz = normal;
    finalNormalDepth.w = clipPos.z / clipPos.w;
    
    gl_FragData[0] = finalDiffuse;
    gl_FragData[1] = finalNormalDepth;
}