#version 120

/*
uniform mat4		shadowTransMatrix;
varying vec4		q;

void main()
{
	vec4 eyeCoord = gl_ModelViewMatrix * gl_Vertex;
	q = shadowTransMatrix * eyeCoord;
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
 
 Matrix44f shadowTransMatrix = (*currSpotLight)->mShadowCam.getProjectionMatrix();
 shadowTransMatrix *= (*currSpotLight)->mShadowCam.getModelViewMatrix();
 shadowTransMatrix *= mCam->getInverseModelViewMatrix();
 */

uniform mat4 modelview_mat;
uniform mat4 model_mat_inv;
uniform mat4 projection_mat;
uniform mat4 shadowTransMatrix;

varying vec4 q;                            //out

void main()
{
    vec4 pos = gl_Vertex;                  //in
    
    vec4 pos_cs = modelview_mat * vec4( pos.xyz, 1.0 );
    q = (shadowTransMatrix ) * pos_cs;
    
    gl_Position = projection_mat * pos_cs;
}