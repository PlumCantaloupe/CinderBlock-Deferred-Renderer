#version 120

uniform sampler2D samplerColor;
uniform sampler2D samplerNormalDepth;

uniform float lightRadius;
uniform float lightIntensity;
uniform int viewHeight;
uniform int viewWidth;

uniform vec3 lightColor;
uniform vec3 lightPositionVS;

uniform mat4 matProjInverse;

//THREE.DeferredShaderChunk[ "unpackFloat" ],
vec3 float_to_vec3( float data )
{
    vec3 uncompressed;
    uncompressed.x = fract( data );
    float zInt = floor( data / 255.0 );
    uncompressed.z = fract( zInt / 255.0 );
    uncompressed.y = fract( floor( data - ( zInt * 255.0 ) ) / 255.0 );
    return uncompressed;
}

//vec3 reconstruct_position(float depth, vec2 tex_coord)
//{
//    vec4 pos = vec4( (tex_coord.x-0.5)*2, (tex_coord.y-0.5)*2, 1, 1 );
//    vec4 ray = matProjInverse * pos;
//    return ray.xyz * depth;
//}

void main()
{
    //THREE.DeferredShaderChunk[ "computeVertexPositionVS" ],
    vec2 texCoord = gl_FragCoord.xy / vec2( viewWidth, viewHeight );
    vec4 normalDepth = texture2D( samplerNormalDepth, texCoord );
    float z = normalDepth.w;
    if ( z == 0.0 ) {
        discard;
    }
    vec2 xy = texCoord * 2.0 - 1.0;
    vec4 vertexPositionProjected = vec4( xy, z, 1.0 );
    vec4 vertexPositionVS = matProjInverse * vertexPositionProjected;
    vertexPositionVS.xyz /= vertexPositionVS.w;
    vertexPositionVS.w = 1.0;

//    vec4 vertexPositionVS = vec4(reconstruct_position(normalDepth.a, texCoord), 1.0);
    
    //bail out early when pixel outside of light sphere
    vec3 lightVector = lightPositionVS - vertexPositionVS.xyz;
    float dist = length( lightVector );

    if ( dist > lightRadius ) {
        discard;
    }

    //THREE.DeferredShaderChunk[ "computeNormal" ],
    vec3 normal = normalDepth.xyz * 2.0 - 1.0;
    
    //THREE.DeferredShaderChunk[ "unpackColorMap" ],
    vec4 colorMap = texture2D( samplerColor, texCoord );
    vec3 albedo = float_to_vec3( abs( colorMap.x ) );
    vec3 specularColor = float_to_vec3( abs( colorMap.y ) );
    float shininess = abs( colorMap.z );
    float wrapAround = sign( colorMap.z );
    float additiveSpecular = sign( colorMap.y );

    //compute light
    lightVector = normalize( lightVector );

    //THREE.DeferredShaderChunk[ "computeDiffuse" ],
    float dotProduct = dot( normal, lightVector );
    float diffuseFull = max( dotProduct, 0.0 );
    vec3 diffuse;
//    if ( wrapAround < 0.0 ) {
        //wrap around lighting
        float diffuseHalf = max( 0.5 * dotProduct + 0.5, 0.0 );
        const vec3 wrapRGB = vec3( 1.0, 1.0, 1.0 );
        diffuse = mix( vec3( diffuseFull ), vec3( diffuseHalf ), wrapRGB );
//    }
//    else {
//        //simple lighting
//        diffuse = vec3( diffuseFull );
//    }
    
    float cost = dot(normal.xyz, lightVector);
	cost = (cost < 0.0)?1.0*cost:cost; //calculate two sided lighting.
    diffuse *= cost;
    
    //THREE.DeferredShaderChunk[ "computeSpecular" ],
    vec3 halfVector = normalize( lightVector - normalize( vertexPositionVS.xyz ) );
    float dotNormalHalf = max( dot( normal, halfVector ), 0.0 );
    
    //simple specular (not used)
    //vec3 specular = specularColor * max( pow( dotNormalHalf, shininess ), 0.0 ) * diffuse;
    
    // physically based specular
    float specularNormalization = ( shininess + 2.0001 ) / 8.0;
    vec3 schlick = specularColor + vec3( 1.0 - specularColor ) * pow( 1.0 - dot( lightVector, halfVector ), 5.0 );
    vec3 specular = schlick * max( pow( dotNormalHalf, shininess ), 0.0 ) * diffuse * specularNormalization;
    
    //combine
    float cutoff = 0.3;
    float denom = dist / lightRadius + 1.0;
    float attenuation = 1.0 / ( denom * denom );
    attenuation = ( attenuation - cutoff ) / ( 1.0 - cutoff );
    attenuation = max( attenuation, 0.0 );
    attenuation *= attenuation;

    //vec3 C = 40.0 * lightColor*(albedo * diffuse + specular)/(dist*dist+0.8); //calculate light contribution with attenuation
    //gl_FragColor = vec4(C, 1.0);
   
    //THREE.DeferredShaderChunk[ "combine" ],
    vec3 light = lightIntensity * lightColor;
    gl_FragColor = vec4( light * ( albedo * diffuse + specular ), attenuation );
    //gl_FragColor = vec4( light, 1.0 );
    //gl_FragColor = vec4(1,0,0,0);
}