#version 120

uniform sampler2D RTBlurH; // this should hold the texture rendered by the horizontal blur pass
uniform float blurStep;

varying vec2 uv;
  
void main(void)
{
   vec4 sum = vec4(0.0);
 
   // blur in y (vertical)
   // take nine samples, with the distance blurStep between them
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y - 4.0*blurStep)) * 0.05;
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y - 3.0*blurStep)) * 0.09;
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y - 2.0*blurStep)) * 0.12;
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y - blurStep)) * 0.15;
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y)) * 0.16;
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y + blurStep)) * 0.15;
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y + 2.0*blurStep)) * 0.12;
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y + 3.0*blurStep)) * 0.09;
   sum += texture2D(RTBlurH, vec2(uv.x, uv.y + 4.0*blurStep)) * 0.05;
 
   gl_FragColor = sum;
}