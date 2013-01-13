uniform sampler2D RTBlurH; // this should hold the texture rendered by the horizontal blur pass
varying vec2 vTexCoord;
 
const float blurSize = 0.5/512.0;
 
void main(void)
{
   vec4 sum = vec4(0.0);
 
   // blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y - 4.0*blurSize)) * 0.05;
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y - 3.0*blurSize)) * 0.09;
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y - 2.0*blurSize)) * 0.12;
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y - blurSize)) * 0.15;
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y)) * 0.16;
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y + blurSize)) * 0.15;
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y + 2.0*blurSize)) * 0.12;
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y + 3.0*blurSize)) * 0.09;
   sum += texture2D(RTBlurH, vec2(vTexCoord.x, vTexCoord.y + 4.0*blurSize)) * 0.05;
 
   gl_FragColor = sum;
}