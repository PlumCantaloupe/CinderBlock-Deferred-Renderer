What is this?
A Cinder Block deferred rendering meant for ready-to-go scenes with many dynamic lights and SSAO. There is also point-light shadow-mapping and FXAA. Works on both Xcode and VS2012, and includes both a simple and advanced template.

Drag this into your blocks folder within your Cinder directory (0.8.5, possibly 0.8.4), and use Tinderbox (under the toold directory) to create a projects with everything copied over appropriately.

Included "Advanced" Template
<img src="http://farm9.staticflickr.com/8363/8402994769_16588ca60f_c.jpg" />
<img src="http://farm9.staticflickr.com/8216/8406568340_3099ee04f3_c.jpg" />
<hr />
Included "Basic" Template
<img src="http://farm8.staticflickr.com/7451/12713642585_8704c8131a_c.jpg" />
<hr />
Projects using this engine
<img src="https://farm8.staticflickr.com/7534/16247456152_977dd31604_c.jpg" />
"Pedestal"

<img src="http://farm8.staticflickr.com/7312/12617617555_267da67848_c.jpg" />
"Space-man Marionette"

<hr />

Deferred rendering + shadow-mapping uses a huge amount if VRAM (one of its disadvantages); but its advantages can be a large amount of dynamic point lights possible.

Tips to get better framerates:
- adjust app window size
- lower FBO resolution
- turn off number shadow-mapped lights by setting last parameter of LIGHT_PS constructor to false
- reduce intensity/AOE of lights (adjust LIGHT_BRIGHTNESS_DEFAULT to be lower).
- Tested on Macbook Pro 2010 Mountain Lion ~40fps (release)
- Tested on HP620 Windows 7  ~ 170 fps (release)

THANKS: Also special thanks to the many Cube-mapping, deferred rendering, SSAO, and shadow-mapping bits of code out there. They were all very helpful in creating this project.
