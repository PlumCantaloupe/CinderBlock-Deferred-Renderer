#version 120

uniform sampler2D sampler_col;

varying vec2  uv;

void main(void)
{
    gl_FragColor = texture2D( sampler_col, uv );
}