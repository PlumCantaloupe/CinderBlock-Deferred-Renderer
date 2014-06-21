#version 120

uniform vec3 light_pos_vs;
uniform vec3 light_dir_vs;

uniform sampler2D sampler_col;
uniform sampler2D sampler_col_whole;
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
    vec3 albedo = texture2D( sampler_col_whole, texCoord ).xyz; //float_to_vec3( abs( colorMap.x ) );
    vec3 specularColor = float_to_vec3( abs( colorMap.y ) );
    float shininess = abs( colorMap.z );
    //float wrapAround = sign( colorMap.z );
    float additiveSpecular = sign( colorMap.y );

    // compute light
    vec3 lightVector = normalize( light_pos_vs.xyz - vertexPositionVS.xyz );

    //OLD
    float rho = dot( light_dir_vs, lightVector );
    float rhoMax = cos( light_angle * 0.5 );
    
    // Compute vector from surface to light position
    //vec3 direction1 = vec3(light_dir_vs);
    //vec3 vertexPos = vec3(vertexPositionVS);
//    vec3 s = normalize ( light_pos_vs - vertexPositionVS.xyz); //normalize IT???????????????????
//    vec3 v = normalize( vec3(-vertexPositionVS.xyz) );
//    vec3 h = normalize( v + s ); //eyeDirection;
//    vec3 spotDir = normalize(light_dir_vs.xyz);
//    float rho = acos( dot(-s, spotDir) );
//    float rhoMax = radians( clamp( light_angle, 0.0, 90.0 ) );

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
    vec3 light = light_intensity * light_col;
    gl_FragColor = vec4( light * ( albedo * diffuse + specular ), attenuation );
    //gl_FragColor = vec4(light, 1);
    //gl_FragColor = vec4(0,0,0,0);
}

/*
#version 120

uniform sampler2D sampler_col;
uniform sampler2D sampler_normal_depth;

uniform vec3 light_pos_vs;
uniform vec3 light_dir_vs;  	// Direction of the spotlight in eye coords.
uniform float light_angle;    	// light_angle angle (between 0 and 90)
uniform float view_height;
uniform float view_width;
uniform mat4 proj_inv_mat; //!!added

vec2 texCoord = gl_FragCoord.xy / vec2(view_width, view_height);
float exponent = 45.0 * 10.0 / 2.0;
float constantAttenuation = 20.0;
float linearAttenuation = 0.0;
float quadraticAttenuation = 0.05;

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;

vec3 float_to_vec3( float data )
{
    vec3 uncompressed;
    uncompressed.x = fract( data );
    float zInt = floor( data / 255.0 );
    uncompressed.z = fract( zInt / 255.0 );
    uncompressed.y = fract( floor( data - ( zInt * 255.0 ) ) / 255.0 );
    return uncompressed;
}

vec4 spotLight( vec3 pos, vec3 norm, vec4 diff, vec4 spec)
{
    float attenuation;
    vec4 diffuse=vec4(0);
    vec4 ambient=vec4(0);
    vec4 specular=vec4(0);
    {
        vec3 direction1 = vec3(light_dir_vs);
        
        // Compute vector from surface to light position
        vec3 s = normalize( light_pos_vs - pos); //normalize IT???????????????????
        vec3 v = normalize(vec3(-pos));
        vec3 h = normalize( v + s ); //eyeDirection;
        vec3 spotDir = normalize(direction1);
        float angle = acos( dot(-s, spotDir) );
        float light_angle = radians( clamp( light_angle, 0.0, 90.0 ) );
        
        
        // Compute attenuation
        float distance = length ( light_pos_vs - pos);
        float atten = 1.f / (constantAttenuation +
                             linearAttenuation * distance +
                             quadraticAttenuation * distance * distance);
        
        ambient += diff * ambient * atten;
        
        if( angle < light_angle )
        {
            float spotFactor = pow( dot(-s, spotDir), exponent );
            vec3 v = normalize(vec3(-pos));
            vec3 h = normalize( v + s ); //eyeDirection;
            
            //float sDotN = max(dot(s, norm), 0.0); // surface Dot normal
            //float hDotN = max(dot(h, norm), 0.0); // halfVector Dot Normal
            
            
            //color and NormalMap
            float sDotN = dot(s, norm); // surface Dot normal
            //float sDotT = dot(s, tangent);
            //float sDotB = dot(s, binormal);
            
            //vec3 final_color_factor = vec3(sDotT, sDotB, sDotN);
            //float ColorFactor= max( dot( final_color_factor, normalMap), 0.0);
            
            //specularity only
            float hDotN =  dot(h, norm); // halfVector Dot Normal
            //float hDotT =  dot(h, tangent);
            //float hDotB =  dot(h, binormal);
            
            //vec3 final_spec_factor = vec3(hDotT, hDotB, hDotN);
            vec3 final_spec_factor = vec3(0.0, 0.0, hDotN);
            //float SpecFactor = pow ( max( dot( final_spec_factor, normalMap), 0.0), 50.0);
            
			diffuse+=diff*diffuse * sDotN * spotFactor *atten;
			ambient+=diff*ambient *atten;
            
			if (hDotN > 0.0)
			{
				specular += spec*specular * pow( max(hDotN, 0.0), 50.0) * attenuation;//material.shininess);
			}
            
            return ambient + diffuse + specular;
        }
        else
        {
            return vec4(1,1,0,1); //ambient;
        }
    }
}


void LightPass()
{
	vec4 colorMap = texture2D( sampler_col, texCoord );
    vec3 albedo = float_to_vec3( abs( colorMap.x ) );
    vec3 specularColor = float_to_vec3( abs( colorMap.y ) );
    float shininess = abs( colorMap.z );
    float wrapAround = sign( colorMap.z );
    float additiveSpecular = sign( colorMap.y );
	
    vec4 normalDepth = texture2D( sampler_normal_depth, texCoord );
    float z = normalDepth.w;
    if ( z == 0.0 ) {
        discard;
    }
    
    vec2 xy = texCoord * 2.0 - 1.0;
    vec4 vertexPositionProjected = vec4( xy, z, 1.0 );
    vec4 pos = proj_inv_mat * vertexPositionProjected;
    pos.xyz /= pos.w;
    pos.w = 1.0;
    
	// Retrieve position and normal information from textures
    //vec3 pos = vec3( texture( PositionTex, texCoord ) );
	//vec3 norm = normalize(vec3(texture(NormalTex, texCoord)));
	//vec3 normalMap = normalize(vec3(texture(NormalMapTex, texCoord)));
	//vec3 tangent = normalize(vec3( texture( TangentTex, texCoord)));
	//vec3 binormal = normalize(vec3( texture( BiNormalTex, texCoord)));
    //vec4 diffColor = texture(sampler_col, texCoord);
	//vec4 specColor = texture(SpecularTex, texCoord);
	
	gl_FragColor = spotLight(pos.xyz, normalDepth.xyz, vec4(albedo, 1.0), vec4(specularColor, 1.0));
}

void main()
{
	LightPass();
}


*/