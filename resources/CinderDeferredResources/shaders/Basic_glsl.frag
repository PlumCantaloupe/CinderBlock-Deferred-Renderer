#version 120

uniform sampler2D tex;

varying vec2  uv;

void main(void)
{
    gl_FragColor = texture2D( tex, uv );
}