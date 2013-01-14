//
//  DeferredRenderer.h
//  CinderDeferredRendering
//
//  Created by Anthony Scavarelli on 2012-12-31.
//
//

#ifndef CinderDeferredRendering_DeferredRenderer_h
#define CinderDeferredRendering_DeferredRenderer_h

#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"

#include "boost/function.hpp"

#include "CubeShadowMap.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const Vec2i	FBO_RESOLUTION(1280, 720);  //using window dimensions
static const float  LIGHT_CUTOFF = 0.01;        //light intensity cutoff point
static const float  LIGHT_BRIGHTNESS = 40;      //brightness of lights
static const int	SHADOW_MAP_RESOLUTION = 512;

enum
{
    SHOW_FINAL_VIEW,
	SHOW_DIFFUSE_VIEW,
    SHOW_NORMALMAP_VIEW,
    SHOW_DEPTH_VIEW,
    SHOW_POSITION_VIEW,
    SHOW_ATTRIBUTE_VIEW,
	SHOW_SSAO_VIEW,
    SHOW_SSAO_BLURRED_VIEW,
    SHOW_LIGHT_VIEW,
    SHOW_SHADOWS_VIEW
};

//Light Cube Class
class Light_PS
{
public:
    float           aoeDist;
    float           cubeSize;
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
        aoeDist = sqrt(col.length()/LIGHT_CUTOFF);
        cubeSize = 2.0f;
        
        //set up fake "light" to grab matrix calculations from 
        mShadowCam.setPerspective( 90.0f, 1.0f, 1.0f, 100.0f );
        mShadowCam.lookAt( p, Vec3f( p.x, 0.0f, p.z ) );
        
        mCastShadows = castsShadows;
        if (mCastShadows)
        {
            setUpShadowStuff();
        }
    }
    
    void setUpShadowStuff()
    {
        //set up cube map for point shadows
        mShadowMap.setup( SHADOW_MAP_RESOLUTION );
        
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
        mShadowsFbo	= gl::Fbo( SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, format );
    }
    
	void setPos(Vec3f p)
    {
        mShadowCam.lookAt( p, Vec3f( p.x, 0.0f, p.z ) );
        pos = p;
    }
    
	void setCol(Vec3f c)
    {
        col = c;
        aoeDist = sqrt(col.length()/LIGHT_CUTOFF);
    }
    
	void renderCube() const
    {
        gl::drawCube(pos, Vec3f(cubeSize, cubeSize, cubeSize));
    }
    
    void renderCubeAOE() const
    {
        gl::drawCube(pos, Vec3f(aoeDist, aoeDist, aoeDist));
    }
    
    inline const bool doesCastShadows(){ return mCastShadows; }
    inline const Vec3f& getPos() const {return pos;}
    inline const Vec3f& getCol() const {return col;}
	inline float getDist() const {return aoeDist;}
};

class DeferredRenderer
{
    public: 
    boost::function<void()> fRenderShadowCastersFunc;
    boost::function<void()> fRenderNotShadowCastersFunc;
    MayaCamUI              *mMayaCam;
    
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
    
    public:
    DeferredRenderer(){  };
    ~DeferredRenderer(){  };
    
    vector<Light_PS*>* getCubeLightsRef(){ return &mCubeLights; };
    const int getNumCubeLights(){ return mCubeLights.size(); };
    
    void setup(boost::function<void(void)> renderShadowCastFunc, boost::function<void(void)> renderObjFunc, MayaCamUI *cam )
    {
        fRenderShadowCastersFunc = renderShadowCastFunc;
        fRenderNotShadowCastersFunc = renderObjFunc;
        mMayaCam = cam;
        
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
        
        initTextures();
        initFBOs();
        initShaders();
    }

    void update(){}
    
    void addCubeLight(Vec3f position, Vec3f color, bool castsShadows = false)
    {
        mCubeLights.push_back( new Light_PS( position, color, castsShadows ));
    }
    
