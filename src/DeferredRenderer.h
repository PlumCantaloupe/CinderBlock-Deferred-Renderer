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
#include "cinder/gl/Vbo.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"

#include "boost/function.hpp"

#include "CubeShadowMap.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const float  LIGHT_CUTOFF_DEFAULT = 0.01;        //light intensity cutoff point
static const float  LIGHT_BRIGHTNESS_DEFAULT = 60;      //brightness of lights

//Light Cube Class
class Light_PS
{
    
public:
    float                       mCubeSize;
    CameraPersp                 mShadowCam;
    gl_Plum::CubeShadowMap      mShadowMap;
    gl::Fbo                     mCubeDepthFbo;
    gl::Fbo                     mShadowsFbo;
    
private:
    Vec3f                       mPos;
    Color                       mCol;
    bool                        mCastShadows;
    bool                        mVisible;
    int                         mShadowMapRes;
    float                       mAOEDist;
    
    gl::VboMesh                 *mCubeVBOMeshRef;
    float                       transformMatBase[16]; //scale/translate matrix
    float                       transformMatAOE[16]; //scale/translate matrix
    
public:
	Light_PS(gl::VboMesh *vboMeshRef, Vec3f pos, Color col, int shadowMapRes, bool castsShadows = false, bool visible = true)
    {
        mCubeVBOMeshRef = vboMeshRef;
        mPos = pos;
        mCol = col;
        mAOEDist = sqrt(col.length()/LIGHT_CUTOFF_DEFAULT);
        mCubeSize = 2.0f;
        mShadowMapRes = shadowMapRes;
        
        //set up fake "light" to grab matrix calculations from
        mShadowCam.setPerspective( 90.0f, 1.0f, 1.0f, 100.0f );
        mShadowCam.lookAt( pos, Vec3f( pos.x, 0.0f, pos.z ) );
        
        mCastShadows = castsShadows;
        mVisible = visible;
        if (mCastShadows) {setUpShadowStuff();}
        
        //create matrix
        transformMatBase[0] = mCubeSize;    transformMatBase[1] = 0.0f;         transformMatBase[2] = 0.0f;         transformMatBase[3] = 0.0f;
        transformMatBase[4] = 0.0f;         transformMatBase[5] = mCubeSize;    transformMatBase[6] = 0.0f;         transformMatBase[7] = 0.0f;
        transformMatBase[8] = 0.0f;         transformMatBase[9] = 0.0f;         transformMatBase[10] = mCubeSize;   transformMatBase[11] = 0.0f;
        transformMatBase[12] = pos.x;       transformMatBase[13] = pos.y;       transformMatBase[14] = pos.z;       transformMatBase[15] = 1.0f;
        
        transformMatAOE[0] = mAOEDist;      transformMatAOE[1] = 0.0f;          transformMatAOE[2] = 0.0f;          transformMatAOE[3] = 0.0f;
        transformMatAOE[4] = 0.0f;          transformMatAOE[5] = mAOEDist;      transformMatAOE[6] = 0.0f;          transformMatAOE[7] = 0.0f;
        transformMatAOE[8] = 0.0f;          transformMatAOE[9] = 0.0f;          transformMatAOE[10] = mAOEDist;     transformMatAOE[11] = 0.0f;
        transformMatAOE[12] = pos.x;        transformMatAOE[13] = pos.y;        transformMatAOE[14] = pos.z;        transformMatAOE[15] = 1.0f;
    }
    
    void setUpShadowStuff()
    {
        //set up cube map for point shadows
        mShadowMap.setup( mShadowMapRes );
        
        //create FBO to hold depth values from cube map
        gl::Fbo::Format formatShadow;
        formatShadow.enableColorBuffer(true, 1);
        formatShadow.enableDepthBuffer(true, true);
        formatShadow.setMinFilter(GL_LINEAR);
        formatShadow.setMagFilter(GL_LINEAR);
        formatShadow.setWrap(GL_CLAMP, GL_CLAMP);
        mCubeDepthFbo   = gl::Fbo( mShadowMapRes, mShadowMapRes, formatShadow);
        
        gl::Fbo::Format format;
        format.setDepthInternalFormat( GL_DEPTH_COMPONENT24 );
        format.setColorInternalFormat( GL_RGBA8 );
        //format.setSamples( 4 ); // enable 4x antialiasing
        mShadowsFbo	= gl::Fbo( mShadowMapRes, mShadowMapRes, format );
    }
    
