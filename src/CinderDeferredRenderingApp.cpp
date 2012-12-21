//What is this?
//A Cinder app that utilizes a deferred rendering engine to render lights and SSAO. There is also point-light shadow-mapping (heavy GPU cost though)

//TODO: lots of optimization to do here yet.
//!!!!! If your computer is slow turn off the shadow-mapping by setting last parameter of Light_PS to false (not true)
//Main reason this will be slow will be GPU pipes and RAM as deferred rendering + shadow-mapping uses a huge amount if VRAM (one of its disadvantages)
//Deferred Rendering ADVANTAGE: tons of dynamic point lights (w/o shadows) possible, if not at GPU limits already anyhow

//Inspiration and shader base from http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/image-space-lighting-r2644 
//and http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter09.html

//Tips to get better framerates:
// - adjust ap window size
// - go to initFBO's and lower resolution of maps
// - turn off number shadow-mapped lights by setting last parameter of LIGHT_PS constructor to false
// - Tested on Macbook Pro 2010 Mountain Lion ~10fps
// - Tested on HP620 Windows 7 ~60-70 fps

//Controls
//keys 1-8 toggle through deferred layers (depth, colour, normal, shadows etc.)
//key 9 shows final composed scene
//Using MayaCam so use alt and mouse buttons to move camera
//can move a selected light with arrows keys (and shift and arrow keys for up and down movement)
//switch between "selected" lights by using - and + (will update index number in params with a delay)

//please let me know if you make any optimizations, or have any optimization suggestions. Always more to learn; and more fps is always appreciated :)

#ifndef CinderDeferredRendering_CinderDeferredRenderingApp_h
#define CinderDeferredRendering_CinderDeferredRenderingApp_h

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/DisplayList.h"
#include "cinder/gl/Material.h"
#include "cinder/ImageIo.h"
#include "cinder/Arcball.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Rand.h"
#include "cinder/params/Params.h"

#include "CubeShadowMap.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const float	APP_RES_HORIZONTAL = 1280;
static const float	APP_RES_VERTICAL = 720;
static const float	FBO_RESOLUTION = 1024;
static const Vec3f	CAM_POSITION_INIT( -14.0f, 12.0f, -14.0f );
static const Vec3f	LIGHT_POSITION_INIT( 3.0f, 1.5f, 0.0f );
static const float  LIGHT_CUTOFF = 0.01;   //light intensity cutoff point
static const int    NUM_LIGHTS = 6;        //number of lights
static const float  LIGHT_BRIGTNESS = 40;  //brightness of lights
static const int	SHADOW_MAP_RESOLUTION = 512;

//render modes (switch using keys 1-9)
enum
{
	SHOW_DIFFUSE_VIEW,
    SHOW_NORMALMAP_VIEW,
    SHOW_DEPTH_VIEW,
    SHOW_POSITION_VIEW,
    SHOW_ATTRIBUTE_VIEW,
	SHOW_SSAO_VIEW,
    SHOW_LIGHT_VIEW,
    SHOW_SHADOWS_VIEW,
	SHOW_FINAL_VIEW
};

//Light Cube Class
class Light_PS
{
    public:
    float           dist;
	Vec3f           pos, col;
    CameraPersp     mShadowCam;
    CubeShadowMap   mShadowMap;
    gl::Fbo			mCubeDepthFbo;
    gl::Fbo			mShadowsFbo;
    
    private:
    bool            mCastShadows;
    
    public:
	Light_PS(Vec3f p, Vec3f c, bool castsShadows = false) : pos(p), col(c)
    {
        dist = sqrt(col.length()/LIGHT_CUTOFF);
        
        //set up fake "light"
        mShadowCam.setPerspective( 90.0f, 1.0f, 1.0f, 40.0f );
        mShadowCam.lookAt( p, Vec3f( p.x, 0.0f, p.z ) );
        
        //set up cube map for point shadows
        mShadowMap.setup( SHADOW_MAP_RESOLUTION );
        
        mCastShadows = castsShadows;
        if (mCastShadows)
        {
            setUpShadowStuff();
        }
    }
    
    void setUpShadowStuff()
    {
        //create FBO to hold depth values from cube map
        gl::Fbo::Format formatShadow;
        formatShadow.enableColorBuffer(true, 1);
        formatShadow.enableDepthBuffer(true, true);
        formatShadow.setMinFilter(GL_LINEAR);
        formatShadow.setMagFilter(GL_LINEAR);
        formatShadow.setWrap(GL_CLAMP, GL_CLAMP);
        mCubeDepthFbo   = gl::Fbo( SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, formatShadow);
        
        gl::Fbo::Format format;
        //format.setDepthInternalFormat( GL_DEPTH_COMPONENT32 );
        format.setColorInternalFormat( GL_RGBA16F_ARB );
        //format.setSamples( 4 ); // enable 4x antialiasing
        mShadowsFbo	= gl::Fbo( FBO_RESOLUTION, FBO_RESOLUTION, format );
    }
    
	void setPos(Vec3f p)
    {
        mShadowCam.lookAt( p, Vec3f( p.x, 0.0f, p.z ) );
        pos = p;
    }
    
