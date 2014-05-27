//
//  DeferredRenderer.cpp
//  CinderDeferredRendering
//
//  Created by Anthony Scavarelli on 2012-12-31.
//
//

#include "DeferredRenderer.h"

vector<Light_Point*>* DeferredRenderer::getPointLightsRef(){
    return &mPointLights;
};

const int DeferredRenderer::getNumPointLights(){
    return mPointLights.size();
};

vector<Light_Spot*>* DeferredRenderer::getSpotLightsRef(){
    return &mSpotLights;
};

const int DeferredRenderer::getNumSpotLights(){
    return mSpotLights.size();
};

void DeferredRenderer::setup(   const boost::function<void(gl::GlslProg*)> renderShadowCastFunc,
                                const boost::function<void(gl::GlslProg*)> renderObjFunc,
                                const boost::function<void()> renderOverlayFunc,
                                const boost::function<void()> renderParticlesFunc,
                                Camera    *cam,
                                Vec2i     FBORes,
                                int       shadowMapRes,
                                BOOL      useSSAO,
                                BOOL      useShadows,
                                BOOL      useFXAA )
{
    //create cube VBO reference for lights
    getCubeVboMesh( &mCubeVBOMesh, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f) );
    getConeVboMesh( &mConeVBOMesh, Vec3f(0.0f, 1.0f, 0.0f), 1.0f, 0.5f, 10);
    getSphereVboMesh( &mSphereVBOMesh, Vec3f(0.0f, 0.0f, 0.0f), 0.5f, Vec2i(10,10));
    
    fRenderShadowCastersFunc = renderShadowCastFunc;
    fRenderNotShadowCastersFunc = renderObjFunc;
    fRenderOverlayFunc = renderOverlayFunc;
    fRenderParticlesFunc = renderParticlesFunc;
    mCam = cam;
    mFBOResolution = FBORes;
    mShadowMapRes = shadowMapRes;
    mUseSSAO = useSSAO;
    mUseShadows = useShadows;
    mUseFXAA = useFXAA;
    
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

Light_Point* DeferredRenderer::addPointLight(const Vec3f position, const Color color, const float intensity, const bool castsShadows, const bool visible)
{
    Light_Point *newLightP = new Light_Point( &mCubeVBOMesh, position, color, intensity, mShadowMapRes, (castsShadows && mUseShadows), visible );
    mPointLights.push_back( newLightP );
    return newLightP;
}

Light_Spot* DeferredRenderer::addSpotLight(const Vec3f position, const Vec3f target, const Color color, const bool castsShadows, const bool visible)
{
    Light_Spot *newLightP = new Light_Spot( &mConeVBOMesh, position, target, color, mShadowMapRes, (castsShadows && mUseShadows), visible );
    mSpotLights.push_back( newLightP );
    return newLightP;
}

