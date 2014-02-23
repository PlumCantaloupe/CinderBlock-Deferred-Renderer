#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CinderMath.h"

#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/lambda/lambda.hpp"

#include "DeferredRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class _TBOX_PREFIX_App : public AppNative {
public:
    void prepareSettings(Settings *settings);
	void setup();
    void keyDown( KeyEvent event );
	void update();
	void draw();
    
    CameraPersp mCam;
    
    //deferred render functionality
    DeferredRenderer    mDeferredRenderer;
    int                 mRenderView;
    void drawShadowCasters(gl::GlslProg* deferShader) const;
    void drawNonShadowCasters(gl::GlslProg* deferShader) const;
    void drawOverlay() const;
    void drawDepthParticles() const;
};

void _TBOX_PREFIX_App::prepareSettings(Settings *settings)
{
    settings->setFrameRate(60.0f);
    settings->setWindowSize(1024, 512);
}

void _TBOX_PREFIX_App::setup()
{
    //set up camera
	mCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1f, 10000.0f );
    mCam.lookAt(Vec3f( 1.5f, 7.5f, 1.5f ), Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
    mCam.setCenterOfInterestPoint(Vec3f::zero());
    
    //create functions pointers to send to deferred renderer
    boost::function<void(gl::GlslProg*)> fRenderShadowCastersFunc = boost::bind( &_TBOX_PREFIX_App::drawShadowCasters, this, boost::lambda::_1 );
    boost::function<void(gl::GlslProg*)> fRenderNotShadowCastersFunc = boost::bind( &_TBOX_PREFIX_App::drawNonShadowCasters, this,  boost::lambda::_1 );
    boost::function<void(void)> fRenderOverlayFunc = boost::bind( &_TBOX_PREFIX_App::drawOverlay, this );
    boost::function<void(void)> fRenderParticlesFunc = boost::bind( &_TBOX_PREFIX_App::drawDepthParticles, this );
    
    mRenderView = DeferredRenderer::SHOW_FINAL_VIEW;
    mDeferredRenderer.setup( fRenderShadowCastersFunc, fRenderNotShadowCastersFunc, NULL, NULL, &mCam, Vec2i(1024, 512), 1024, true, true);
    mDeferredRenderer.addCubeLight( Vec3f(-2.5f, 3.0f, 0.0f), Color(0.10f, 0.69f, 0.93f) * LIGHT_BRIGHTNESS_DEFAULT * 0.3f, true, false);      //blue
    mDeferredRenderer.addCubeLight( Vec3f(2.5f, 3.0f, 0.0f), Color(0.94f, 0.15f, 0.23f) * LIGHT_BRIGHTNESS_DEFAULT * 0.3f, false, false);      //red
}

void _TBOX_PREFIX_App::keyDown( KeyEvent event )
{
    //toggle through deferred render modes using - and + keys
    switch (event.getCode()) {
        case KeyEvent::KEY_EQUALS:
        {
            mRenderView++;
            if(mRenderView>(DeferredRenderer::NUM_RENDER_VIEWS - 1)) {
                mRenderView = 0;
            }
            
            console() << mRenderView << "\n";
        }
            break;
        case KeyEvent::KEY_KP_MINUS:
        {
            mRenderView--;
            if(mRenderView<0) {
                mRenderView = DeferredRenderer::NUM_RENDER_VIEWS - 1;
            }
            
            console() << mRenderView << "\n";
        }
            break;
        default:
            break;
    }
}

void _TBOX_PREFIX_App::update()
{
    //moving some lights for effect
    int counter = 0;
    for( vector<Light_PS*>::iterator lightIter = mDeferredRenderer.mCubeLights.begin(); lightIter != mDeferredRenderer.mCubeLights.end(); lightIter++ ) {
        (*lightIter)->setPos(Vec3f( math<float>::sin(getElapsedSeconds() + (M_PI * counter)) * 3.0f, 3.0f, math<float>::cos(getElapsedSeconds() + (M_PI * counter)) * 3.0f ));
        counter++;
    }
}

void _TBOX_PREFIX_App::draw()
{
    mDeferredRenderer.renderFullScreenQuad(mRenderView);
}


void _TBOX_PREFIX_App::drawShadowCasters(gl::GlslProg* deferShader) const
{
    gl::drawSphere(Vec3f::zero(), 1.0f, 30);
}

void _TBOX_PREFIX_App::drawNonShadowCasters(gl::GlslProg* deferShader) const
{
    //a plane to capture shadows (though it won't cast any itself)
    int size = 3000;
    glColor3ub(255, 255, 255);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3i( size, -2,-size);
    glVertex3i(-size, -2,-size);
    glVertex3i(-size, -2, size);
    glVertex3i( size, -2, size);
    glEnd();
}

void _TBOX_PREFIX_App::drawOverlay() const
{}

void _TBOX_PREFIX_App::drawDepthParticles() const
{}

CINDER_APP_NATIVE( _TBOX_PREFIX_App, RendererGl )
