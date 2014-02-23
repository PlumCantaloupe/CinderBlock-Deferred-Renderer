//What is this?
//A Cinder app that utilizes a deferred rendering engine to render lights and SSAO. There is also point-light shadow-mapping (heavy GPU cost though very nice)
//anthony.scavarelli@gmail.com

#ifndef CinderDeferredRendering_CinderDeferredRenderingApp_h
#define CinderDeferredRendering_CinderDeferredRenderingApp_h

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "cinder/Text.h"
#include "cinder/Font.h"

#include "DeferredRenderer.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const float	APP_RES_HORIZONTAL = 1024.0f;
static const float	APP_RES_VERTICAL = 768.0f;
static const Vec3f	CAM_POSITION_INIT( -14.0f, 7.0f, -14.0f );
static const Vec3f	LIGHT_POSITION_INIT( 3.0f, 1.5f, 0.0f );
static const int    NUM_LIGHTS = 500;        //number of lights


class CinderDeferredRenderingApp : public AppBasic 
{
public:
    CinderDeferredRenderingApp();
    virtual ~CinderDeferredRenderingApp();
    void prepareSettings( Settings *settings );
    
    void setup();
    void update();
    void draw();
    
    void keyDown( app::KeyEvent event );
    
    void drawShadowCasters(gl::GlslProg* deferShader) const;
    void drawNonShadowCasters(gl::GlslProg* deferShader) const;
    void drawOverlay() const;
    void drawDepthParticles() const;
    
protected:
    int RENDER_MODE;
	
    //debug
    cinder::params::InterfaceGl mParams;
    bool				mShowParams;
    float				mCurrFramerate;
    gl::Texture         mEarthTex;
	
    //camera
    CameraPersp         mCam;
    DeferredRenderer    mDeferredRenderer;
    
    int mCurrLightIndex;
};


CinderDeferredRenderingApp::CinderDeferredRenderingApp(){}
CinderDeferredRenderingApp::~CinderDeferredRenderingApp(){}

#pragma mark - lifecycle functions

void CinderDeferredRenderingApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( APP_RES_HORIZONTAL, APP_RES_VERTICAL );
    settings->setBorderless( true );
	settings->setFrameRate( 1000.0f );			//the more the merrier!
	settings->setResizable( false );			//this isn't going to be resizable
    settings->setFullScreen( false );
    
	//make sure secondary screen isn't blacked out as well when in fullscreen mode ( do wish it could accept keyboard focus though :(
	//settings->enableSecondaryDisplayBlanking( false );
}