void DeferredRenderer::prepareDeferredScene()
{
    //clear depth and color every frame
    glClearColor( 0.5f, 0.5f, 0.5f, 1 );
    glClearDepth(1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    gl::setMatrices( *mCam );
    renderDeferredFBO();
    
    if(mUseShadows) {
        createShadowMaps();
        renderShadowsToFBOs();
    }
    
    gl::setMatrices( *mCam );
    renderLightsToFBO();
    
    if(mUseSSAO) {
        renderSSAOToFBO();
    }
}

void DeferredRenderer::createShadowMaps()
{
    //render depth map cube
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    for(vector<Light_Point*>::iterator currPointLight = mPointLights.begin(); currPointLight != mPointLights.end(); ++currPointLight)
    {
        if (!(*currPointLight)->doesCastShadows()) {
            continue;
        }
        
        (*currPointLight)->mDepthFBO.bindFramebuffer();
        glClearDepth(1.0f);
        glClear( GL_DEPTH_BUFFER_BIT );
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glViewport(0, 0, mShadowMapRes, mShadowMapRes);
        
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf((*currPointLight)->mShadowCam.getProjectionMatrix());
        glMatrixMode(GL_MODELVIEW);
        
        for (size_t i = 0; i < 6; ++i) {
            (*currPointLight)->mShadowMap.bindDepthFB( i );
            glClear(GL_DEPTH_BUFFER_BIT);
            
            glLoadMatrixf(mLightFaceViewMatrices[i]);
            glMultMatrixf((*currPointLight)->mShadowCam.getModelViewMatrix());
            
            if (fRenderShadowCastersFunc) {
                fRenderShadowCastersFunc( NULL );
            }
        }
        
        (*currPointLight)->mDepthFBO.unbindFramebuffer();
    }
    
    //render point light
    for(vector<Light_Spot*>::iterator currSpotLight = mSpotLights.begin(); currSpotLight != mSpotLights.end(); ++currSpotLight)
    {
        if (!(*currSpotLight)->doesCastShadows()) {
            continue;
        }
        
        gl::enableDepthWrite();
        (*currSpotLight)->mDepthFBO.bindFramebuffer();
        glClear( GL_DEPTH_BUFFER_BIT );
//            glDrawBuffer(GL_NONE);
//            glReadBuffer(GL_NONE);
        glViewport(0, 0, mShadowMapRes, mShadowMapRes);
        
        gl::setMatrices((*currSpotLight)->mShadowCam);
        
        if (fRenderShadowCastersFunc) {
            fRenderShadowCastersFunc( NULL );
        }
        
        (*currSpotLight)->mDepthFBO.unbindFramebuffer();
    }
    glDisable(GL_CULL_FACE);
}

void DeferredRenderer::renderShadowsToFBOs()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP);
    for(vector<Light_Point*>::iterator currPointLight = mPointLights.begin(); currPointLight != mPointLights.end(); ++currPointLight) {
        if (!(*currPointLight)->doesCastShadows()) {
            continue;
        }
        
        (*currPointLight)->mShadowsFbo.bindFramebuffer();
        glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        gl::setViewport( (*currPointLight)->mShadowsFbo.getBounds() );
        glCullFace(GL_BACK); //don't need what we won't see
        gl::setMatrices( *mCam );
        
        mPointShadowShader.bind();
        (*currPointLight)->mShadowMap.bind(0); //the magic texture
        mPointShadowShader.uniform( "shadow", 0);
        mPointShadowShader.uniform( "light_position", mCam->getModelViewMatrix().transformPointAffine( (*currPointLight)->mShadowCam.getEyePoint() )); //conversion from world-space to camera-space (required here)
        mPointShadowShader.uniform( "camera_view_matrix_inv", mCam->getInverseModelViewMatrix());
        mPointShadowShader.uniform( "light_view_matrix", (*currPointLight)->mShadowCam.getModelViewMatrix());
        mPointShadowShader.uniform( "light_projection_matrix", (*currPointLight)->mShadowCam.getProjectionMatrix());
        drawScene();
        (*currPointLight)->mShadowMap.unbind();
        mPointShadowShader.unbind();
        
        (*currPointLight)->mShadowsFbo.unbindFramebuffer();
    }
    glDisable(GL_TEXTURE_CUBE_MAP);
    
    for(vector<Light_Spot*>::iterator currSpotLight = mSpotLights.begin(); currSpotLight != mSpotLights.end(); ++currSpotLight) {
        if (!(*currSpotLight)->doesCastShadows()) {
            continue;
        }
        
        (*currSpotLight)->mShadowsFbo.bindFramebuffer();
        glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        gl::setViewport( (*currSpotLight)->mShadowsFbo.getBounds() );
        glCullFace(GL_BACK); //don't need what we won't see
        gl::setMatrices( *mCam );
        
        //get shadow matrix transform
        Matrix44f shadowTransMatrix = (*currSpotLight)->mShadowCam.getProjectionMatrix();
        shadowTransMatrix *= (*currSpotLight)->mShadowCam.getModelViewMatrix();
        shadowTransMatrix *= mCam->getInverseModelViewMatrix();

        (*currSpotLight)->mDepthFBO.bindDepthTexture(0);
        mSpotShadowShader.bind();
        mSpotShadowShader.uniform( "depthTexture", 0 );
        mSpotShadowShader.uniform( "shadowTransMatrix", shadowTransMatrix );
        drawScene();
        mSpotShadowShader.unbind();
        (*currSpotLight)->mDepthFBO.unbindTexture();
        (*currSpotLight)->mShadowsFbo.unbindFramebuffer();
    }
    glDisable(GL_CULL_FACE);
    
    //render all shadow layers to one FBO
    mAllShadowsFBO.bindFramebuffer();
    gl::setViewport( mAllShadowsFBO.getBounds() );
    gl::setMatricesWindow( mAllShadowsFBO.getSize() ); //want textures to fill FBO
    glClearColor( 0.5f, 0.5f, 0.5f, 0.0 );
    glClearDepth(1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    gl::enableAlphaBlending();
    
    //draw point light shadows
    for(vector<Light_Point*>::iterator currPointLight = mPointLights.begin(); currPointLight != mPointLights.end(); ++currPointLight) {
        if ( !(*currPointLight)->doesCastShadows() ) {
            continue;
        }
        
        (*currPointLight)->mShadowsFbo.getTexture().bind();
        gl::drawSolidRect( Rectf( 0, (float)mAllShadowsFBO.getHeight(), (float)mAllShadowsFBO.getWidth(), 0) ); //this is different as we are not using shaders to color these quads (need to fit viewport)
        (*currPointLight)->mShadowsFbo.getTexture().unbind();
    }
    
    //draw spot light shadows
    for(vector<Light_Spot*>::iterator currSpotLight = mSpotLights.begin(); currSpotLight != mSpotLights.end(); ++currSpotLight) {
        if ( !(*currSpotLight)->doesCastShadows() ) {
            continue;
        }
        
        (*currSpotLight)->mShadowsFbo.getTexture().bind();
        gl::drawSolidRect( Rectf( 0, (float)mAllShadowsFBO.getHeight(), (float)mAllShadowsFBO.getWidth(), 0) ); //this is different as we are not using shaders to color these quads (need to fit viewport)
        (*currSpotLight)->mShadowsFbo.getTexture().unbind();
    }
    
    gl::disableAlphaBlending();
    mAllShadowsFBO.unbindFramebuffer();
}

