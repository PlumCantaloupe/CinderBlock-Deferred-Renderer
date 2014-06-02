#version 120

uniform vec3 light_pos_vs;
uniform vec3 light_dir_vs;

uniform sampler2D sampler_col;
uniform sampler2D sampler_normal_depth;

uniform float view_height;
uniform float view_width;

uniform float light_angle;
uniform float light_intensity;
uniform vec3 light_col;

uniform mat4 proj_inv_mat;

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

void main()
{
    //THREE.DeferredShaderChunk[ "computeVertexPositionVS" ],
    vec2 texCoord = gl_FragCoord.xy / vec2( view_width, view_height );
    vec4 normalDepth = texture2D( sampler_normal_depth, texCoord );
    float z = normalDepth.w;
    if ( z == 0.0 ) {
        discard;
    }
    vec2 xy = texCoord * 2.0 - 1.0;
    vec4 vertexPositionProjected = vec4( xy, z, 1.0 );
    vec4 vertexPositionVS = proj_inv_mat * vertexPositionProjected;
    vertexPositionVS.xyz /= vertexPositionVS.w;
    vertexPositionVS.w = 1.0;
    
    //THREE.DeferredShaderChunk[ "computeNormal" ],
    vec3 normal = normalDepth.xyz * 2.0 - 1.0;
    
    //THREE.DeferredShaderChunk[ "unpackColorMap" ],
    vec4 colorMap = texture2D( sampler_col, texCoord );
    vec3 albedo = float_to_vec3( abs( colorMap.x ) );
    vec3 specularColor = float_to_vec3( abs( colorMap.y ) );
    float shininess = abs( colorMap.z );
    float wrapAround = sign( colorMap.z );
    float additiveSpecular = sign( colorMap.y );

    // compute light
    vec3 lightVector = normalize( light_pos_vs.xyz - vertexPositionVS.xyz );

    float rho = dot( light_dir_vs, lightVector );
    float rhoMax = cos( light_angle * 0.5 );

    if ( rho <= rhoMax ) {
        //discard;
    }

    float theta = rhoMax + 0.0001;
    float phi = rhoMax + 0.05;
    float falloff = 4.0;

    float spot = 0.0;

    if ( rho >= phi ) {
        spot = 1.0;
    }
    else if ( rho <= theta ) {
        spot = 0.0;
    }
    else {
        spot = pow( ( rho - theta ) / ( phi - theta ), falloff );
    }

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

    diffuse *= spot;

    //THREE.DeferredShaderChunk[ "computeSpecular" ],
    vec3 halfVector = normalize( lightVector - normalize( vertexPositionVS.xyz ) );
    float dotNormalHalf = max( dot( normal, halfVector ), 0.0 );
    
    //simple specular (not used)
    //vec3 specular = specularColor * max( pow( dotNormalHalf, shininess ), 0.0 ) * diffuse;
    
    // physically based specular
    float specularNormalization = ( shininess + 2.0001 ) / 8.0;
    vec3 schlick = specularColor + vec3( 1.0 - specularColor ) * pow( 1.0 - dot( lightVector, halfVector ), 5.0 );
    vec3 specular = schlick * max( pow( dotNormalHalf, shininess ), 0.0 ) * diffuse * specularNormalization;

    // combine
    const float attenuation = 1.0;

    //THREE.DeferredShaderChunk[ "combine" ],
    vec3 light = light_intensity * light_col;
    //gl_FragColor = vec4( light * ( albedo * diffuse + specular ), attenuation );
    gl_FragColor = vec4(1,1,0,1);
}