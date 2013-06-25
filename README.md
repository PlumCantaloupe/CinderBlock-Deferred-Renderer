What is this?
A Cinder application that utilizes a deferred rendering engine to render lights and SSAO. There is also point-light shadow-mapping (heavy GPU cost though)

Can hit about 500 on 2010 MBP and stay around 30fps. Adjust NUM_LIGHTS static var for performance in sample.

<img src="http://farm9.staticflickr.com/8363/8402994769_16588ca60f_c.jpg" />
<img src="http://farm9.staticflickr.com/8087/8406554876_8af255d2e0_c.jpg" />
<img src="http://farm9.staticflickr.com/8216/8406568340_3099ee04f3_c.jpg" />

TODO: <br />
- Better quality shadow-mapping <br />

Deferred rendering + shadow-mapping uses a huge amount if VRAM (one of its disadvantages); but its advantages can be a large amount of dynamic point lights possible.

Inspiration and shader base from http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/image-space-lighting-r2644 and http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter09.html

Tips to get better framerates:
 - adjust app window size
 - lower FBO resolution
 - turn off number shadow-mapped lights by setting last parameter of LIGHT_PS constructor to false
 - reduce intensity/AOE of lights (adjust LIGHT_BRIGHTNESS_DEFAULT to be lower).

 - Tested on Macbook Pro 2010 Mountain Lion ~30fps (release)
 - Tested on HP620 Windows 7  ~ 170 fps (release)

Controls
keys 0-9 toggle through deferred layers (depth, colour, normal, shadows etc.)
key 0 shows final composed scene
Using MayaCam so use alt and mouse buttons to move camera
can move a selected light with arrows keys (and shift and arrow keys for up and down movement)
switch between "selected" lights by using - and + (will update index number in params with a delay)

please let me know if you make any optimizations, or have any optimization suggestions. Always more to learn; and more fps is always appreciated :)

THANKS: Also special thanks to the many Cube-mapping, deferred rendering, SSAO, and shadow-mapping bits of code out there. They were all very helpful in creating this project.