void DeferredRenderer::renderDeferredFBO()
{
    //render out main scene to FBO
    mDeferredFBO.bindFramebuffer();
    gl::setViewport( mDeferredFBO.getBounds() );
    
    glClearColor( 0.5f, 0.5f, 0.5f, 1 );
    glClearDepth(1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    mDeferredShader.bind();
    mDeferredShader.uniform("projectionMatrix", mCam->getProjectionMatrix());
    mDeferredShader.uniform("diffuse", Vec3f(1.0f, 1.0f, 1.0f));
    mDeferredShader.uniform("specular", Vec3f(1.0f, 1.0f, 1.0f));
    mDeferredShader.uniform("emissive", Vec3f(1.0f, 1.0f, 1.0f));
    mDeferredShader.uniform("shininess", 10.0f);
    mDeferredShader.uniform("additiveSpecular", 10.0f);
    Matrix33f normalMatrix = mCam->getModelViewMatrix().subMatrix33(0, 0);
    normalMatrix.invert();
    normalMatrix.transpose();
    mDeferredShader.uniform("normalMatrix", normalMatrix ); //getInverse( object modelMatrix ).transpose()
    
    drawLightMeshes( &mDeferredShader, true );
    
    mDeferredShader.uniform("modelViewMatrix", mCam->getModelViewMatrix());
    
    if (fRenderShadowCastersFunc) {fRenderShadowCastersFunc(&mDeferredShader);}
    if (fRenderNotShadowCastersFunc) {fRenderNotShadowCastersFunc(&mDeferredShader);}
    
    mDeferredShader.unbind();
    mDeferredFBO.unbindFramebuffer();
}

void DeferredRenderer::renderSSAOToFBO()
{
    //render out main scene to FBO
    mSSAOMap.bindFramebuffer();
    gl::setViewport( mSSAOMap.getBounds() );
    gl::setMatricesWindow( mSSAOMap.getSize() ); //setting orthogonal view as rendering to a fullscreen quad
    
    glClearColor( 0.5f, 0.5f, 0.5f, 1 );
    glClearDepth(1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    mRandomNoise.bind(0);
    mDeferredFBO.getTexture(1).bind(1);
    mSSAOShader.bind();
    mSSAOShader.uniform("rnm", 0 );
    mSSAOShader.uniform("normalMap", 1 );
    
    gl::drawSolidRect( Rectf( 0.0f, 0.0f, (float)mSSAOMap.getWidth(), (float)mSSAOMap.getHeight()) );
    
    mSSAOShader.unbind();
    
    mDeferredFBO.getTexture(1).unbind(1);
    mRandomNoise.unbind(0);
    
    mSSAOMap.unbindFramebuffer();
}

void DeferredRenderer::renderLightsToFBO()
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

void DeferredRenderer::pingPongBlurSSAO()
{
    //--------- render horizontal blur first --------------
    mPingPongBlurH.bindFramebuffer();
    gl::setViewport( mPingPongBlurH.getBounds() );
    gl::setMatricesWindow( mPingPongBlurH.getSize() );
    glClearColor( 0.5f, 0.5f, 0.5f, 1 );
    glClearDepth(1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    mSSAOMap.getTexture().bind(0);
    mHBlurShader.bind();
    mHBlurShader.uniform("RTScene", 0);
    mHBlurShader.uniform("blurStep", 1.0f/mPingPongBlurH.getWidth()); //so that every "blur step" will equal one pixel width of this FBO
    gl::drawSolidRect( Rectf( 0.0f, 0.0f, (float)mPingPongBlurH.getWidth(), (float)mPingPongBlurH.getHeight()) );
    mHBlurShader.unbind();
    mSSAOMap.getTexture().unbind(0);
    mPingPongBlurH.unbindFramebuffer();
    
    //--------- now render vertical blur --------------
    mPingPongBlurV.bindFramebuffer();
    gl::setViewport( mPingPongBlurV.getBounds() );
    gl::setMatricesWindow( mPingPongBlurV.getSize() );
    glClearColor( 0.5f, 0.5f, 0.5f, 1 );
    glClearDepth(1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    mPingPongBlurH.getTexture().bind(0);
    mHBlurShader.bind();
    mHBlurShader.uniform("RTBlurH", 0);
    mHBlurShader.uniform("blurStep", 1.0f/mPingPongBlurH.getHeight()); //so that every "blur step" will equal one pixel height of this FBO
    gl::drawSolidRect( Rectf( 0.0f, 0.0f, (float)mPingPongBlurH.getWidth(), (float)mPingPongBlurH.getHeight()) );
    mHBlurShader.unbind();
    mPingPongBlurH.getTexture().unbind(0);
    mPingPongBlurV.unbindFramebuffer();
}

void DeferredRenderer::renderFullScreenQuad( const int renderType, const BOOL autoPrepareScene )
{
    renderQuad( renderType, Rectf( 0.0f, (float)getWindowHeight(), (float)getWindowWidth(), 0.0f), autoPrepareScene );
}

void DeferredRenderer::renderQuad( const int renderType, Rectf renderQuad, const BOOL autoPrepareScene )
{
    if( autoPrepareScene ) {
        prepareDeferredScene(); //prepare all FBOs for use
    }
    
    switch (renderType)
    {
        case SHOW_FINAL_VIEW: {
            
            if(mUseSSAO) {
                pingPongBlurSSAO();
            }
            
            if(fRenderOverlayFunc || fRenderParticlesFunc) {
                glDisable(GL_DEPTH_TEST);
            }
            
            if(fRenderOverlayFunc) {
                gl::enableAlphaBlending();
                
                mOverlayFBO.bindFramebuffer();
                glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
                glClearDepth(1.0f);
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                gl::setMatrices(*mCam);
                gl::setViewport( mOverlayFBO.getBounds() );
                
                fRenderOverlayFunc();
                
                mOverlayFBO.unbindFramebuffer();
            }
            
            if(fRenderParticlesFunc) {
                //copy deferred depth buffer to "particles" FBO so we can have transparency + occlusion
                glEnable(GL_DEPTH_TEST);    //want to read depth buffer
                glDepthMask(GL_TRUE);       //want to write to depth buffer
                glBindFramebuffer(GL_READ_FRAMEBUFFER, mDeferredFBO.getId());
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mParticlesFBO.getId());
                
                //glReadBuffer(GL_DEPTH_ATTACHMENT);
                //glDrawBuffer(GL_DEPTH_ATTACHMENT);
                
                glBlitFramebuffer(  0, 0, mDeferredFBO.getWidth(), mDeferredFBO.getHeight(),
                                  0, 0, mParticlesFBO.getWidth(), mParticlesFBO.getHeight(),
                                  GL_DEPTH_BUFFER_BIT, GL_NEAREST );
                
                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                
                //draw to FBO
                glDepthMask(GL_FALSE);  //don't want to write to depth buffer / prevent overriding
                mParticlesFBO.bindFramebuffer();
                glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
                glClear( GL_COLOR_BUFFER_BIT );
                gl::setMatrices(*mCam);
                gl::setViewport( mParticlesFBO.getBounds() );
                
                fRenderParticlesFunc();
                
                mParticlesFBO.unbindFramebuffer();
                glDepthMask(GL_TRUE);
            }
            
            if(fRenderOverlayFunc || fRenderParticlesFunc) {
                glEnable(GL_DEPTH_TEST);
                gl::disableAlphaBlending();
            }
            
            mFinalSSFBO.bindFramebuffer();
            glClearColor( 0.5f, 0.5f, 0.5f, 1 );
            glClearDepth(1.0f);
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            gl::setViewport( mFinalSSFBO.getBounds() );
            gl::setMatricesWindow( mFinalSSFBO.getSize() );
            if(mUseSSAO) {
                mPingPongBlurV.getTexture().bind(0);
            }
            if(mUseShadows) {
                mAllShadowsFBO.getTexture().bind(1);
            }
            mLightGlowFBO.getTexture().bind(2);
            mBasicBlender.bind();
            mBasicBlender.uniform("ssaoTex", 0 );
            mBasicBlender.uniform("shadowsTex", 1 );
            mBasicBlender.uniform("baseTex", 2 );
            mBasicBlender.uniform("useSSAO", mUseSSAO );
            mBasicBlender.uniform("useShadows", mUseShadows );
            mBasicBlender.uniform("onlySSAO", false );
            gl::drawSolidRect( Rectf( 0.0f, (float)mFinalSSFBO.getHeight(), (float)mFinalSSFBO.getWidth(), 0.0f) );
            mBasicBlender.unbind();
            mLightGlowFBO.getTexture().unbind(2);
            if(mUseShadows) {
                mAllShadowsFBO.getTexture().unbind(1);
            }
            if(mUseSSAO) {
                mPingPongBlurV.getTexture().unbind(0);
            }
            
            if(fRenderOverlayFunc || fRenderParticlesFunc) {
                gl::enableAlphaBlending();
            }
            
            if(fRenderParticlesFunc) {
                mParticlesFBO.getTexture().bind(0);
                gl::drawSolidRect( Rectf( 0.0f, (float)mFinalSSFBO.getHeight(), (float)mFinalSSFBO.getWidth(), 0.0f) );
                mParticlesFBO.getTexture().unbind(0);
            }
            
            if(fRenderOverlayFunc) {
                mOverlayFBO.getTexture().bind();
                gl::drawSolidRect( Rectf( 0.0f, (float)mFinalSSFBO.getHeight(), (float)mFinalSSFBO.getWidth(), 0.0f) );
                mOverlayFBO.getTexture().unbind();
            }
            
            if(fRenderOverlayFunc || fRenderParticlesFunc) {
                gl::disableAlphaBlending();
            }
            
            mFinalSSFBO.unbindFramebuffer();
            
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
            mFinalSSFBO.getTexture().bind(0);
            
            if(mUseFXAA) {
                mFXAAShader.bind();
                mFXAAShader.uniform("buf0", 0);
                mFXAAShader.uniform("frameBufSize", Vec2f((float)mFinalSSFBO.getWidth(), (float)mFinalSSFBO.getHeight()));
            }
            gl::drawSolidRect( renderQuad );
            if(mUseFXAA) {
                mFXAAShader.unbind();
            }
            mFinalSSFBO.getTexture().unbind(0);
        }
            break;
        case SHOW_DIFFUSE_VIEW: {
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
            mDeferredFBO.getTexture(0).bind(0);
            gl::drawSolidRect( renderQuad );
            mDeferredFBO.getTexture(0).unbind(0);
        }
            break;
        case SHOW_DEPTH_VIEW: {
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
            mDeferredFBO.getTexture(1).bind(0);
            mAlphaToRBG.bind();
            mAlphaToRBG.uniform("alphaTex", 0);
            gl::drawSolidRect( renderQuad );
            mAlphaToRBG.unbind();
            mDeferredFBO.getTexture(1).unbind(0);
        }
            break;
        case SHOW_POSITION_VIEW: {
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
            mDeferredFBO.getTexture(2).bind(0);
            gl::drawSolidRect( renderQuad );
            mDeferredFBO.getTexture(2).unbind(0);
        }
            break;
        case SHOW_ATTRIBUTE_VIEW: {
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
            mDeferredFBO.getTexture(3).bind(0);
            gl::drawSolidRect( renderQuad );
            mDeferredFBO.getTexture(3).unbind(0);
        }
            break;
            
        case SHOW_NORMALMAP_VIEW: {
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
            mDeferredFBO.getTexture(1).bind(0);
            gl::drawSolidRect( renderQuad );
            mDeferredFBO.getTexture(1).unbind(0);
        }
            break;
        case SHOW_SSAO_VIEW: {
            if(mUseSSAO) {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mSSAOMap.getTexture().bind(0);
                mBasicBlender.bind();
                mBasicBlender.uniform("ssaoTex", 0 );
                mBasicBlender.uniform("shadowsTex", 0 );    //just binding same one so only it shows....
                mBasicBlender.uniform("baseTex", 0 );       //just binding same one so only it shows....
                mBasicBlender.uniform("useSSAO", true );
                mBasicBlender.uniform("useShadows", false );
                mBasicBlender.uniform("onlySSAO", true );
                gl::drawSolidRect( renderQuad );
                mBasicBlender.unbind();
                mSSAOMap.getTexture().unbind(0);
            }
        }
            break;
        case SHOW_SSAO_BLURRED_VIEW: {
            if(mUseSSAO) {
                pingPongBlurSSAO();
                
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mPingPongBlurV.getTexture().bind(0);
                gl::drawSolidRect( renderQuad );
                mPingPongBlurV.getTexture().unbind(0);
            }
        }
            break;
        case SHOW_LIGHT_VIEW: {
            gl::setViewport( getWindowBounds() );
            gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
            mLightGlowFBO.getTexture().bind(0);
            gl::drawSolidRect( renderQuad );
            mLightGlowFBO.getTexture().unbind(0);
        }
            break;
        case SHOW_SHADOWS_VIEW: {
            if(mUseShadows) {
                gl::setViewport( getWindowBounds() );
                gl::setMatricesWindow( getWindowSize() ); //want textures to fill screen
                mAllShadowsFBO.getTexture().bind(0);
                gl::drawSolidRect( renderQuad );
                mAllShadowsFBO.getTexture().unbind(0);
            }
        }
            break;
    }
}

void DeferredRenderer::drawLightMeshes(gl::GlslProg* shader, BOOL deferShaderUsed )
{
    //point lights
    for(vector<Light_Point*>::iterator currPointLight = mPointLights.begin(); currPointLight != mPointLights.end(); ++currPointLight) {
        if ( shader != NULL && !deferShaderUsed ) {
            mLightPointShader.uniform("viewHeight", mDeferredFBO.getHeight());
            mLightPointShader.uniform("viewWidth", mDeferredFBO.getWidth());
            mLightPointShader.uniform("lightColor", (*currPointLight)->getColor());
            mLightPointShader.uniform("lightRadius", (*currPointLight)->getLightMaskRadius());
            mLightPointShader.uniform("lightIntensity", (*currPointLight)->getIntensity());
//                Matrix44f lightPosMat = Matrix44f::identity();
//                lightPosMat.setTranslate((*currCube)->getPos());
//                Matrix44f camPosMat = Matrix44f::identity();
//                camPosMat.setTranslate(mCam->getEyePoint());
//                lightPosMat = lightPosMat * camPosMat.inverted();
            mLightPointShader.uniform("lightPositionVS", mCam->getModelViewMatrix().transformPointAffine( (*currPointLight)->getPos() ));
            mLightPointShader.uniform("modelViewMatrix", mCam->getModelViewMatrix() * (*currPointLight)->modelMatrixAOE );
        
//                console() << mCam->getModelViewMatrix() << "\n";
//                console() << mCam->getProjectionMatrix() << "\n";
//                console() << mCam->getModelViewMatrix() * (*currCube)->modelMatrixAOE << "\n";
//                console() << mCam->getModelViewMatrix().transformPointAffine( (*currCube)->getPos() ) << "\n";
            
//                shader->uniform("lightPos", mCam->getModelViewMatrix().transformPointAffine( (*currCube)->getPos() ) ); //pass light pos to pixel shader
//                shader->uniform("lightCol", (*currCube)->getColor()); //pass light color (magnitude is power) to pixel shader
//                shader->uniform("dist", (*currCube)->getAOEDist()); //pass the light's area of effect radius to pixel shader
            (*currPointLight)->renderProxyAOE(); //render the proxy shape
        }
        else if( shader != NULL && deferShaderUsed ) {
            mDeferredShader.uniform("modelViewMatrix", mCam->getModelViewMatrix() * (*currPointLight)->modelMatrix);
//                Matrix44f modelMatrix = Matrix44f::identity();
//                modelMatrix.createTranslation((*currCube)->getPos());
//                mDeferredShader.uniform("modelMatrix", modelMatrix);  //matrix of object position, rotation, scale
            (*currPointLight)->renderProxy(); //render the proxy shape
        }
        
    }
}

void DeferredRenderer::drawScene()
{
    if(fRenderShadowCastersFunc) {
        fRenderShadowCastersFunc(NULL);
    }
    
    if(fRenderNotShadowCastersFunc) {
        fRenderNotShadowCastersFunc(NULL);
    }
    
    drawLightMeshes(NULL, false); //!!need to relook at this for shadow-mapping as I need to pass matrices
}

void DeferredRenderer::renderLights()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE); //set blend function
    glEnable(GL_CULL_FACE); //cull front faces
    glCullFace(GL_FRONT);
    glDisable(GL_DEPTH_TEST); //disable depth testing
    glDepthMask(false);
    
    mLightPointShader.bind(); //bind point light pixel shader
    mDeferredFBO.getTexture(0).bind(0); //bind color tex
    mDeferredFBO.getTexture(1).bind(1); //bind normal/depth tex
    mLightPointShader.uniform("projectionMatrix", mCam->getProjectionMatrix());
    mLightPointShader.uniform("samplerColor", 0);
    mLightPointShader.uniform("samplerNormalDepth", 1);
    mLightPointShader.uniform("matProjInverse", mCam->getProjectionMatrix().inverted());
    
    drawLightMeshes( &mLightPointShader, false );
    
    mLightPointShader.unbind(); //unbind and reset everything to desired values
    mDeferredFBO.getTexture(0).unbind(0); //bind color tex
    mDeferredFBO.getTexture(1).unbind(1); //bind normal/depth tex
    
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);
    glDisable(GL_BLEND);
}