	void setPos(const Vec3f pos)
    {
        mShadowCam.lookAt( pos, Vec3f( pos.x, 0.0f, pos.z ) );
        mPos = pos;
        transformMatBase[12] = pos.x;        transformMatBase[13] = pos.y;        transformMatBase[14] = pos.z;
        transformMatAOE[12] = pos.x;         transformMatAOE[13] = pos.y;         transformMatAOE[14] = pos.z;
    }
    
    Vec3f getPos() const
    {
        return mPos;
    }
    
	void setCol(const Color col)
    {
        mCol = col;
        mAOEDist = sqrt(col.length()/LIGHT_CUTOFF_DEFAULT);
    }
    
    Color getColor() const
    {
        return mCol;
    }
    
    float getAOEDist() const
    {
        return mAOEDist;
    }
    
	void renderCube() const
    {
        if( mVisible ) {
            //gl::drawCube(mPos, Vec3f(mCubeSize, mCubeSize, mCubeSize));
            
            gl::pushMatrices();
            gl::multModelView(transformMatBase);
            gl::draw(*mCubeVBOMeshRef);
            gl::popMatrices();
        }
    }
    
    void renderCubeAOE() const
    {
        //gl::drawCube(mPos, Vec3f(mAOEDist, mAOEDist, mAOEDist));
        
        gl::pushMatrices();
        gl::multModelView(transformMatAOE);
        gl::draw(*mCubeVBOMeshRef);
        gl::popMatrices();
    }
    
    bool doesCastShadows() const { return mCastShadows; }
};

class DeferredRenderer
{
public:
    std::function<void(gl::GlslProg*)> fRenderShadowCastersFunc;
    std::function<void(gl::GlslProg*)> fRenderNotShadowCastersFunc;
    std::function<void()> fRenderOverlayFunc;
    MayaCamUI              *mMayaCam;
    
    Matrix44f           mLightFaceViewMatrices[6];
	
    gl::Texture			mRandomNoise;
    
    gl::Fbo				mDeferredFBO;
    gl::Fbo				mSSAOMap;
    gl::Fbo				mPingPongBlurH;
    gl::Fbo				mPingPongBlurV;
    gl::Fbo				mLightGlowFBO;
    gl::Fbo				mAllShadowsFBO;
	gl::Fbo				mFinalSSFBO;
    gl::Fbo				mOverlayFBO;
    
	gl::GlslProg		mCubeShadowShader;
	
    gl::GlslProg		mSSAOShader;
    gl::GlslProg		mDeferredShader;
    gl::GlslProg		mBasicBlender;
    gl::GlslProg		mHBlurShader;
    gl::GlslProg		mVBlurShader;
    gl::GlslProg		mLightShader;
    gl::GlslProg		mAlphaToRBG;
	gl::GlslProg		mFXAAShader;
    
    vector<Light_PS*>   mCubeLights;
    
    Vec2i               mFBOResolution;
    int                 mShadowMapRes;
    
    gl::VboMesh         mCubeVBOMesh; //pass cube Vbo to all "lights" to save on draw calls
    
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
    
public:
    DeferredRenderer(){};
    ~DeferredRenderer(){};
    
    vector<Light_PS*>* getCubeLightsRef(){ return &mCubeLights; };
    const int getNumCubeLights(){ return mCubeLights.size(); };
    
    void setup( const std::function<void(gl::GlslProg*)> renderShadowCastFunc,
                const std::function<void(gl::GlslProg*)> renderObjFunc,
                const std::function<void()> renderOverlayFunc,
                MayaCamUI *cam,
                Vec2i     FBORes = Vec2i(512, 512),
                int       shadowMapRes = 512)
    {
        //create cube VBO reference for lights
        getCubeVboMesh( &mCubeVBOMesh, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f) );
        
