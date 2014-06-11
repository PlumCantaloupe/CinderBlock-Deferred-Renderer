uniform mat4 modelview_mat;
uniform mat4 projection_mat;
uniform mat4 light_modelview_mat;
uniform mat4 camera_modelview_mat_inv;
uniform mat4 model_mat_inv;

varying vec4 pos_cs;                       //out
varying vec4 pos_ls;

void main()
{
    vec4 pos = gl_Vertex;                  //in

    pos_cs = modelview_mat * vec4( pos.xyz, 1.0 );
    pos_ls = light_modelview_mat * camera_modelview_mat_inv * model_mat_inv * pos_cs;
    
    gl_Position = projection_mat * pos_cs;
}