void DeferredRenderer::initTextures()
{
    mRandomNoise = gl::Texture( loadImage( loadResource( RES_TEX_NOISE_SAMPLER ) ) ); //noise texture required for SSAO calculations
}

void DeferredRenderer::initShaders()
{
    mDeferredShader		= gl::GlslProg( loadResource( RES_GLSL_DEFER_VERT ), loadResource( RES_GLSL_DEFER_FRAG ) );
    mBasicBlender		= gl::GlslProg( loadResource( RES_GLSL_BASIC_BLENDER_VERT ), loadResource( RES_GLSL_BASIC_BLENDER_FRAG ) );
    
    if(mUseSSAO) {
        mSSAOShader			= gl::GlslProg( loadResource( RES_GLSL_SSAO_VERT ), loadResource( RES_GLSL_SSAO_FRAG ) );
        mHBlurShader		= gl::GlslProg( loadResource( RES_GLSL_BLUR_H_VERT ), loadResource( RES_GLSL_BLUR_H_FRAG ) );
        mVBlurShader		= gl::GlslProg( loadResource( RES_GLSL_BLUR_V_VERT ), loadResource( RES_GLSL_BLUR_V_FRAG ) );
    }
    
    mLightPointShader		= gl::GlslProg( loadResource( RES_GLSL_LIGHT_POINT_VERT ), loadResource( RES_GLSL_LIGHT_POINT_FRAG ) );
    mAlphaToRBG         = gl::GlslProg( loadResource( RES_GLSL_ALPHA_RGB_VERT ), loadResource( RES_GLSL_ALPHA_RGB_FRAG ) );
    
    if (mUseShadows) {
        mPointShadowShader   = gl::GlslProg( loadResource( RES_GLSL_POINTSHADOW_VERT ), loadResource( RES_GLSL_POINTSHADOW_FRAG ) );
        mSpotShadowShader   = gl::GlslProg( loadResource( RES_GLSL_SPOTSHADOW_VERT ), loadResource( RES_GLSL_SPOTSHADOW_FRAG ) );
    }
    
    mFXAAShader			= gl::GlslProg( loadResource( RES_GLSL_FXAA_VERT ), loadResource( RES_GLSL_FXAA_FRAG ) );
}

