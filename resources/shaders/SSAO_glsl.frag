#version 120
//original SSAO shader graciously written at: http://www.gamedev.net/topic/556187-the-best-ssao-ive-seen/
//haven't got this working yet here ... TODO to see the difference

//uniform sampler2D gnormals;
uniform sampler2D gposition;
uniform sampler2D gdepthAndNormals;
uniform sampler2D gdiffuse;
uniform sampler2D grandom;

varying vec2 uv;

vec3 readNormal(vec2 coord)
{
    return normalize(texture2D(gdepthAndNormals, coord).xyz*2.0  - 1.0);
}

vec3 posFromDepth(vec2 coord)
{
    float d = texture2D(gdepthAndNormals, coord).a;
    vec3 tray = mat3x3(gl_ProjectionMatrixInverse)*vec3((coord.x-0.5)*2.0,(coord.y-0.5)*2.0,1.0);
    return tray*d;
//    return texture2D(gposition, coord).rgb;
}

//Ambient Occlusion form factor:
float aoFF(vec3 ddiff, vec3 cnorm, float c1, float c2)
{
    vec3 vv = normalize(ddiff);
    float rd = length(ddiff);
    return (1.0-clamp(dot(readNormal(uv+vec2(c1,c2)),-vv),0.0,1.0)) * clamp(dot( cnorm,vv ),0.0,1.0) * (1.0 - 1.0/sqrt(1.0/(rd*rd) + 1.0));
}

//GI form factor:
float giFF(vec3 ddiff,vec3 cnorm, float c1, float c2)
{
    vec3 vv = normalize(ddiff);
    float rd = length(ddiff);
    return 1.0*clamp(dot(readNormal(uv+vec2(c1,c2)),-vv),0.0,1.0) * clamp(dot( cnorm,vv ),0.0,1.0) / (rd*rd+1.0);
}

void main(void)
{
    //read current normal,position and color.
    vec3 n = readNormal(uv);
    vec3 p = posFromDepth(uv);
    vec3 col = texture2D(gdiffuse, uv).rgb;
    
    //randomization texture
    vec2 fres = vec2(800.0/128.0*5,600.0/128.0*5);
    vec3 random = texture2D(grandom, uv*fres.xy).rgb;
    random = random*2.0-vec3(1.0);
    
    //initialize variables:
    float ao = 0.0;
    vec3 gi = vec3(0.0,0.0,0.0);
    float incx = 1.0/800.0*0.1;
    float incy = 1.0/600.0*0.1;
    float pw = incx;
    float ph = incy;
    float cdepth = texture2D(gdepthAndNormals, uv).a;
    
    //3 rounds of 8 samples each.
    for(float i=0.0; i<3.0; ++i)
    {
        float npw = (pw+0.0007*random.x)/cdepth;
        float nph = (ph+0.0007*random.y)/cdepth;
        
        vec3 ddiff = posFromDepth(uv+vec2(npw,nph))-p;
        vec3 ddiff2 = posFromDepth(uv+vec2(npw,-nph))-p;
        vec3 ddiff3 = posFromDepth(uv+vec2(-npw,nph))-p;
        vec3 ddiff4 = posFromDepth(uv+vec2(-npw,-nph))-p;
        vec3 ddiff5 = posFromDepth(uv+vec2(0,nph))-p;
        vec3 ddiff6 = posFromDepth(uv+vec2(0,-nph))-p;
        vec3 ddiff7 = posFromDepth(uv+vec2(npw,0))-p;
        vec3 ddiff8 = posFromDepth(uv+vec2(-npw,0))-p;
        
        ao+=  aoFF(ddiff,n,npw,nph);
        ao+=  aoFF(ddiff2,n,npw,-nph);
        ao+=  aoFF(ddiff3,n,-npw,nph);
        ao+=  aoFF(ddiff4,n,-npw,-nph);
        ao+=  aoFF(ddiff5,n,0,nph);
        ao+=  aoFF(ddiff6,n,0,-nph);
        ao+=  aoFF(ddiff7,n,npw,0);
        ao+=  aoFF(ddiff8,n,-npw,0);
        
        gi+=  giFF(ddiff,n,npw,nph)*texture2D(gdiffuse, uv+vec2(npw,nph)).rgb;
        gi+=  giFF(ddiff2,n,npw,-nph)*texture2D(gdiffuse, uv+vec2(npw,-nph)).rgb;
        gi+=  giFF(ddiff3,n,-npw,nph)*texture2D(gdiffuse, uv+vec2(-npw,nph)).rgb;
        gi+=  giFF(ddiff4,n,-npw,-nph)*texture2D(gdiffuse, uv+vec2(-npw,-nph)).rgb;
        gi+=  giFF(ddiff5,n,0,nph)*texture2D(gdiffuse, uv+vec2(0,nph)).rgb;
        gi+=  giFF(ddiff6,n,0,-nph)*texture2D(gdiffuse, uv+vec2(0,-nph)).rgb;
        gi+=  giFF(ddiff7,n,npw,0)*texture2D(gdiffuse, uv+vec2(npw,0)).rgb;
        gi+=  giFF(ddiff8,n,-npw,0)*texture2D(gdiffuse, uv+vec2(-npw,0)).rgb;
        
        //increase sampling area:
        pw += incx;
        ph += incy;
    }
    ao/=24.0;
    gi/=24.0;
    
    
    gl_FragColor = vec4(col-vec3(ao)+gi*5.0,1.0);
    //gl_FragColor = vec4(1.0,1.0,1.0,1.0);
}