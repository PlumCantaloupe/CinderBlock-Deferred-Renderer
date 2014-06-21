#version 120

uniform sampler2D tex;
uniform bool useTex;
uniform vec4 col;

varying vec2  uv;

void main(void)
{
    gl_FragColor = (texture2D( tex, uv ) * col * float(useTex)) + (col * (1-float(useTex)));
}