void DeferredRenderer::initFBOs()
{
    //this FBO will capture normals, depth, and base diffuse in one render pass (as opposed to three)
    gl::Fbo::Format deferredFBO;
    deferredFBO.enableDepthBuffer();
    deferredFBO.setDepthInternalFormat( GL_DEPTH_COMPONENT24 ); //want fbo to have precision depth map as well
    deferredFBO.setColorInternalFormat( GL_RGBA32F_ARB );
    deferredFBO.enableColorBuffer( true, 2 ); // create an FBO with two color attachments (basic diffuse and normal/depth view, get position from depth)
    //deferredFBO.setSamples(4);
    
    gl::Fbo::Format basicFormat;
    basicFormat.enableDepthBuffer(false); //don't need depth "layers"
    
    gl::Fbo::Format basicDepthFormat;
    basicDepthFormat.enableDepthBuffer(true); //don't need depth "layers"
    basicDepthFormat.setDepthInternalFormat( GL_DEPTH_COMPONENT24 ); //want fbo to have same precision as deferred
    
    //init screen space render
    mDeferredFBO	= gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   deferredFBO );
    mLightGlowFBO   = gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicFormat );
    
    if(mUseSSAO) {
        mPingPongBlurH	= gl::Fbo( mFBOResolution.x/2,  mFBOResolution.y/2, basicFormat ); //don't need as high res on ssao as it will be blurred anyhow ...
        mPingPongBlurV	= gl::Fbo( mFBOResolution.x/2,  mFBOResolution.y/2, basicFormat );
        mSSAOMap		= gl::Fbo( mFBOResolution.x/2,  mFBOResolution.y/2, basicFormat );
    }
    
    if (mUseShadows) {
        mAllShadowsFBO  = gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicFormat );
    }
    
    if(fRenderOverlayFunc) {
        mOverlayFBO     = gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicFormat );
    }
    
    if(fRenderParticlesFunc) {
        mParticlesFBO   = gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicDepthFormat );
    }
    
    mFinalSSFBO		= gl::Fbo( mFBOResolution.x,    mFBOResolution.y,   basicFormat );
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
}