	void setCol(Vec3f c)
    {
        col = c;
        dist = sqrt(col.length()/LIGHT_CUTOFF);
    }
    
	void renderCube() const
    {
        gl::drawCube(pos, Vec3f(2.0, 2.0, 2.0));
    }
    
    void renderCubeDist() const
    {
        gl::drawCube(pos, Vec3f(dist, dist, dist));
    }
    
    inline const bool doesCastShadows(){ return mCastShadows; }
    inline const Vec3f& getPos() const {return pos;}
    inline const Vec3f& getCol() const {return col;}
	inline float getDist() const {return dist;}
};


class CinderDeferredRenderingApp : public AppBasic 
{
public:
    CinderDeferredRenderingApp();
    virtual ~CinderDeferredRenderingApp();
    void prepareSettings( Settings *settings );
    
    void setup();
    void update();
    void draw();
    
    void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
    void keyDown( app::KeyEvent event );
	
    //render functions ( when looking to optimize GPU look here ... )
    void createShadowMaps();
    void renderShadowsToFBOs();
    
    void drawShadowCasters();
    void drawPlane();
    void drawScene();
    void drawLightMeshes(gl::GlslProg* shader = NULL);
    void renderLights();
	void renderDeferredFBO();
    void renderSSAOToFBO();
    void renderLightsToFBO();
    void pingPongBlur();	
    void renderScreenSpace();
    
    void initShaders();
    void initFBOs();
    
protected:
	
    int RENDER_MODE;
	
    //debug
    cinder::params::InterfaceGl mParams;
    bool				mShowParams;
    float				mCurrFramerate;
	
    //camera
    MayaCamUI           mMayaCam;
    
    Matrix44f           mLightFaceViewMatrices[6];
	
    gl::Texture			mRandomNoise;
    
    gl::Fbo				mDeferredFBO;
    gl::Fbo				mSSAOMap;
    gl::Fbo				mPingPongBlurH;
    gl::Fbo				mPingPongBlurV;
    gl::Fbo				mLightGlowFBO;
    gl::Fbo				mAllShadowsFBO;
    
	gl::GlslProg		mCubeShadowShader;
	
    gl::GlslProg		mSSAOShader;
    gl::GlslProg		mDeferredShader;
    gl::GlslProg		mBasicBlender;
    gl::GlslProg		mHBlurShader;
    gl::GlslProg		mVBlurShader;
    gl::GlslProg		mLightShader;
    gl::GlslProg		mAplhaToRBG;
    
    vector<Light_PS*>   mCubeLights;
    
    int mCurrLightIndex;
};

/* 
 * @Description: constructor
 * @param: none
 * @return: none
 */
CinderDeferredRenderingApp::CinderDeferredRenderingApp()
{}

/* 
 * @Description: deconstructor ( called when program exits )
 * @param: none
 * @return: none
 */
CinderDeferredRenderingApp::~CinderDeferredRenderingApp()
{}

/* 
 * @Description: set basic app settings here
 * @param: Settings ( pointer to base app settings )
 * @return: none
 */
void CinderDeferredRenderingApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( APP_RES_HORIZONTAL, APP_RES_VERTICAL );
	settings->setFrameRate( 1000.0f );			//the more the merrier!
	settings->setResizable( false );			//this isn't going to be resizable
    settings->setFullScreen( false );
	
	//make sure secondary screen isn't blacked out as well when in fullscreen mode ( do wish it could accept keyboard focus though :(
	//settings->enableSecondaryDisplayBlanking( false );
}

/* 
 * @Description: setup function ( more aesthetic as could be in constructor here )
 * @param: none
 * @return: none
 */