    void prepareDeferredScene()
    {
        //clear depth and color every frame
        glClearColor( 0.5f, 0.5f, 0.5f, 1 );
        glClearDepth(1.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        gl::setMatrices( mMayaCam->getCamera() );
        
        renderDeferredFBO();
        
        createShadowMaps();
        renderShadowsToFBOs();
        
        gl::setMatrices( mMayaCam->getCamera() );
        
        renderLightsToFBO();
        renderSSAOToFBO();
        
//        renderScreenSpace(); //render full screen quad image of scene
    }

    void createShadowMaps()
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
                
                if (fRenderShadowCastersFunc) fRenderShadowCastersFunc();
            }
            
            (*currCube)->mCubeDepthFbo.unbindFramebuffer();
        }
        glDisable(GL_CULL_FACE);
    }
    
    void renderShadowsToFBOs()
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
            gl::setViewport( (*currCube)->mShadowsFbo.getBounds() );
            
            glCullFace(GL_BACK); //don't need what we won't see
            
            gl::setMatrices( mMayaCam->getCamera() );
            
            mCubeShadowShader.bind();
            (*currCube)->mShadowMap.bind(0); //the magic texture
            mCubeShadowShader.uniform("shadow", 0);

            mCubeShadowShader.uniform("light_position", mMayaCam->getCamera().getModelViewMatrix().transformPointAffine( (*currCube)->mShadowCam.getEyePoint() )); //conversion from world-space to camera-space (required here)
            mCubeShadowShader.uniform("camera_view_matrix_inv", mMayaCam->getCamera().getInverseModelViewMatrix());
            mCubeShadowShader.uniform("light_view_matrix", (*currCube)->mShadowCam.getModelViewMatrix());
            mCubeShadowShader.uniform("light_projection_matrix", (*currCube)->mShadowCam.getProjectionMatrix());
            
            drawScene();
            
            (*currCube)->mShadowMap.unbind();
            glDisable(GL_TEXTURE_CUBE_MAP);

            mCubeShadowShader.unbind();
            
            (*currCube)->mShadowsFbo.unbindFramebuffer();
        }
        glDisable(GL_CULL_FACE);
        
        //render all shadow layers to one FBO
        mAllShadowsFBO.bindFramebuffer();
        gl::setViewport( mAllShadowsFBO.getBounds() );
        gl::setMatricesWindow( (float)mAllShadowsFBO.getWidth(), (float)mAllShadowsFBO.getHeight() ); //want textures to fill FBO
        glClearColor( 0.5f, 0.5f, 0.5f, 0.0 );
        glClearDepth(1.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        gl::enableAlphaBlending();
        
        for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube)
        {
            if ( !(*currCube)->doesCastShadows() )
                continue;
            
            (*currCube)->mShadowsFbo.getTexture().bind();
            gl::drawSolidRect( Rectf( 0, (float)mAllShadowsFBO.getHeight(), (float)mAllShadowsFBO.getWidth(), 0) ); //this is different as we are not using shaders to color these quads (need to fit viewport)
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
    void renderDeferredFBO()
    {
        //render out main scene to FBO
        mDeferredFBO.bindFramebuffer();
        gl::setViewport( mDeferredFBO.getBounds() );
        gl::setMatrices( mMayaCam->getCamera() );
        
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
        
        if (fRenderShadowCastersFunc) fRenderShadowCastersFunc();
        if (fRenderNotShadowCastersFunc) fRenderNotShadowCastersFunc();
//        drawShadowCasters();
//        drawPlane();
        
        mDeferredShader.unbind();
        
        mDeferredFBO.unbindFramebuffer();
    }
    
    /*
     * @Description: render SSAO now - woohoo!
     * @param: KeyEvent
     * @return: none
     */
    void renderSSAOToFBO()
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
    
    void renderLightsToFBO()
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
    void pingPongBlur()
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
        mHBlurShader.uniform("blurStep", 1.0f/mPingPongBlurH.getWidth()); //so that every "blur step" will equal one pixel width of this FBO
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
        mHBlurShader.uniform("blurStep", 1.0f/mPingPongBlurH.getHeight()); //so that every "blur step" will equal one pixel height of this FBO
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
    void renderFullScreenQuad( int renderType )
    {
        prepareDeferredScene(); //prepare all FBOs for use
        
        switch (renderType)
        {
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
            case SHOW_SSAO_BLURRED_VIEW:
            {
                pingPongBlur();
                
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mPingPongBlurV.getTexture().bind(0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mPingPongBlurV.getTexture().unbind(0);
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
        }
        
        //glDisable(GL_TEXTURE_2D);
    }
    
    void drawLightMeshes(gl::GlslProg* shader = NULL)
    {
        for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube)
        {
            if ( shader != NULL )
            {
                shader->uniform("lightPos", mMayaCam->getCamera().getModelViewMatrix().transformPointAffine( (*currCube)->getPos() ) ); //pass light pos to pixel shader
                shader->uniform("lightCol", (*currCube)->getCol()); //pass light color (magnitude is power) to pixel shader
                shader->uniform("dist", (*currCube)->getDist()); //pass the light's area of effect radius to pixel shader
                (*currCube)->renderCubeAOE(); //render the proxy shape
            }
            else
            {
                (*currCube)->renderCube(); //render the proxy shape
            }
            
        }
    }
    
    void drawScene()
    {
        if(fRenderShadowCastersFunc) fRenderShadowCastersFunc();
        if(fRenderNotShadowCastersFunc) fRenderNotShadowCastersFunc();
        drawLightMeshes();
    }
    
    void renderLights()
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
        mLightShader.uniform("camPos", mMayaCam->getCamera().getEyePoint());
        
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
    
    void initTextures()
    {
        mRandomNoise = gl::Texture( loadImage( loadResource( NOISE_SAMPLER ) ) ); //noise texture required for SSAO calculations
    }
    
    /* 
     * @Description: initialize all shaders here
     * @param: none
     * @return: none
     */
    void initShaders()
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
    void initFBOs()
    {		
        //this FBO will capture normals, depth, and base diffuse in one render pass (as opposed to three)
        gl::Fbo::Format mtRFBO;
        mtRFBO.enableDepthBuffer();
        mtRFBO.setDepthInternalFormat( GL_DEPTH_COMPONENT16 ); //want fbo to have precision depth map as well
        mtRFBO.setColorInternalFormat( GL_RGBA16F_ARB );
        mtRFBO.enableColorBuffer( true, 4 ); // create an FBO with four color attachments (basic diffuse, normal/depth view, attribute view, and position view)
        //mtRFBO.setSamples( 4 ); // uncomment this to enable 4x antialiasing
        
        gl::Fbo::Format format;
        //format.setDepthInternalFormat( GL_DEPTH_COMPONENT32 );
        //format.setColorInternalFormat( GL_RGBA16F_ARB );
        //format.setSamples( 4 ); // enable 4x antialiasing
        
        //init screen space render
        mDeferredFBO	= gl::Fbo( FBO_RESOLUTION.x, FBO_RESOLUTION.y, mtRFBO );
        mLightGlowFBO   = gl::Fbo( FBO_RESOLUTION.x, FBO_RESOLUTION.y, format );
        mPingPongBlurH	= gl::Fbo( FBO_RESOLUTION.x, FBO_RESOLUTION.y, format );
        mPingPongBlurV	= gl::Fbo( FBO_RESOLUTION.x, FBO_RESOLUTION.y, format );
        mSSAOMap		= gl::Fbo( FBO_RESOLUTION.x, FBO_RESOLUTION.y, format );
        mAllShadowsFBO  = gl::Fbo( FBO_RESOLUTION.x, FBO_RESOLUTION.y, format );
        
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );	
    }
    
};

#endif