#pragma mark - static VBO primitive functions

void DeferredRenderer::getCubeVboMesh( gl::VboMesh *vboMesh, const Vec3f &c, const Vec3f &size )
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
    
    *vboMesh = gl::VboMesh( 24, 36, layout, GL_TRIANGLES );
    vboMesh->bufferPositions(std::vector<Vec3f>(vertices, vertices + sizeof(vertices)/sizeof(vertices[0])));
    vboMesh->bufferNormals(std::vector<Vec3f>(normals, normals + sizeof(normals)/sizeof(normals[0])));
    vboMesh->bufferIndices(std::vector<uint32_t>(indices, indices + sizeof(indices)/sizeof(indices[0])));
}

//modfied from Stephen Schieberl's MeshHelper class https://github.com/bantherewind/Cinder-MeshHelper
void DeferredRenderer::getSphereVboMesh( gl::VboMesh *vboMesh, const Vec3f &center, const float radius, const Vec2i resolution )
{
    vector<uint32_t> indices;
    vector<Vec3f> normals;
    vector<Vec3f> positions;
    
    float step = (float)M_PI / (float)resolution.y;
    float delta = ((float)M_PI * 2.0f) / (float)resolution.x;
    
    int32_t p = 0;
    for ( float phi = 0.0f; p <= resolution.y; p++, phi += step ) {
        int32_t t = 0;
        
        uint32_t a = (uint32_t)( ( p + 0 ) * resolution.x );
        uint32_t b = (uint32_t)( ( p + 1 ) * resolution.x );
        
        for ( float theta = delta; t < resolution.x; t++, theta += delta ) {
            float sinPhi = math<float>::sin( phi );
            float x = sinPhi * math<float>::cos( theta );
            float y = sinPhi * math<float>::sin( theta );
            float z = -math<float>::cos( phi );
            Vec3f position( x, y, z );
            position = (position * radius) + center;
            Vec3f normal = position.normalized();
            
            normals.push_back( normal );
            positions.push_back( position );
            
            uint32_t n = (uint32_t)( t + 1 >= resolution.x ? 0 : t + 1 );
            indices.push_back( a + t );
            indices.push_back( b + t );
            indices.push_back( a + n );
            indices.push_back( a + n );
            indices.push_back( b + t );
            indices.push_back( b + n );
        }
    }
    
    for ( vector<uint32_t>::iterator iter = indices.begin(); iter != indices.end(); ) {
        if ( *iter < positions.size() ) {
            ++iter;
        } else {
            iter = indices.erase( iter );
        }
    }
    
    gl::VboMesh::Layout layout;
    layout.setStaticPositions();
    layout.setStaticIndices();
    layout.setStaticNormals();
    
    *vboMesh = gl::VboMesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
    vboMesh->bufferPositions( positions );
    vboMesh->bufferNormals( normals );
    vboMesh->bufferIndices( indices );
    
    indices.clear();
    normals.clear();
    positions.clear();
}