void CinderDeferredRenderingApp::setup()
{
	gl::disableVerticalSync();

	RENDER_MODE = SHOW_FINAL_VIEW;
	
	//glEnable( GL_LIGHTING );
	//glEnable( GL_DEPTH_TEST );
	//glEnable(GL_RESCALE_NORMAL); //important if things are being scaled as OpenGL also scales normals ( for proper lighting they need to be normalized )
	
    glClearDepth(1.0f);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_LIGHTING);
	glClearColor(0, 0, 0, 0);
	glColor4d(1, 1, 1, 1);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable( GL_TEXTURE_2D );
    
	mParams = params::InterfaceGl( "3D_Scene_Base", Vec2i( 225, 125 ) );
	mParams.addParam( "Framerate", &mCurrFramerate, "", true );
    mParams.addParam( "Selected Light Index", &mCurrLightIndex);
	mParams.addParam( "Show/Hide Params", &mShowParams, "key=x");
	mParams.addSeparator();
    
	mCurrFramerate = 0.0f;
	mShowParams = true;
	
	//set up camera
    CameraPersp initialCam;
	initialCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 10000 );
    initialCam.lookAt(CAM_POSITION_INIT, Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
	mMayaCam.setCurrentCam( initialCam );
	
    //axial matrices required for six-sides of calculations for cube shadows
	CameraPersp cubeCam;
    cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(1.0, 0.0, 0.0),  Vec3f(0.0,-1.0, 0.0));
    mLightFaceViewMatrices[ CubeShadowMap::X_FACE_POS ] = cubeCam.getModelViewMatrix();
    cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0), Vec3f(-1.0, 0.0, 0.0),  Vec3f(0.0,-1.0, 0.0));
    mLightFaceViewMatrices[ CubeShadowMap::X_FACE_NEG ] = cubeCam.getModelViewMatrix();
    cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(0.0, 1.0, 0.0),  Vec3f(0.0, 0.0, 1.0));
    mLightFaceViewMatrices[ CubeShadowMap::Y_FACE_POS ] = cubeCam.getModelViewMatrix();
    cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(0.0,-1.0, 0.0),  Vec3f(0.0, 0.0,-1.0) );
    mLightFaceViewMatrices[ CubeShadowMap::Y_FACE_NEG ] = cubeCam.getModelViewMatrix();
    cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(0.0, 0.0, 1.0),  Vec3f(0.0,-1.0, 0.0) );
    mLightFaceViewMatrices[ CubeShadowMap::Z_FACE_POS ] = cubeCam.getModelViewMatrix();
    cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(0.0, 0.0,-1.0),  Vec3f(0.0,-1.0, 0.0) );
    mLightFaceViewMatrices[ CubeShadowMap::Z_FACE_NEG ] = cubeCam.getModelViewMatrix();
    
    //noise texture required for SSAO calculations
	mRandomNoise = gl::Texture( loadImage( loadResource( NOISE_SAMPLER ) ) );
	
    //have these cast point light shadows
    mCubeLights.push_back( new Light_PS(    Vec3f(-2.0f, 4.0f, 6.0f),      Vec3f(0.10f, 0.69f, 0.93f) * LIGHT_BRIGTNESS, true) );      //blue
    mCubeLights.push_back( new Light_PS(    Vec3f(4.0f, 6.0f, -4.0f),      Vec3f(0.94f, 0.15f, 0.23f) * LIGHT_BRIGTNESS, true) );      //red
    
    //add lights to scene (without shadows)
    mCubeLights.push_back( new Light_PS(    Vec3f(-10.0f, 8.0f, 12.0f),    Vec3f(0.99f, 0.67f, 0.23f) * LIGHT_BRIGTNESS) );           //orange
    mCubeLights.push_back( new Light_PS(    Vec3f(6.0f, 10.0f, -10.0f),    Vec3f(0.97f, 0.24f, 0.85f) * LIGHT_BRIGTNESS) );           //pink
    mCubeLights.push_back( new Light_PS(    Vec3f(-8.0f, 12.0f, -8.0f),    Vec3f(0.00f, 0.93f, 0.30f) * LIGHT_BRIGTNESS) );           //green
    mCubeLights.push_back( new Light_PS(    Vec3f(12.0f, 8.0f, 14.0f),     Vec3f(0.98f, 0.96f, 0.32f) * LIGHT_BRIGTNESS) );            //yellow
    

	//just adding more lights for fun (could potentiallay add dozens more with minimal performance hit)
	int multiplier = 4.0f;
	mCubeLights.push_back( new Light_PS(    Vec3f(-10.0f * multiplier, 8.0f, 12.0f * multiplier),   Vec3f(0.99f, 0.67f, 0.23f) * LIGHT_BRIGTNESS) );           //orange
    mCubeLights.push_back( new Light_PS(    Vec3f(6.0f * multiplier, 10.0f, -10.0f * multiplier),   Vec3f(0.97f, 0.24f, 0.85f) * LIGHT_BRIGTNESS) );           //pink
    mCubeLights.push_back( new Light_PS(    Vec3f(-8.0f, 12.0f, -8.0f * multiplier),				Vec3f(0.00f, 0.93f, 0.30f) * LIGHT_BRIGTNESS) );           //green
    mCubeLights.push_back( new Light_PS(    Vec3f(12.0f * multiplier, 8.0f, 14.0f),					Vec3f(0.98f, 0.96f, 0.32f) * LIGHT_BRIGTNESS) );            //yellow
	mCubeLights.push_back( new Light_PS(    Vec3f(-10.0f * multiplier, 8.0f, 12.0f),				Vec3f(0.99f, 0.67f, 0.23f) * LIGHT_BRIGTNESS) );           //orange
    mCubeLights.push_back( new Light_PS(    Vec3f(6.0f, 10.0f, -10.0f * multiplier),				Vec3f(0.97f, 0.24f, 0.85f) * LIGHT_BRIGTNESS) );           //pink
    mCubeLights.push_back( new Light_PS(    Vec3f(-8.0f * multiplier, 12.0f, -8.0f * multiplier),   Vec3f(0.00f, 0.93f, 0.30f) * LIGHT_BRIGTNESS) );           //green
    mCubeLights.push_back( new Light_PS(    Vec3f(12.0f, 8.0f, 14.0f * multiplier),					Vec3f(0.98f, 0.96f, 0.32f) * LIGHT_BRIGTNESS) );            //yellow
	mCubeLights.push_back( new Light_PS(    Vec3f(-10.0f * multiplier, 8.0f, 12.0f * multiplier),   Vec3f(0.99f, 0.67f, 0.23f) * LIGHT_BRIGTNESS) );           //orange
    mCubeLights.push_back( new Light_PS(    Vec3f(6.0f * multiplier, 10.0f, -10.0f),				Vec3f(0.97f, 0.24f, 0.85f) * LIGHT_BRIGTNESS) );           //pink
    mCubeLights.push_back( new Light_PS(    Vec3f(-8.0f * multiplier, 12.0f, -8.0f * multiplier),   Vec3f(0.00f, 0.93f, 0.30f) * LIGHT_BRIGTNESS) );           //green
    mCubeLights.push_back( new Light_PS(    Vec3f(12.0f, 8.0f, 14.0f * multiplier),					Vec3f(0.98f, 0.96f, 0.32f) * LIGHT_BRIGTNESS) );            //yellow

    mCurrLightIndex = 0;
    
	initFBOs();
	initShaders();
}

