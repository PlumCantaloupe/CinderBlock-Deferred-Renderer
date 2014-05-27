#version 120

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