void CinderDeferredRenderingApp::setup()
{
    //!!test texture for diffuse texture
    mEarthTex = gl::Texture( loadImage( loadResource( RES_TEX_EARTH ) ) );
    
	gl::disableVerticalSync(); //so I can get a true representation of FPS (if higher than 60 anyhow :/)
    
	RENDER_MODE = DeferredRenderer::SHOW_FINAL_VIEW;
    
	mParams = params::InterfaceGl( "3D_Scene_Base", Vec2i( 225, 125 ) );
	mParams.addParam( "Framerate", &mCurrFramerate, "", true );
    mParams.addParam( "Selected Light Index", &mCurrLightIndex);
	mParams.addParam( "Show/Hide Params", &mShowParams, "key=x");
	mParams.addSeparator();
    
	mCurrFramerate = 0.0f;
	mShowParams = true;
	
	//set up camera
	mCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 10000 );
    mCam.lookAt(CAM_POSITION_INIT * 1.5f, Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
    mCam.setCenterOfInterestPoint(Vec3f::zero());

    //create functions pointers to send to deferred renderer
    std::function<void(gl::GlslProg*)> fRenderShadowCastersFunc;
    std::function<void(gl::GlslProg*)> fRenderNotShadowCastersFunc;
    std::function<void()> fRenderOverlayFunc;
    std::function<void()> fRenderParticlesFunc;
    fRenderShadowCastersFunc = std::bind(&CinderDeferredRenderingApp::drawShadowCasters, this, std::placeholders::_1 );
    fRenderNotShadowCastersFunc = std::bind(&CinderDeferredRenderingApp::drawNonShadowCasters, this,  std::placeholders::_1 );
    fRenderOverlayFunc = std::bind(&CinderDeferredRenderingApp::drawOverlay, this ); //drawOverlay function pointer
    fRenderParticlesFunc = std::bind(&CinderDeferredRenderingApp::drawDepthParticles, this ); //an overlay that also uses depth from rendered screen for occulsions (as opposed to overlay func which has no occlusions enabled)
    
    //NULL value represents the opportunity to a function pointer to an "overlay" method. Basically only basic textures can be used and it is overlayed onto the final scene.
    //see example of such a function (from another project) commented out at the bottom of this class ...
    
    mDeferredRenderer.setup( fRenderShadowCastersFunc, fRenderNotShadowCastersFunc, NULL, NULL, &mCam, Vec2i(APP_RES_HORIZONTAL, APP_RES_VERTICAL), 1024, true, true ); //no overlay or "particles"
    //mDeferredRenderer.setup( fRenderShadowCastersFunc, fRenderNotShadowCastersFunc, fRenderOverlayFunc, NULL, &mMayaCam, Vec2i(APP_RES_HORIZONTAL, APP_RES_VERTICAL), 1024, true, true ); //overlay enabled
    //mDeferredRenderer.setup( fRenderShadowCastersFunc, fRenderNotShadowCastersFunc, fRenderOverlayFunc, fRenderParticlesFunc, &mMayaCam, Vec2i(APP_RES_HORIZONTAL, APP_RES_VERTICAL), 1024, true, true ); //overlay and "particles" enabled -- not working yet
    
    //have these cast point light shadows
    mDeferredRenderer.addCubeLight(    Vec3f(-2.0f, 4.0f, 6.0f),      Color(0.10f, 0.69f, 0.93f) * LIGHT_BRIGHTNESS_DEFAULT, true);      //blue
    mDeferredRenderer.addCubeLight(    Vec3f(4.0f, 6.0f, -4.0f),      Color(0.94f, 0.15f, 0.23f) * LIGHT_BRIGHTNESS_DEFAULT, true);      //red
    
    //add a bunch of lights
    for(int i = 0; i < NUM_LIGHTS; i++) {
        
        int randColIndex = Rand::randInt(5);
        Color randCol;
        switch( randColIndex ) {
            case 0:
                randCol = Color(0.99f, 0.67f, 0.23f); //orange
                break;
            case 1:
                randCol = Color(0.97f, 0.24f, 0.85f); //pink
                break;
            case 2:
                randCol = Color(0.00f, 0.93f, 0.30f); //green
                break;
            case 3:
                randCol = Color(0.98f, 0.96f, 0.32f); //yellow
                break;
            case 4:
                randCol = Color(0.10f, 0.69f, 0.93f); //blue
                break;
            case 5:
                randCol = Color(0.94f, 0.15f, 0.23f); //red
                break;
        };
        
        mDeferredRenderer.addCubeLight( Vec3f(Rand::randFloat(-1000.0f, 1000.0f),Rand::randFloat(0.0f, 50.0f),Rand::randFloat(-1000.0f, 1000.0f)),
                                        randCol * LIGHT_BRIGHTNESS_DEFAULT,
                                        false,
                                        true);
    }
    
    mCurrLightIndex = 0;
}

void CinderDeferredRenderingApp::update()
{
	mCurrFramerate = getAverageFps();
}

void CinderDeferredRenderingApp::draw()
{
    mDeferredRenderer.renderFullScreenQuad(RENDER_MODE);
    
	if (mShowParams) {
		mParams.draw();
    }
}