        fRenderShadowCastersFunc = renderShadowCastFunc;
        fRenderNotShadowCastersFunc = renderObjFunc;
        fRenderOverlayFunc = renderOverlayFunc;
        mMayaCam = cam;
        mFBOResolution = FBORes;
        mShadowMapRes = shadowMapRes;
        
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
        mLightFaceViewMatrices[ gl_Plum::CubeShadowMap::X_FACE_POS ] = cubeCam.getModelViewMatrix();
        cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0), Vec3f(-1.0, 0.0, 0.0),  Vec3f(0.0,-1.0, 0.0));
        mLightFaceViewMatrices[ gl_Plum::CubeShadowMap::X_FACE_NEG ] = cubeCam.getModelViewMatrix();
        cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(0.0, 1.0, 0.0),  Vec3f(0.0, 0.0, 1.0));
        mLightFaceViewMatrices[ gl_Plum::CubeShadowMap::Y_FACE_POS ] = cubeCam.getModelViewMatrix();
        cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(0.0,-1.0, 0.0),  Vec3f(0.0, 0.0,-1.0) );
        mLightFaceViewMatrices[ gl_Plum::CubeShadowMap::Y_FACE_NEG ] = cubeCam.getModelViewMatrix();
        cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(0.0, 0.0, 1.0),  Vec3f(0.0,-1.0, 0.0) );
        mLightFaceViewMatrices[ gl_Plum::CubeShadowMap::Z_FACE_POS ] = cubeCam.getModelViewMatrix();
        cubeCam.lookAt( Vec3f(0.0, 0.0, 0.0),  Vec3f(0.0, 0.0,-1.0),  Vec3f(0.0,-1.0, 0.0) );
        mLightFaceViewMatrices[ gl_Plum::CubeShadowMap::Z_FACE_NEG ] = cubeCam.getModelViewMatrix();
        
        initTextures();
        initFBOs();
        initShaders();
    }
    
    void update(){}
    
    Light_PS* addCubeLight(const Vec3f position, const Color color, const bool castsShadows = false, const bool visible = true)
    {
        Light_PS *newLightP = new Light_PS( &mCubeVBOMesh, position, color, mShadowMapRes, castsShadows, visible );
        mCubeLights.push_back( newLightP );
        return newLightP;
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
    }
    
    void createShadowMaps()
    {
        //render depth map cube
        glEnable(GL_CULL_FACE);
        for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube)
        {
            if (!(*currCube)->doesCastShadows()) {continue;}
            
            (*currCube)->mCubeDepthFbo.bindFramebuffer();
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glViewport(0, 0, mShadowMapRes, mShadowMapRes);
            
            glCullFace(GL_FRONT);
            
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf((*currCube)->mShadowCam.getProjectionMatrix());
            glMatrixMode(GL_MODELVIEW);
            
            for (size_t i = 0; i < 6; ++i) {
                (*currCube)->mShadowMap.bindDepthFB( i );
                glClear(GL_DEPTH_BUFFER_BIT);
                
                glLoadMatrixf(mLightFaceViewMatrices[i]);
                glMultMatrixf((*currCube)->mShadowCam.getModelViewMatrix());
                
                if (fRenderShadowCastersFunc) {fRenderShadowCastersFunc(NULL);}
            }
            
            (*currCube)->mCubeDepthFbo.unbindFramebuffer();
        }
        glDisable(GL_CULL_FACE);
    }
    
    void renderShadowsToFBOs()
    {
        glEnable(GL_CULL_FACE);
        for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube) {
            if (!(*currCube)->doesCastShadows()) {continue;}
            
            (*currCube)->mShadowsFbo.bindFramebuffer();
            
            glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            //glDrawBuffer(GL_BACK);
            //glReadBuffer(GL_BACK);
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
        
        for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube) {
            if ( !(*currCube)->doesCastShadows() ) {continue;}
            
            (*currCube)->mShadowsFbo.getTexture().bind();
            gl::drawSolidRect( Rectf( 0, (float)mAllShadowsFBO.getHeight(), (float)mAllShadowsFBO.getWidth(), 0) ); //this is different as we are not using shaders to color these quads (need to fit viewport)
            (*currCube)->mShadowsFbo.getTexture().unbind();
        }
        gl::disableAlphaBlending();
        mAllShadowsFBO.unbindFramebuffer();
    }
    
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
        mDeferredShader.uniform("useTexture", 0.0f);
        
        drawLightMeshes();
        
        mDeferredShader.bind();
        mDeferredShader.uniform("diff_coeff", 1.0f);
        mDeferredShader.uniform("phong_coeff", 0.0f);
        mDeferredShader.uniform("two_sided", 0.8f);
        
        if (fRenderShadowCastersFunc) {fRenderShadowCastersFunc(&mDeferredShader);}
        if (fRenderNotShadowCastersFunc) {fRenderNotShadowCastersFunc(&mDeferredShader);}
        
        mDeferredShader.unbind();
        mDeferredFBO.unbindFramebuffer();
    }
    
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
        
        gl::drawSolidRect( Rectf( 0, 0, mSSAOMap.getWidth(), mSSAOMap.getHeight()) );
        
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
    
    void pingPongBlurSSAO()
    {
        //--------- render horizontal blur first --------------
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
        gl::drawSolidRect( Rectf( 0, 0, mPingPongBlurH.getWidth(), mPingPongBlurH.getHeight()) );
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
        gl::drawSolidRect( Rectf( 0, 0, mPingPongBlurH.getWidth(), mPingPongBlurH.getHeight()) );
        mHBlurShader.unbind();
        mPingPongBlurH.getTexture().unbind(0);
        mPingPongBlurV.unbindFramebuffer();
    }
    
    void renderFullScreenQuad( const int renderType )
    {
        prepareDeferredScene(); //prepare all FBOs for use
        
        switch (renderType)
        {
            case SHOW_FINAL_VIEW: {
                pingPongBlurSSAO();
                
                if(fRenderOverlayFunc) {
                    gl::enableAlphaBlending();
                    gl::disableDepthRead();
                    
                    mOverlayFBO.bindFramebuffer();
                    glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
                    glClearDepth(1.0f);
                    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                    gl::setMatrices(mMayaCam->getCamera());
                    gl::setViewport( mOverlayFBO.getBounds() );
                    
                    fRenderOverlayFunc();
                    
                    mOverlayFBO.unbindFramebuffer();
                    gl::enableDepthRead();
                }
                
				mFinalSSFBO.bindFramebuffer();
				glClearColor( 0.5f, 0.5f, 0.5f, 1 );
				glClearDepth(1.0f);
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                gl::setMatricesWindow( (float)mFinalSSFBO.getWidth(), (float)mFinalSSFBO.getHeight() );
                gl::setViewport( mFinalSSFBO.getBounds() );
                mPingPongBlurV.getTexture().bind(0);
                mAllShadowsFBO.getTexture().bind(1);
                mLightGlowFBO.getTexture().bind(2);
                mBasicBlender.bind();
                mBasicBlender.uniform("ssaoTex", 0 );
                mBasicBlender.uniform("shadowsTex", 1 );
                mBasicBlender.uniform("baseTex", 2 );
                gl::drawSolidRect( Rectf( 0, mFinalSSFBO.getWidth(), mFinalSSFBO.getHeight(), 0) );
                mBasicBlender.unbind();
                mLightGlowFBO.getTexture().unbind(2);
                mAllShadowsFBO.getTexture().unbind(1);
                mPingPongBlurV.getTexture().unbind(0);
                
                
                if(fRenderOverlayFunc) {
                    mOverlayFBO.getTexture().bind();
                    gl::drawSolidRect( Rectf( 0, mFinalSSFBO.getHeight(), mFinalSSFBO.getWidth(), 0) ); //?? why do I have to draw this sideways??
                    mOverlayFBO.getTexture().unbind();
                    gl::disableAlphaBlending();
                }
                
				mFinalSSFBO.unbindFramebuffer();
                
				mFinalSSFBO.getTexture().bind(0);
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
				mFXAAShader.bind();
				mFXAAShader.uniform("buf0", 0);
				mFXAAShader.uniform("frameBufSize", Vec2f(mFinalSSFBO.getWidth(), mFinalSSFBO.getHeight()));
				gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
				mFXAAShader.unbind();
				mFinalSSFBO.getTexture().unbind(0);
            }
                break;
            case SHOW_DIFFUSE_VIEW: {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mDeferredFBO.getTexture(0).bind(0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mDeferredFBO.getTexture(0).unbind(0);
            }
                break;
            case SHOW_DEPTH_VIEW: {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mDeferredFBO.getTexture(1).bind(0);
                mAlphaToRBG.bind();
                mAlphaToRBG.uniform("alphaTex", 0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mAlphaToRBG.unbind();
                mDeferredFBO.getTexture(1).unbind(0);
            }
                break;
            case SHOW_POSITION_VIEW: {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mDeferredFBO.getTexture(2).bind(0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mDeferredFBO.getTexture(2).unbind(0);
            }
                break;
            case SHOW_ATTRIBUTE_VIEW: {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mDeferredFBO.getTexture(3).bind(0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mDeferredFBO.getTexture(3).unbind(0);
            }
                break;
                
            case SHOW_NORMALMAP_VIEW: {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mDeferredFBO.getTexture(1).bind(0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mDeferredFBO.getTexture(1).unbind(0);
            }
                break;
            case SHOW_SSAO_VIEW: {
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
            case SHOW_SSAO_BLURRED_VIEW: {
                pingPongBlurSSAO();
                
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mPingPongBlurV.getTexture().bind(0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mPingPongBlurV.getTexture().unbind(0);
            }
                break;
            case SHOW_LIGHT_VIEW: {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mLightGlowFBO.getTexture().bind(0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mLightGlowFBO.getTexture().unbind(0);
            }
                break;
            case SHOW_SHADOWS_VIEW: {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mAllShadowsFBO.getTexture().bind(0);
                gl::drawSolidRect( Rectf( 0, getWindowHeight(), getWindowWidth(), 0) );
                mAllShadowsFBO.getTexture().unbind(0);
            }
                break;
        }
    }
    
    void drawLightMeshes(gl::GlslProg* shader = NULL)
    {
        for(vector<Light_PS*>::iterator currCube = mCubeLights.begin(); currCube != mCubeLights.end(); ++currCube) {
            if ( shader != NULL ) {
                shader->uniform("lightPos", mMayaCam->getCamera().getModelViewMatrix().transformPointAffine( (*currCube)->getPos() ) ); //pass light pos to pixel shader
                shader->uniform("lightCol", (*currCube)->getColor()); //pass light color (magnitude is power) to pixel shader
                shader->uniform("dist", (*currCube)->getAOEDist()); //pass the light's area of effect radius to pixel shader
                (*currCube)->renderCubeAOE(); //render the proxy shape
            }
            else {
                (*currCube)->renderCube(); //render the proxy shape
            }
            
        }
    }
    
    void drawScene()
    {
        if(fRenderShadowCastersFunc) {fRenderShadowCastersFunc(NULL);}
        if(fRenderNotShadowCastersFunc) {fRenderNotShadowCastersFunc(NULL);}
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
        mDeferredFBO.getTexture(1).unbind(1); //bind normal tex
        mDeferredFBO.getTexture(0).unbind(2); //bind color tex
        mDeferredFBO.getTexture(3).unbind(3); //bind attr tex
        
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glDisable(GL_BLEND);
    }
    
    void initTextures()
    {
        mRandomNoise = gl::Texture( loadImage( loadResource( NOISE_SAMPLER ) ) ); //noise texture required for SSAO calculations
    }
    
    void initShaders()
    {
        mSSAOShader			= gl::GlslProg( loadResource( SSAO_VERT ), loadResource( SSAO_FRAG_LIGHT ) );
        mDeferredShader		= gl::GlslProg( loadResource( DEFER_VERT ), loadResource( DEFER_FRAG ) );
        mBasicBlender		= gl::GlslProg( loadResource( BBlender_VERT ), loadResource( BBlender_FRAG ) );
        mHBlurShader		= gl::GlslProg( loadResource( BLUR_H_VERT ), loadResource( BLUR_H_FRAG ) );
        mVBlurShader		= gl::GlslProg( loadResource( BLUR_V_VERT ), loadResource( BLUR_V_FRAG ) );
        mLightShader		= gl::GlslProg( loadResource( LIGHT_VERT ), loadResource( LIGHT_FRAG ) );
        mAlphaToRBG         = gl::GlslProg( loadResource( ALPHA_RGB_VERT ), loadResource( ALPHA_RGB_FRAG ) );
        mCubeShadowShader   = gl::GlslProg( loadResource( RES_SHADER_CUBESHADOW_VERT ), loadResource( RES_SHADER_CUBESHADOW_FRAG ) );
		mFXAAShader			= gl::GlslProg( loadResource( RES_SHADER_FXAA_VERT ), loadResource( RES_SHADER_FXAA_FRAG ) );
    }
    
    void initFBOs()
    {
        //this FBO will capture normals, depth, and base diffuse in one render pass (as opposed to three)
        gl::Fbo::Format deferredFBO;
        deferredFBO.enableDepthBuffer();
        deferredFBO.setDepthInternalFormat( GL_DEPTH_COMPONENT24 ); //want fbo to have precision depth map as well
        deferredFBO.setColorInternalFormat( GL_RGBA16F_ARB );
        deferredFBO.enableColorBuffer( true, 4 ); // create an FBO with four color attachments (basic diffuse, normal/depth view, attribute view, and position view)
        
        gl::Fbo::Format basicFormat;
        basicFormat.enableDepthBuffer(false); //don't need depth "layers"
        
        //init screen space render
        mDeferredFBO	= gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   deferredFBO );
        mLightGlowFBO   = gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicFormat );
        mPingPongBlurH	= gl::Fbo( mFBOResolution.x/2,  mFBOResolution.y/2, basicFormat ); //don't need as high res on ssao as it will be blurred anyhow ...
        mPingPongBlurV	= gl::Fbo( mFBOResolution.x/2,  mFBOResolution.y/2, basicFormat );
        mSSAOMap		= gl::Fbo( mFBOResolution.x/2,  mFBOResolution.y/2, basicFormat );
        mAllShadowsFBO  = gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicFormat );
        mFinalSSFBO		= gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicFormat );
        mOverlayFBO     = gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicFormat );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    }
    
    void getCubeVboMesh( gl::VboMesh *vboMesh, const Vec3f &c, const Vec3f &size )
    {
        float sx = size.x * 0.5f;
        float sy = size.y * 0.5f;
        float sz = size.z * 0.5f;
        Vec3f vertices[24]={
            Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),     Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),    Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	// +X
            Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),     Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),	// +Y
            Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),     Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	// +Z
            Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	// -X
            Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	// -Y
            Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz)     // -Z
        };
        
        
        Vec3f normals[24]={ Vec3f(1,0,0),   Vec3f(1,0,0),   Vec3f(1,0,0),   Vec3f(1,0,0),
                            Vec3f(0,1,0),	Vec3f(0,1,0),	Vec3f(0,1,0),	Vec3f(0,1,0),
                            Vec3f(0,0,1),	Vec3f(0,0,1),	Vec3f(0,0,1),	Vec3f(0,0,1),
                            Vec3f(-1,0,0),	Vec3f(-1,0,0),	Vec3f(-1,0,0),	Vec3f(-1,0,0),
                            Vec3f(0,-1,0),	Vec3f(0,-1,0),  Vec3f(0,-1,0),  Vec3f(0,-1,0),
                            Vec3f(0,0,-1),	Vec3f(0,0,-1),	Vec3f(0,0,-1),	Vec3f(0,0,-1)
                        };
        
        Color colors[24]={	Color::white(), Color::white(), Color::white(), Color::white(),
                            Color::white(), Color::white(), Color::white(), Color::white(),
                            Color::white(), Color::white(), Color::white(), Color::white(),
                            Color::white(), Color::white(), Color::white(), Color::white()
                        };
        
        uint32_t indices[6*6] = {   0, 1, 2, 0, 2, 3,
                                    4, 5, 6, 4, 6, 7,
                                    8, 9,10, 8, 10,11,
                                    12,13,14,12,14,15,
                                    16,17,18,16,18,19,
                                    20,21,22,20,22,23
                                };
        
        gl::VboMesh::Layout layout;
        layout.setStaticPositions();
        layout.setStaticIndices();
        layout.setStaticNormals();
        layout.setStaticColorsRGB();
        
        *vboMesh = gl::VboMesh( 24, 36, layout, GL_TRIANGLES );
        vboMesh->bufferPositions(std::vector<Vec3f>(vertices, vertices + sizeof(vertices)/sizeof(vertices[0])));
        vboMesh->bufferNormals(std::vector<Vec3f>(normals, normals + sizeof(normals)/sizeof(normals[0])));
        vboMesh->bufferColorsRGB(std::vector<Color>(colors, colors + sizeof(colors)/sizeof(colors[0])));
        vboMesh->bufferIndices(std::vector<uint32_t>(indices, indices + sizeof(indices)/sizeof(indices[0])));
    }
    
};

#endif
