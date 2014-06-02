//
//  DeferredRenderer.h
//  CinderDeferredRendering
//
//  Created by Anthony Scavarelli on 2012-12-31.
//
//

/*
 TODO:
 - Platform agnostic
 */

#pragma once

#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"

#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/lambda/lambda.hpp"

#include "Lights.h"
#include "DeferredModel.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DeferredRenderer
{
public:
    enum {
        FBO_DEFERRED,
        FBO_SSAO,
        FBO_PINGPONG_H,
        FBO_PINGPONG_V,
        FBO_LIGHTS,
        FBO_SHADOWS,
        FBO_COMPOSITED,
        FBO_OVERLAY,
        FBO_PARTICLES
    };
    
    boost::function<void(int, gl::GlslProg*)> fRenderShadowCastersFunc;
    boost::function<void(int, gl::GlslProg*)> fRenderNotShadowCastersFunc;
    boost::function<void()> fRenderOverlayFunc;
    boost::function<void()> fRenderParticlesFunc;
    Camera              *mCam;
    CameraOrtho         mOrthoCam;
    
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
    gl::Fbo				mParticlesFBO;
    
	gl::GlslProg		mPointShadowShader;
    gl::GlslProg		mSpotShadowShader;
	
    gl::GlslProg		mSSAOShader;
    gl::GlslProg		mDeferredShader;
    gl::GlslProg		mBasicBlender;
    gl::GlslProg		mHBlurShader;
    gl::GlslProg		mVBlurShader;
    gl::GlslProg		mLightPointShader;
    gl::GlslProg		mLightSpotShader;
    gl::GlslProg		mAlphaToRBG;
	gl::GlslProg		mFXAAShader;
    gl::GlslProg		mDepthWriteShader;
    
    vector<Light_Point*>    mPointLights;
    vector<Light_Spot*>     mSpotLights;
    
    Vec2i               mFBOResolution;
    int                 mShadowMapRes;
    
    gl::VboMesh         mCubeVBOMesh;    //pass cube Vbo to all point lights to save on draw calls
    gl::VboMesh         mConeVBOMesh;    //pass cube Vbo to all spot lights to save on draw calls
    gl::VboMesh         mSphereVBOMesh;
    gl::VboMesh         mPlaneVBOMesh;
    
    enum
    {
        SHOW_FINAL_VIEW,
        SHOW_DIFFUSE_VIEW,
        SHOW_NORMALMAP_VIEW,
        SHOW_DEPTH_VIEW,
        SHOW_SSAO_VIEW,
        SHOW_SSAO_BLURRED_VIEW,
        SHOW_LIGHT_VIEW,
        SHOW_SHADOWS_VIEW,
        NUM_RENDER_VIEWS
    };
    
protected:
    int mDeferFlags;
    
public:
    //using bit-shifting so multiple flags can be set simultaneously
    enum {
        SHADOWS_ENABLED_FLAG    = (1 << 0),
        SSAO_ENABLED_FLAG       = (1 << 1),
        FXAA_ENABLED_FLAG       = (1 << 2)
    };
    
    enum {
        SHADER_TYPE_NONE,
        SHADER_TYPE_DEFERRED,
        SHADER_TYPE_LIGHT,
        SHADER_TYPE_SHADOW,
        SHADER_TYPE_DEPTH,
        NUM_SHADER_TYPES
    };
    
    vector<Light_Point*>* getPointLightsRef();
    const int getNumPointLights();
    vector<Light_Spot*>* getSpotLightsRef();
    const int getNumSpotLights();
    
    void setup( const boost::function<void(int, gl::GlslProg*)> renderShadowCastFunc,
               const boost::function<void(int, gl::GlslProg*)> renderObjFunc,
               const boost::function<void()> renderOverlayFunc,
               const boost::function<void()> renderParticlesFunc,
               Camera    *cam,
               Vec2i     FBORes = Vec2i(512, 512),
               int       shadowMapRes = 512,
               int       deferFlags = SHADOWS_ENABLED_FLAG | SSAO_ENABLED_FLAG | FXAA_ENABLED_FLAG );
    
    Light_Point* addPointLight(const Vec3f position, const Color color, const float intensity, const bool castsShadows = false, const bool visible = false);
    Light_Spot* addSpotLight(const Vec3f position, const Vec3f target, const Color color, const float intensity, const float lightAngle, const bool castsShadows, const bool visible);
    void prepareDeferredScene();
    void createShadowMaps();
    void renderShadowsToFBOs();
    void renderDeferredFBO();
    void renderSSAOToFBO();
    void renderLightsToFBO();
    void pingPongBlurSSAO();
    void renderFullScreenQuad( const int renderType, const BOOL autoPrepareScene = true  );
    void renderQuad( const int renderType, Rectf renderQuad, const BOOL autoPrepareScene = true  );
    void drawLightPointMeshes(  int shaderType = SHADER_TYPE_NONE, gl::GlslProg* shader = NULL );
    void drawLightSpotMeshes(  int shaderType = SHADER_TYPE_NONE, gl::GlslProg* shader = NULL );
    void drawScene( int shaderType, gl::GlslProg *shader );
    void renderLights();
    void initTextures();
    void initShaders();
    void initFBOs();
};