void CinderDeferredRenderingApp::keyDown( KeyEvent event ) 
{
    float lightMovInc = 0.1f;
    
	switch ( event.getCode() ) 
	{
        //switch between render views
		case KeyEvent::KEY_0:
        {RENDER_MODE = DeferredRenderer::SHOW_FINAL_VIEW;}
			break;
		case KeyEvent::KEY_1:
        {RENDER_MODE = DeferredRenderer::SHOW_DIFFUSE_VIEW;}
			break;
		case KeyEvent::KEY_2:
        {RENDER_MODE = DeferredRenderer::SHOW_NORMALMAP_VIEW;}
			break;
		case KeyEvent::KEY_3:
        {RENDER_MODE = DeferredRenderer::SHOW_DEPTH_VIEW;}
			break;
        case KeyEvent::KEY_4:
        {RENDER_MODE = DeferredRenderer::SHOW_POSITION_VIEW;}
			break;
        case KeyEvent::KEY_5:
        {RENDER_MODE = DeferredRenderer::SHOW_ATTRIBUTE_VIEW;}
			break;
        case KeyEvent::KEY_6:
        {RENDER_MODE = DeferredRenderer::SHOW_SSAO_VIEW;}
			break;
        case KeyEvent::KEY_7:
        {RENDER_MODE = DeferredRenderer::SHOW_SSAO_BLURRED_VIEW;}
			break;
        case KeyEvent::KEY_8:
        {RENDER_MODE = DeferredRenderer::SHOW_LIGHT_VIEW;}
            break;
        case KeyEvent::KEY_9:
        {RENDER_MODE = DeferredRenderer::SHOW_SHADOWS_VIEW;}
			break;
            
        //change which cube you want to control
        case 269: {
            //minus key
            if( mDeferredRenderer.getNumCubeLights() > 0) {
                --mCurrLightIndex;
                if ( mCurrLightIndex < 0) mCurrLightIndex = mDeferredRenderer.getNumCubeLights() - 1;
            }
        }
            break;
        case 61: {
            if( mDeferredRenderer.getNumCubeLights() > 0) {
                //plus key
                ++mCurrLightIndex;
                if ( mCurrLightIndex > mDeferredRenderer.getNumCubeLights() - 1) mCurrLightIndex = 0;
            }
        }
            break;
			
        //move selected cube light
		case KeyEvent::KEY_UP: {
            if ( mDeferredRenderer.getNumCubeLights() > 0) {
                if(event.isShiftDown()) {
                    mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->setPos( mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->getPos() + Vec3f(0.0f, lightMovInc, 0.0f ));
                }
                else {
                    mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->setPos( mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->getPos() + Vec3f(0.0f, 0.0f, lightMovInc));
                }
            }
		}
			break;
		case KeyEvent::KEY_DOWN: {
            if ( mDeferredRenderer.getNumCubeLights() > 0) {
                if(event.isShiftDown()) {
                    mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->setPos( mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->getPos() + Vec3f(0.0f, -lightMovInc, 0.0f ));
                }
                else {
                    mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->setPos( mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->getPos() + Vec3f(0.0f, 0.0, -lightMovInc));
                }
            }
		}
			break;
		case KeyEvent::KEY_LEFT: {
            if ( mDeferredRenderer.getNumCubeLights() > 0) {
                mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->setPos( mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->getPos() + Vec3f(lightMovInc, 0.0, 0.0f));
            }
		}
			break;
		case KeyEvent::KEY_RIGHT: {
            if ( mDeferredRenderer.getNumCubeLights() > 0) {
                mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->setPos( mDeferredRenderer.getCubeLightsRef()->at(mCurrLightIndex)->getPos() + Vec3f(-lightMovInc, 0.0, 0.0f));
            }
		}
            break;
        case KeyEvent::KEY_ESCAPE: {
            //never know when you need to quit quick
            exit(1);
        }
			break;
		default:
			break;
	}
}

#pragma mark - render functions

