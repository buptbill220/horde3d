/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __COCOS2D_H__
#define __COCOS2D_H__

// 0x00 HI ME LO
// 00   02 01 00
#define COCOS2D_VERSION 0x00020100

//
// all cocos2d include files
//

#include "CCDirector.h"

// platform
#include "platform/CCDevice.h"
#include "platform/CCCommon.h"
#include "platform/CCPlatformConfig.h"
#include "platform/CCPlatformMacros.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    #include "platform/ios/CCAccelerometer.h"
    #include "platform/ios/CCApplication.h"
    #include "platform/ios/CCEGLView.h"
    #include "platform/ios/CCGL.h"
    #include "platform/ios/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    #include "platform/android/CCAccelerometer.h"
    #include "platform/android/CCApplication.h"
    #include "platform/android/CCEGLView.h"
    #include "platform/android/CCGL.h"
    #include "platform/android/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#if (CC_TARGET_PLATFORM == CC_PLATFORM_BLACKBERRY)
    #include "platform/blackberry/CCAccelerometer.h"
    #include "platform/blackberry/CCApplication.h"
    #include "platform/blackberry/CCEGLView.h"
    #include "platform/blackberry/CCGL.h"
    #include "platform/blackberry/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_BLACKBERRY

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	#include "platform/win32/CCAccelerometer.h"
	#include "platform/win32/CCApplication.h"
	#include "platform/win32/CCEGLView.h"
	#include "platform/win32/CCGL.h"
	#include "platform/win32/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
	#include "platform/mac/CCAccelerometer.h"
	#include "platform/mac/CCApplication.h"
	#include "platform/mac/CCEGLView.h"
	#include "platform/mac/CCGL.h"
	#include "platform/mac/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_MAC

#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	#include "platform/linux/CCAccelerometer.h"
	#include "platform/linux/CCApplication.h"
	#include "platform/linux/CCEGLView.h"
	#include "platform/linux/CCGL.h"
	#include "platform/linux/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_LINUX

// MARMALADE CHANGE
// Added for Marmalade support
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MARMALADE)
	#include "platform/Marmalade/CCAccelerometer.h"
	#include "platform/Marmalade/CCApplication.h"
	#include "platform/Marmalade/CCEGLView.h"
	#include "platform/Marmalade/CCGL.h"
	#include "platform/Marmalade/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_LINUX

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NACL)
    #include "platform/nacl/CCAccelerometer.h"
    #include "platform/nacl/CCApplication.h"
    #include "platform/nacl/CCEGLView.h"
    #include "platform/nacl/CCGL.h"
    #include "platform/nacl/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#if (CC_TARGET_PLATFORM == CC_PLATFORM_EMSCRIPTEN)
    #include "platform/emscripten/CCAccelerometer.h"
    #include "platform/emscripten/CCApplication.h"
    #include "platform/emscripten/CCEGLView.h"
    #include "platform/emscripten/CCGL.h"
    #include "platform/emscripten/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_EMSCRIPTEN

#if (CC_TARGET_PLATFORM == CC_PLATFORM_TIZEN)
    #include "platform/tizen/CCAccelerometer.h"
    #include "platform/tizen/CCApplication.h"
    #include "platform/tizen/CCEGLView.h"
    #include "platform/tizen/CCGL.h"
    #include "platform/tizen/CCStdC.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_TIZEN

#endif // __COCOS2D_H__