/* 
 * @Description: update
 * @param: none
 * @return: none
 */
void CinderDeferredRenderingApp::update()
{
	mCurrFramerate = getAverageFps();
}

/* 
 * @Description: draw
 * @param: none
 * @return: none
 */
void CinderDeferredRenderingApp::draw()
{
    //clear depth and color every frame
	glClearColor( 0.5f, 0.5f, 0.5f, 1 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    gl::setMatrices( mMayaCam.getCamera() );
    
	renderDeferredFBO();
    
    createShadowMaps();
    renderShadowsToFBOs();
    
    gl::setMatrices( mMayaCam.getCamera() );
    
    renderLightsToFBO();
	renderSSAOToFBO();
    
	renderScreenSpace(); //render full screen quad image of scene
    
	if (mShowParams)
		params::InterfaceGl::draw();
}

void CinderDeferredRenderingApp::createShadowMaps()
{
	//render depth map cube
    glEnable(GL_CULL_FACE);
    for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube)
    {
        if ( !(*currCube)->doesCastShadows() )
            continue;
        
        (*currCube)->mCubeDepthFbo.bindFramebuffer();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
        
        glCullFace(GL_FRONT);
        
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf((*currCube)->mShadowCam.getProjectionMatrix());
        glMatrixMode(GL_MODELVIEW);
        
        for (size_t i = 0; i < 6; ++i)
        {
            (*currCube)->mShadowMap.bindDepthFB( i );
            glClear(GL_DEPTH_BUFFER_BIT);
            
            glLoadMatrixf(mLightFaceViewMatrices[i]);
            glMultMatrixf((*currCube)->mShadowCam.getModelViewMatrix());
            
            drawShadowCasters();
        }
        
        (*currCube)->mCubeDepthFbo.unbindFramebuffer();
    }
    glDisable(GL_CULL_FACE);
}

void CinderDeferredRenderingApp::renderShadowsToFBOs()
{
    glEnable(GL_CULL_FACE);
    for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube)
    {
        if ( !(*currCube)->doesCastShadows() )
            continue;
        
        (*currCube)->mShadowsFbo.bindFramebuffer();
        
        glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);
//        glViewport(0, 0, (*currCube)->mShadowsFbo.getWidth(), (*currCube)->mShadowsFbo.getHeight()); //change viewport back to original size after changing to fit shadowmap
        gl::setViewport( (*currCube)->mShadowsFbo.getBounds() );
        
        
        glCullFace(GL_BACK); //don't need what we won't see
        
        gl::setMatrices( mMayaCam.getCamera() );
        
        //gl::setMatricesWindow( (float)(*currCube)->mShadowsFbo.getWidth(), (float)(*currCube)->mShadowsFbo.getHeight() );
//        gl::setViewport( (*currCube)->mShadowsFbo.getBounds() );
//        glClearColor( 0.5f, 0.5f, 0.5f, 1 );
//        glClearDepth(1.0f);
//        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//        
//        glEnable(GL_TEXTURE_CUBE_MAP);
        
        mCubeShadowShader.bind();
        (*currCube)->mShadowMap.bind(0); //the magic texture
        mCubeShadowShader.uniform("shadow", 0);
        //
//        mDeferredFBO.getTexture(2).bind(1); //bind position, normal and color textures from deferred shading pass
//        mLightShader.uniform("positionMap", 1);
//        mDeferredFBO.getTexture(1).bind(2); //bind normal tex
//        mLightShader.uniform("normalMap", 2);
//        mDeferredFBO.getTexture(0).bind(3); //bind color tex
//        mLightShader.uniform("colorMap", 3);
        //
        mCubeShadowShader.uniform("light_position", mMayaCam.getCamera().getModelViewMatrix().transformPointAffine( (*currCube)->mShadowCam.getEyePoint() )); //conversion from world-space to camera-space (required here)
        mCubeShadowShader.uniform("camera_view_matrix_inv", mMayaCam.getCamera().getInverseModelViewMatrix());
        mCubeShadowShader.uniform("light_view_matrix", (*currCube)->mShadowCam.getModelViewMatrix());
        mCubeShadowShader.uniform("light_projection_matrix", (*currCube)->mShadowCam.getProjectionMatrix());

        drawScene();