void CinderDeferredRenderingApp::drawShadowCasters(gl::GlslProg* deferShader) const
{
	//just some test objects
    if(deferShader != NULL) {
        deferShader->uniform("useTexture", 1.0f);
        deferShader->uniform("tex0", 0);
        mEarthTex.bind(0);
    }
    
    glColor3ub(255,255,255);
    gl::drawSphere( Vec3f(-1.0, 0.0,-1.0), 1.0f, 30 );
    
    if(deferShader != NULL) {
        deferShader->uniform("useTexture", 0.0f);
        mEarthTex.unbind(0);
    }
    
	glColor3ub(0,255,0);
    gl::drawCube( Vec3f( 1.0f, 0.0f, 1.0f ), Vec3f( 2.0f, 2.0f, 2.0f ) );
    
    glColor3ub(255,0,255);
    gl::drawCube( Vec3f( 0.0f, 0.0f, 4.5f ), Vec3f( 1.0f, 2.0f, 1.0f ) );
    
    glColor3ub(255,255,0);
    gl::drawCube( Vec3f( 3.0f, 0.0f, -1.5f ), Vec3f( 1.0f, 3.0f, 1.0f ) );
    
	glColor3ub(255,0,255);
    gl::pushMatrices();
	glTranslatef(-2.0f, -0.7f, 2.0f);
	glRotated(90.0f, 1, 0, 0);
    gl::drawTorus( 1.0f, 0.3f, 32, 64 );
	gl::popMatrices();
    
}

void CinderDeferredRenderingApp::drawNonShadowCasters(gl::GlslProg* deferShader) const
{
    int size = 3000;
    //a plane to capture shadows (though it won't cast any itself)
    glColor3ub(255, 255, 255);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3i( size, -1,-size);
    glVertex3i(-size, -1,-size);
    glVertex3i(-size, -1, size);
    glVertex3i( size, -1, size);
    glEnd();
}

void CinderDeferredRenderingApp::drawOverlay() const
{
    Vec3f camUp, camRight;
    mCam.getBillboardVectors(&camRight, &camUp);
    
    //create text labels
    TextLayout layout1;
	layout1.clear( ColorA( 1.0f, 1.0f, 1.0f, 0.0f ) );
	layout1.setFont( Font( "Arial", 34 ) );
	layout1.setColor( ColorA( 255.0f/255.0f, 255.0f/255.0f, 8.0f/255.0f, 1.0f ) );
	layout1.addLine( to_string(getAverageFps()) ); //to_string is a c++11 function for conversions
	Surface8u rendered1 = layout1.render( true, false );
    gl::Texture fontTexture_FR = gl::Texture( rendered1 );
    
    //draw framerate
    fontTexture_FR.bind();
    gl::drawBillboard(Vec3f(-3.0f, 7.0f, 20.0f), Vec2f(fontTexture_FR.getWidth()/20.0f , fontTexture_FR.getHeight()/20.0f), 0, camRight, camUp);
    fontTexture_FR.unbind();
}

void CinderDeferredRenderingApp::drawDepthParticles() const
{
    gl::enableAdditiveBlending();
    
    //this where typically a particle engine would go. For now lets just draw some "earths"
    glColor4ub(255, 255, 255, 160);
    mEarthTex.bind();
    gl::drawCube(Vec3f(3.0f, 2.0f, 8.0f), Vec3f(3.0f, 3.0f, 3.0f));
    gl::drawCube(Vec3f(1.0f, 5.0f, -3.0f), Vec3f(3.0f, 3.0f, 3.0f));
    gl::drawCube(Vec3f(-3.0f, 3.0f, 4.0f), Vec3f(3.0f, 3.0f, 3.0f));
    gl::drawCube(Vec3f(-2.0f, 4.0f, 7.0f), Vec3f(3.0f, 3.0f, 3.0f));
    mEarthTex.unbind();
    glColor4ub(255, 255, 255, 255);
}

CINDER_APP_BASIC( CinderDeferredRenderingApp, RendererGl )

#endif