//ogre3D implementation
void DeferredRenderer::getConeVboMesh( gl::VboMesh *vboMesh, const Vec3f &pointPos, const float &coneHeight, const float &coneRadius, const int numSegments )
{
    vector<uint32_t> indices;
    vector<Vec3f> normals;
    vector<Vec3f> positions;
    
    //Positions : cone head and base
    positions.push_back( pointPos + Vec3f(0.0f, 0.0f, 0.0f) );
    normals.push_back( Vec3f(0, 1, 0) );
    
    //Base :
    Vec3f basePoint = Vec3f( pointPos.x, -coneHeight, pointPos.z );
    float fDeltaBaseAngle = (2 * M_PI) / numSegments;
    for (int i=0; i<numSegments; i++)
    {
        float angle = i * fDeltaBaseAngle;
        Vec3f vertPos = pointPos + Vec3f(coneRadius * cosf(angle), -coneHeight, coneRadius * sinf(angle));
        positions.push_back( vertPos );
        normals.push_back( (vertPos - basePoint).normalized() );
    }
    
    //Indices :
    //Cone head to vertices
    for (int i=0; i<numSegments; i++)
    {
        indices.push_back( 0 );
        indices.push_back( (i%numSegments) + 1 );
        indices.push_back( ((i+1)%numSegments) + 1 );
    }
    //Cone base
    for (int i=0; i<numSegments-2; i++)
    {
        indices.push_back( 1 );
        indices.push_back( i+3 );
        indices.push_back( i+2 );
    }
    
    gl::VboMesh::Layout layout;
    layout.setStaticPositions();
    layout.setStaticIndices();
    layout.setStaticNormals();
    
    *vboMesh = gl::VboMesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
    vboMesh->bufferPositions( positions );
    vboMesh->bufferNormals( normals );
    vboMesh->bufferIndices( indices );
    
    indices.clear();
    normals.clear();
    positions.clear();
}