//        drawShadowCasters();
//        drawPlane();
        
        (*currCube)->mShadowMap.unbind();
        glDisable(GL_TEXTURE_CUBE_MAP);
//        mDeferredFBO.getTexture(2).unbind(1); //bind position, normal and color textures from deferred shading pass
//        mDeferredFBO.getTexture(1).unbind(2); //bind normal tex
//        mDeferredFBO.getTexture(0).unbind(3); //bind color tex
        mCubeShadowShader.unbind();
        
        (*currCube)->mShadowsFbo.unbindFramebuffer();
    }
    glDisable(GL_CULL_FACE);
    
    //render all shadow layers to one FBO
    mAllShadowsFBO.bindFramebuffer();
    glClearColor( 0.5f, 0.5f, 0.5f, 0.0 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    gl::enableAlphaBlending();
    
//    mCubeLights.at(1)->mShadowsFbo.getTexture().bind();
//    gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
//    mCubeLights.at(1)->mShadowsFbo.getTexture().unbind();
    
    for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube)
    {
        if ( !(*currCube)->doesCastShadows() )
            continue;
        
        gl::setViewport( (*currCube)->mShadowsFbo.getBounds() );
        //gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
        gl::setMatricesWindow( (float)(*currCube)->mShadowsFbo.getWidth(), (float)(*currCube)->mShadowsFbo.getHeight() ); //want textures to fill screen
        
        (*currCube)->mShadowsFbo.getTexture().bind();
        gl::drawSolidRect( Rectf( 0, (float)(*currCube)->mShadowsFbo.getHeight(), (float)(*currCube)->mShadowsFbo.getWidth(), 0) ); //this is different as we are not using shaders to color these quads (need to fit viewport)
        (*currCube)->mShadowsFbo.getTexture().unbind();
    }
    gl::disableAlphaBlending();
    mAllShadowsFBO.unbindFramebuffer();
}

/*
 * @Description: render scene to FBO texture
 * @param: none
 * @return: none
 */
