#include "AppDelegate1.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>
#include "CScene.h"

USING_NS_CC;



const char* pModelName[5] = {"models/flower", "models/car", "models/moto", "models/door", "models/boy"};

const char* modelName = pModelName[4];

AppDelegate1::AppDelegate1() {

}

AppDelegate1::~AppDelegate1() 
{
}

bool AppDelegate1::applicationDidFinishLaunching() {
    // initialize director
    CCDirector* pDirector = CCDirector::sharedDirector();
    CCEGLView* pEGLView = CCEGLView::sharedOpenGLView();

    pDirector->setOpenGLView(pEGLView);
	
    // turn on display FPS
    pDirector->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    pDirector->setAnimationInterval(1.0 / 60);

	//set root node
    NSString*       contentPath;
    if (contentPath == nil)
    {
        contentPath = [[NSBundle mainBundle] pathForResource:@"Content" ofType:@""];
        if (contentPath == nil)
        {
            return false;
        }
    }
    const char* path = [contentPath UTF8String];
    if (*path == 0)
    {
        return 0;
    }
	pDirector->setRootNode( new CScene(contentPath) );

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate1::applicationDidEnterBackground() {
    CCDirector::sharedDirector()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate1::applicationWillEnterForeground() {
    CCDirector::sharedDirector()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
}
