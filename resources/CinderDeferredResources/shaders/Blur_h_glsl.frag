uniform sampler2D RTScene; // the texture with the scene you want to blur
uniform float blurStep;

varying vec2 uv;
  
void main(void)
{
   vec4 sum = vec4(0.0);
 
   // blur in y (vertical)
   // take nine samples, with the distance blurStep between them
   sum += texture2D(RTScene, vec2(uv.x - 4.0*blurStep, uv.y)) * 0.05;
   sum += texture2D(RTScene, vec2(uv.x - 3.0*blurStep, uv.y)) * 0.09;
   sum += texture2D(RTScene, vec2(uv.x - 2.0*blurStep, uv.y)) * 0.12;
   sum += texture2D(RTScene, vec2(uv.x - blurStep, uv.y)) * 0.15;
   sum += texture2D(RTScene, vec2(uv.x, uv.y)) * 0.16;
   sum += texture2D(RTScene, vec2(uv.x + blurStep, uv.y)) * 0.15;
   sum += texture2D(RTScene, vec2(uv.x + 2.0*blurStep, uv.y)) * 0.12;
   sum += texture2D(RTScene, vec2(uv.x + 3.0*blurStep, uv.y)) * 0.09;
   sum += texture2D(RTScene, vec2(uv.x + 4.0*blurStep, uv.y)) * 0.05;
 
   gl_FragColor = sum;
}