void CinderDeferredRenderingApp::renderDeferredFBO()
{
	//render out main scene to FBO
	mDeferredFBO.bindFramebuffer();
    gl::setViewport( mDeferredFBO.getBounds() );
    gl::setMatrices( mMayaCam.getCamera() );

	glClearColor( 0.5f, 0.5f, 0.5f, 1 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	mDeferredShader.bind();
    mDeferredShader.uniform("diff_coeff", 0.15f);
	mDeferredShader.uniform("phong_coeff", 0.3f);
	mDeferredShader.uniform("two_sided", 0.8f);
    
    drawLightMeshes();
    
    mDeferredShader.bind();
	mDeferredShader.uniform("diff_coeff", 1.0f);
	mDeferredShader.uniform("phong_coeff", 0.0f);
	mDeferredShader.uniform("two_sided", 0.0f);
    
    drawShadowCasters();
    drawPlane();
    
	mDeferredShader.unbind();
	
	mDeferredFBO.unbindFramebuffer();
}

/* 
 * @Description: render SSAO now - woohoo!
 * @param: KeyEvent
 * @return: none
 */
void CinderDeferredRenderingApp::renderSSAOToFBO()
{	
	//render out main scene to FBO
	mSSAOMap.bindFramebuffer();
	gl::setViewport( mSSAOMap.getBounds() );
    gl::setMatricesWindow( (float)mSSAOMap.getWidth(), (float)mSSAOMap.getHeight() ); //setting orthogonal view as rendering to a fullscreen quad

	glClearColor( 0.5f, 0.5f, 0.5f, 1 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    mRandomNoise.bind(0);
	mDeferredFBO.getTexture(1).bind(1);
	mSSAOShader.bind();
	mSSAOShader.uniform("rnm", 0 );
	mSSAOShader.uniform("normalMap", 1 );
    
    //look at shader and see you can set these through the client if you so desire.
    //	mSSAOShader.uniform("rnm", 1 );
    //	mSSAOShader.uniform("normalMap", 2 );	
    //	mSSAOShader.uniform("totStrength", 1.38f);
    //	mSSAOShader.uniform("strength", 0.07f);
    //	mSSAOShader.uniform("offset", 10.0f);
    //	mSSAOShader.uniform("falloff", 0.2f);
    //	mSSAOShader.uniform("rad", 0.8f);
    
    //	mSSAOShader.uniform("rnm", 1 );
    //	mSSAOShader.uniform("normalMap", 2 );
    //	mSSAOShader.uniform("farClipDist", 20.0f);
    //	mSSAOShader.uniform("screenSizeWidth", (float)getWindowWidth());
    //	mSSAOShader.uniform("screenSizeHeight", (float)getWindowHeight());
	
    //	mSSAOShader.uniform("grandom", 1 );
    //	mSSAOShader.uniform("gnormals", 2 );
    //	mSSAOShader.uniform("gdepth", 1 );
    //	mSSAOShader.uniform("gdiffuse", 1 );
    
    gl::drawSolidRect( Rectf( 0, 0, getWindowWidth(), getWindowHeight()) );
	
	mSSAOShader.unbind();
	
	mDeferredFBO.getTexture(1).unbind(1);
	mRandomNoise.unbind(0);
	
	mSSAOMap.unbindFramebuffer();
}

void CinderDeferredRenderingApp::renderLightsToFBO()
{
    mLightGlowFBO.bindFramebuffer();
    gl::setViewport( mLightGlowFBO.getBounds() );
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    //draw glowing cubes
    renderLights();
    mLightGlowFBO.unbindFramebuffer();
}

/* 
 * @Description: need to blur[the SSAO texture] horizonatally then vertically (for shader performance reasons). Called ping-ponging as it one FBO drawn to another
 * @param: KeyEvent
 * @return: none
 */
void CinderDeferredRenderingApp::pingPongBlur()
{
	//render horizontal blue first
	mPingPongBlurH.bindFramebuffer();
    gl::setMatricesWindow( (float)mPingPongBlurH.getWidth(), (float)mPingPongBlurH.getHeight() );
	gl::setViewport( mPingPongBlurH.getBounds() );
	glClearColor( 0.5f, 0.5f, 0.5f, 1 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	mSSAOMap.getTexture().bind(0);
	mHBlurShader.bind();
	mHBlurShader.uniform("RTScene", 0);
    gl::drawSolidRect( Rectf( 0, 0, getWindowWidth(), getWindowHeight()) );
	mHBlurShader.unbind();
	mSSAOMap.getTexture().unbind(0);
	
	mPingPongBlurH.unbindFramebuffer();
	
	//--------- now render vertical blur --------------
	mPingPongBlurV.bindFramebuffer();
	gl::setViewport( mPingPongBlurV.getBounds() );
    gl::setMatricesWindow( (float)mPingPongBlurV.getWidth(), (float)mPingPongBlurV.getHeight() );
	glClearColor( 0.5f, 0.5f, 0.5f, 1 );
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	mPingPongBlurH.getTexture().bind(0);
	mHBlurShader.bind();
	mHBlurShader.uniform("RTBlurH", 0);
	gl::drawSolidRect( Rectf( 0, 0, getWindowWidth(), getWindowHeight()) );
	mHBlurShader.unbind();
	mPingPongBlurH.getTexture().unbind(0);
	
	mPingPongBlurV.unbindFramebuffer();
}

/* 
 * @Description: render the final scene ( using all prior created FBOs and combine )
 * @param: none
 * @return: none
 */
void CinderDeferredRenderingApp::renderScreenSpace()
{	    
	// show the FBO texture in the upper left corner
	
	switch (RENDER_MODE) 
	{
		case SHOW_DIFFUSE_VIEW:
		{
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
			mDeferredFBO.getTexture(0).bind(0);
            gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
			mDeferredFBO.getTexture(0).unbind(0);
		}
            break;
			
		case SHOW_DEPTH_VIEW:
		{
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
			mDeferredFBO.getTexture(1).bind(0);
            mAplhaToRBG.bind();
            mAplhaToRBG.uniform("alphaTex", 0);
            gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
            mAplhaToRBG.unbind();
			mDeferredFBO.getTexture(1).unbind(0);
		}
            break;
		case SHOW_POSITION_VIEW:
		{
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
			mDeferredFBO.getTexture(2).bind(0);
            gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
			mDeferredFBO.getTexture(2).unbind(0);
		}
            break;
        
        case SHOW_ATTRIBUTE_VIEW:
		{
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
			mDeferredFBO.getTexture(3).bind(0);
            gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
			mDeferredFBO.getTexture(3).unbind(0);
		}
            break;
            
		case SHOW_NORMALMAP_VIEW:
		{
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
			mDeferredFBO.getTexture(1).bind(0);
            gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
			mDeferredFBO.getTexture(1).unbind(0);
		}
            break;
		
        case SHOW_SSAO_VIEW:
		{
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
			mSSAOMap.getTexture().bind(0);
			mBasicBlender.bind();
			mBasicBlender.uniform("ssaoTex", 0 );
            mBasicBlender.uniform("shadowsTex", 0 );
			mBasicBlender.uniform("baseTex", 0 );
			gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
			mBasicBlender.unbind();
			mSSAOMap.getTexture().unbind(0);
		}
            break;
        case SHOW_LIGHT_VIEW:
		{
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
			mLightGlowFBO.getTexture().bind(0);
			gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
            mLightGlowFBO.getTexture().unbind(0);
		}
            break;
        case SHOW_SHADOWS_VIEW:
		{
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
            mAllShadowsFBO.getTexture().bind(0);
            gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
            mAllShadowsFBO.getTexture().unbind(0);
		}
            break;
		case SHOW_FINAL_VIEW:
		{
			pingPongBlur();
            
			gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
			mPingPongBlurV.getTexture().bind(0);
            mAllShadowsFBO.getTexture().bind(1);
			mLightGlowFBO.getTexture().bind(2);
			mBasicBlender.bind();
			mBasicBlender.uniform("ssaoTex", 0 );
            mBasicBlender.uniform("shadowsTex", 1 );
			mBasicBlender.uniform("baseTex", 2 );
			gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
			mBasicBlender.unbind();
			mLightGlowFBO.getTexture().bind(2);
            mAllShadowsFBO.getTexture().bind(1);
			mPingPongBlurV.getTexture().unbind(0);
		}
			break;
	}
	
	//glDisable(GL_TEXTURE_2D);
}

/* 
 * @Description: don't use but i like to have it available
 * @param: MouseEvent
 * @return: none
 */
void CinderDeferredRenderingApp::mouseDown( MouseEvent event )
{
	if( event.isAltDown() )
		mMayaCam.mouseDown( event.getPos() );
}

void CinderDeferredRenderingApp::mouseDrag( MouseEvent event )
{
	if( event.isAltDown() )
		mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

/* 
 * @Description: Listen for key presses
 * @param: KeyEvent
 * @return: none
 */
void CinderDeferredRenderingApp::keyDown( KeyEvent event ) 
{
	//printf("%i \n", event.getCode());
	
    float lightMovInc = 0.1f;
    
	switch ( event.getCode() ) 
	{
        //switch between render views
		case KeyEvent::KEY_1:
			RENDER_MODE = 0;
			break;
		case KeyEvent::KEY_2:
			RENDER_MODE = 1;
			break;
		case KeyEvent::KEY_3:
			RENDER_MODE = 2;
			break;
		case KeyEvent::KEY_4:
			RENDER_MODE = 3;
			break;
        case KeyEvent::KEY_5:
			RENDER_MODE = 4;
			break;
        case KeyEvent::KEY_6:
			RENDER_MODE = 5;
			break;
        case KeyEvent::KEY_7:
			RENDER_MODE = 6;
			break;
        case KeyEvent::KEY_8:
			RENDER_MODE = 7;
			break;
        case KeyEvent::KEY_9:
			RENDER_MODE = 8;
			break;
            
        //change which cube you want to control
        case 269:
        {
            //minus key
            if( mCubeLights.size() > 0)
            {
                --mCurrLightIndex;
                if ( mCurrLightIndex < 0) mCurrLightIndex = mCubeLights.size() - 1;
            }
        }
            break;
        case 61:
            if( mCubeLights.size() > 0)
            {
                //plus key
                ++mCurrLightIndex;
                if ( mCurrLightIndex > mCubeLights.size() - 1) mCurrLightIndex = 0;
            }
            break;
			
        //move selected cube light
		case KeyEvent::KEY_UP:
		{
            if ( mCubeLights.size() > 0)
            {
                if(event.isShiftDown())
                    mCubeLights.at(mCurrLightIndex)->setPos( mCubeLights.at(mCurrLightIndex)->getPos() + Vec3f(0.0f, lightMovInc, 0.0f ));
                else
                    mCubeLights.at(mCurrLightIndex)->setPos( mCubeLights.at(mCurrLightIndex)->getPos() + Vec3f(0.0f, 0.0f, lightMovInc));
            }
		}
			break;
		case KeyEvent::KEY_DOWN:
		{
            if ( mCubeLights.size() > 0)
            {
                if(event.isShiftDown())
                    mCubeLights.at(mCurrLightIndex)->setPos( mCubeLights.at(mCurrLightIndex)->getPos() + Vec3f(0.0f, -lightMovInc, 0.0f ));
                else
                    mCubeLights.at(mCurrLightIndex)->setPos( mCubeLights.at(mCurrLightIndex)->getPos() + Vec3f(0.0f, 0.0, -lightMovInc));
            }
		}
			break;
		case KeyEvent::KEY_LEFT:
		{
            if ( mCubeLights.size() > 0)
            {
                mCubeLights.at(mCurrLightIndex)->setPos( mCubeLights.at(mCurrLightIndex)->getPos() + Vec3f(lightMovInc, 0.0, 0.0f));
            }
		}
			break;
		case KeyEvent::KEY_RIGHT:
		{
            if ( mCubeLights.size() > 0)
            {
                mCubeLights.at(mCurrLightIndex)->setPos( mCubeLights.at(mCurrLightIndex)->getPos() + Vec3f(-lightMovInc, 0.0, 0.0f));
            }
		}
            
        case KeyEvent::KEY_ESCAPE:
        {
            //never know when you need to quit quick
            //exit(1);
        }
			break;
		default:
			break;
	}
}

/* 
 * @Description: drawing all objects in scene here
 * @param: none
 * @return: none
 */
void CinderDeferredRenderingApp::drawShadowCasters()
{
	//just some test objects
    glColor3ub(255,0,0);
    gl::drawSphere( Vec3f(-1.0, 0.0,-1.0), 1.0f, 30 );
    
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

void CinderDeferredRenderingApp::drawScene()
{
    drawShadowCasters();
	drawPlane();
    drawLightMeshes();
}

void CinderDeferredRenderingApp::drawPlane()
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

void CinderDeferredRenderingApp::drawLightMeshes(gl::GlslProg* shader)
{
    for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube)
    {
        if ( shader != NULL )
        {
            shader->uniform("lightPos", mMayaCam.getCamera().getModelViewMatrix().transformPointAffine( (*currCube)->getPos() ) ); //pass light pos to pixel shader
            shader->uniform("lightCol", (*currCube)->getCol()); //pass light color (magnitude is power) to pixel shader
            shader->uniform("dist", (*currCube)->getDist()); //pass the light's area of effect radius to pixel shader
            (*currCube)->renderCubeDist(); //render the proxy shape
        }
        else
        {
            (*currCube)->renderCube(); //render the proxy shape
        }
        
    }
}

void CinderDeferredRenderingApp::renderLights()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE); //set blend function
	glEnable(GL_CULL_FACE); //cull front faces
	glCullFace(GL_FRONT);
	glDisable(GL_DEPTH_TEST); //disable depth testing
	glDepthMask(false);
    
	mLightShader.bind(); //bind point light pixel shader
	mDeferredFBO.getTexture(2).bind(0); //bind position, normal and color textures from deferred shading pass
	mLightShader.uniform("positionMap", 0);
	mDeferredFBO.getTexture(1).bind(1); //bind normal tex
	mLightShader.uniform("normalMap", 1);
	mDeferredFBO.getTexture(0).bind(2); //bind color tex
	mLightShader.uniform("colorMap", 2);
	mDeferredFBO.getTexture(3).bind(3); //bind attr tex
	mLightShader.uniform("attrMap", 3);
    mLightShader.uniform("camPos", mMayaCam.getCamera().getEyePoint());
    
    drawLightMeshes( &mLightShader );
    
	mLightShader.unbind(); //unbind and reset everything to desired values
	mDeferredFBO.getTexture(2).unbind(0); //bind position, normal and color textures from deferred shading pass
	mDeferredFBO.getTexture(1).unbind(0); //bind normal tex
	mDeferredFBO.getTexture(0).unbind(0); //bind color tex
	mDeferredFBO.getTexture(3).unbind(0); //bind attr tex
    
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glDisable(GL_BLEND);
}

/* 
 * @Description: initialize all shaders here
 * @param: none
 * @return: none
 */
void CinderDeferredRenderingApp::initShaders()
{
	mSSAOShader			= gl::GlslProg( loadResource( SSAO_VERT ), loadResource( SSAO_FRAG_LIGHT ) );
	mDeferredShader		= gl::GlslProg( loadResource( DEFER_VERT ), loadResource( DEFER_FRAG ) );
	mBasicBlender		= gl::GlslProg( loadResource( BBlender_VERT ), loadResource( BBlender_FRAG ) );
	mHBlurShader		= gl::GlslProg( loadResource( BLUR_H_VERT ), loadResource( BLUR_H_FRAG ) );
	mVBlurShader		= gl::GlslProg( loadResource( BLUR_V_VERT ), loadResource( BLUR_V_FRAG ) );
    mLightShader		= gl::GlslProg( loadResource( LIGHT_VERT ), loadResource( LIGHT_FRAG ) );
    mAplhaToRBG         = gl::GlslProg( loadResource( ALPHA_RGB_VERT ), loadResource( ALPHA_RGB_FRAG ) );
    mCubeShadowShader   = gl::GlslProg( loadResource( RES_SHADER_CUBESHADOW_VERT ), loadResource( RES_SHADER_CUBESHADOW_FRAG ) );
}

/* 
 * @Description: initialize all FBOs here
 * @param: none
 * @return: none
 */
void CinderDeferredRenderingApp::initFBOs()
{		
	//this FBO will capture normals, depth, and base diffuse in one render pass (as opposed to three)
	gl::Fbo::Format mtRFBO;
	mtRFBO.enableDepthBuffer();
	mtRFBO.setDepthInternalFormat( GL_DEPTH_COMPONENT32 ); //want fbo to have precision depth map as well
	mtRFBO.setColorInternalFormat( GL_RGBA16F_ARB );
	mtRFBO.enableColorBuffer( true, 4 ); // create an FBO with four color attachments (basic diffuse, normal/depth view, attribute view, and position view)
	//mtRFBO.setSamples( 4 ); // uncomment this to enable 4x antialiasing

    gl::Fbo::Format format;
	//format.setDepthInternalFormat( GL_DEPTH_COMPONENT32 );
	format.setColorInternalFormat( GL_RGBA16F_ARB );
	//format.setSamples( 4 ); // enable 4x antialiasing
    
	//init screen space render
    //x2 so anti-aliasing can be automatically done when scaling down texture to window size (deferred rendering doesn't leave many options for anti-aliasing ...
    
	mDeferredFBO	= gl::Fbo( FBO_RESOLUTION * 2.0f, FBO_RESOLUTION * 2.0f, mtRFBO );
    mLightGlowFBO   = gl::Fbo( FBO_RESOLUTION, FBO_RESOLUTION, format );
    mPingPongBlurH	= gl::Fbo( FBO_RESOLUTION, FBO_RESOLUTION, format );
	mPingPongBlurV	= gl::Fbo( FBO_RESOLUTION, FBO_RESOLUTION, format );
	mSSAOMap		= gl::Fbo( FBO_RESOLUTION, FBO_RESOLUTION, format );
    mAllShadowsFBO  = gl::Fbo( FBO_RESOLUTION, FBO_RESOLUTION, format );
    
    
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );	
}

CINDER_APP_BASIC( CinderDeferredRenderingApp, RendererGl )

#endif
