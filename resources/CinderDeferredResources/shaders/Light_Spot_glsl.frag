#version 120

/*
//input parameters
uniform sampler2D positionMap, normalMap, colorMap, attrMap;
uniform vec3 camPos, lightPos, lightCol;
uniform float dist;
varying vec4 sPos;

void main(void)
{
	vec2 coord = sPos.xy/sPos.w*0.5+0.5;
	vec4 pos = texture2D(positionMap, coord); //get the position from deferred shading

	vec3 VP = lightPos-pos.xyz; //vector between light and point
	float d = length(VP); //get the distance between the light and point
	if(d > dist) discard; //if outside of area of effect, discard pixel
	VP /= d; //normalize vector between light and point (divide by distance)
	vec4 norm = texture2D(normalMap, coord); //get the normal from deferred shading
	vec4 col = texture2D(colorMap, coord); //get the color from deferred shading
	vec4 attr = texture2D(attrMap, coord); //get lighting attributes from deferred shading
	float diff_coeff = attr.r;
	float phong_coeff = attr.g;
	float two_sided = attr.b;
	float cost = dot(norm.xyz, VP);
	cost = (cost < 0.0)?-two_sided*cost:cost; //calculate two sided lighting.
	float diff = diff_coeff*cost; //calculate diffuse shading
	vec3 H = normalize(VP+normalize(camPos - pos.xyz)); //calculate half vector
	float phong = phong_coeff*pow(max(dot(H, norm.xyz), 0.0), 100.0); //calculate phong shading
	vec3 C = lightCol*(col.rgb*diff+phong)/(d*d+0.8); //calculate light contribution with attenuation
	//all lights have constant quadratic attenuation of 1.0, with a constant attenuation of 0.8 to avoid dividing by small numbers
	
    gl_FragColor = vec4(C, 1.0); //output color
}
*/

uniform vec3 lightPositionVS;
uniform vec3 lightDirectionVS;

uniform sampler2D samplerColor;
uniform sampler2D samplerNormalDepth;

uniform float viewHeight;
uniform float viewWidth;

uniform float lightAngle;
uniform float lightIntensity;
uniform vec3 lightColor;

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
    
    //THREE.DeferredShaderChunk[ "computeNormal" ],
    vec3 normal = normalDepth.xyz * 2.0 - 1.0;
    
    //THREE.DeferredShaderChunk[ "unpackColorMap" ],
    vec4 colorMap = texture2D( samplerColor, texCoord );
    vec3 albedo = float_to_vec3( abs( colorMap.x ) );
    vec3 specularColor = float_to_vec3( abs( colorMap.y ) );
    float shininess = abs( colorMap.z );
    float wrapAround = sign( colorMap.z );
    float additiveSpecular = sign( colorMap.y );

    // compute light
    vec3 lightVector = normalize( lightPositionVS.xyz - vertexPositionVS.xyz );

    float rho = dot( lightDirectionVS, lightVector );
    float rhoMax = cos( lightAngle * 0.5 );

    if ( rho <= rhoMax ) {
        discard;
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
    vec3 light = lightIntensity * lightColor;
    gl_FragColor = vec4( light * ( albedo * diffuse + specular ), attenuation );
}