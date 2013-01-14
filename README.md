What is this?
A Cinder application that utilizes a deferred rendering engine to render lights and SSAO. There is also point-light shadow-mapping (heavy GPU cost though)

TODO: lots of optimization to do here yet.

!!!!! If your computer is slow turn off the shadow-mapping by setting last parameter of Light_PS to false (not true)
Main reason this will be slow will be GPU pipes and RAM as deferred rendering + shadow-mapping uses a huge amount if VRAM (one of its disadvantages)
Deferred Rendering ADVANTAGE: tons of dynamic point lights (w/o shadows) possible, if not at GPU limits already anyhow

Inspiration and shader base from http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/image-space-lighting-r2644 and http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter09.html

Tips to get better framerates:
 - adjust ap window size
 - go to initFBO's and lower resolution of maps
 - turn off number shadow-mapped lights by setting last parameter of LIGHT_PS constructor to false
 - Tested on Macbook Pro 2010 Mountain Lion ~18-10fps
 - Tested on HP620 Windows 7 ~60-70 fps

Controls
keys 0-9 toggle through deferred layers (depth, colour, normal, shadows etc.)
key 0 shows final composed scene
Using MayaCam so use alt and mouse buttons to move camera
can move a selected light with arrows keys (and shift and arrow keys for up and down movement)
switch between "selected" lights by using - and + (will update index number in params with a delay)

please let me know if you make any optimizations, or have any optimization suggestions. Always more to learn; and more fps is always appreciated :)
