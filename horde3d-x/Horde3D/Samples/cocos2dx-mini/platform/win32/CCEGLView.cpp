/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

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

#include "CCEGLView.h"
#include "CCDirector.h"
#include "CCApplication.h"

NS_CC_BEGIN

Key::Code VirtualKeyCodeToCocos2D(WPARAM VirtualKey, LPARAM Flags);

#if(_MSC_VER >= 1600) // Visual Studio 2010 or higher version.
// Windows Touch define
#define MOUSEEVENTF_FROMTOUCH 0xFF515700

// Windows Touch functions
// Workaround to be able tu run app on Windows XP
typedef WINUSERAPI BOOL (WINAPI *RegisterTouchWindowFn)(_In_ HWND hwnd, _In_ ULONG ulFlags);
typedef WINUSERAPI BOOL (WINAPI *UnregisterTouchWindowFn)(_In_ HWND hwnd);
typedef WINUSERAPI LPARAM (WINAPI *GetMessageExtraInfoFn)(VOID);
typedef WINUSERAPI BOOL (WINAPI *GetTouchInputInfoFn)(_In_ HTOUCHINPUT hTouchInput, _In_ UINT cInputs, __out_ecount(cInputs) PTOUCHINPUT pInputs, _In_ int cbSize);
typedef WINUSERAPI BOOL (WINAPI *CloseTouchInputHandleFn)(_In_ HTOUCHINPUT hTouchInput);

static RegisterTouchWindowFn s_pfRegisterTouchWindowFunction = NULL;
static UnregisterTouchWindowFn s_pfUnregisterTouchWindowFunction = NULL;
static GetMessageExtraInfoFn s_pfGetMessageExtraInfoFunction = NULL;
static GetTouchInputInfoFn s_pfGetTouchInputInfoFunction = NULL;
static CloseTouchInputHandleFn s_pfCloseTouchInputHandleFunction = NULL;

static bool CheckTouchSupport()
{
	s_pfRegisterTouchWindowFunction = (RegisterTouchWindowFn)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "RegisterTouchWindow");
	s_pfUnregisterTouchWindowFunction = (UnregisterTouchWindowFn)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "UnregisterTouchWindow");
	s_pfGetMessageExtraInfoFunction = (GetMessageExtraInfoFn)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "GetMessageExtraInfo");
	s_pfGetTouchInputInfoFunction = (GetTouchInputInfoFn)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "GetTouchInputInfo");
	s_pfCloseTouchInputHandleFunction = (CloseTouchInputHandleFn)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "CloseTouchInputHandle");

	return (s_pfRegisterTouchWindowFunction && s_pfUnregisterTouchWindowFunction && s_pfGetMessageExtraInfoFunction && s_pfGetTouchInputInfoFunction && s_pfCloseTouchInputHandleFunction);
}

#endif /* #if(_MSC_VER >= 1600) */

#if defined(RENDERER_GL)

static void SetupPixelFormat(HDC hDC)
{
    int pixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof( PIXELFORMATDESCRIPTOR ),            // Size of this pixel format descriptor
		1,                                          // Version number
		PFD_DRAW_TO_WINDOW |                        // Format must support window
		PFD_SUPPORT_OPENGL |                        // Format must support OpenGL
		PFD_DOUBLEBUFFER,                           // Must support double buffering
		PFD_TYPE_RGBA,                              // Request a RGBA format
		32,                                         // Select our color depth
		0, 0, 0, 0, 0, 0,                           // Color bits ignored
		0,                                          // no alpha buffer
		0,                                          // Shift bit ignored
		0,                                          // No accumulation buffer
		0, 0, 0, 0,                                 // Accumulation bits ignored
		24,                                         // 24Bit z-buffer (depth buffer)  
		8,                                          // 8Bit stencil buffer
		0,                                          // No auxiliary buffer
		PFD_MAIN_PLANE,                             // Main drawing layer
		0,                                          // Reserved
		0, 0, 0                                     // Layer masks ignored
	};

    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pixelFormat, &pfd);
}

class CCGLDevice
{
public:
	CCGLDevice(HWND hWnd): _hWnd(hWnd)
	{
		HDC hDC = GetDC(hWnd);
		SetupPixelFormat(hDC);
		_hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, _hRC);
	}

	~CCGLDevice()
	{
		HDC hDC = GetDC(_hWnd);
		if (hDC != NULL && _hRC != NULL)
		{
			// deselect rendering context and delete it
			wglMakeCurrent(hDC, NULL);
			wglDeleteContext(_hRC);
		}
	}

	void swapBuffers()
	{
		HDC hDC = GetDC(_hWnd);
		::SwapBuffers(hDC); 
	}

	bool isReady() {  return (_hRC != NULL); }

	void* getNativeHandle() { return NULL; }
protected:
	HWND _hWnd;
	HGLRC _hRC; 
};

#elif defined(RENDERER_GLES2)

#include "EGL/EGL.h"
class CCGLDevice
{
public:
	CCGLDevice(HWND hWnd)
	{
		_eglNativeWindow = hWnd;
		_eglNativeDisplay = GetDC(hWnd);
		_eglSurface = EGL_NO_SURFACE;

		EGLDisplay eglDisplay;
		if (EGL_NO_DISPLAY == (eglDisplay = eglGetDisplay(_eglNativeDisplay)))
			return;

		EGLint nMajor, nMinor;
		if (EGL_FALSE == eglInitialize(eglDisplay, &nMajor, &nMinor) || 1 != nMajor)
			return;

		const EGLint aConfigAttribs[] =
		{
			EGL_LEVEL,				0,
			EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
			EGL_NATIVE_RENDERABLE,	EGL_FALSE,
			EGL_DEPTH_SIZE,			16,
			EGL_NONE,
		};
		EGLint iConfigs;
		EGLConfig eglConfig;
		if (EGL_FALSE == eglChooseConfig(eglDisplay, aConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
			return;

		const EGLint contextAttribs[] = 
		{
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};

		EGLContext eglContext;
		eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT , contextAttribs);
		if	(EGL_NO_CONTEXT == eglContext)
			return;

		EGLSurface eglSurface;
		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, _eglNativeWindow, NULL);
		if (EGL_NO_SURFACE == eglSurface)
			return;

		if (EGL_FALSE == eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
			return;

		_eglDisplay = eglDisplay;
		_eglConfig  = eglConfig;
		_eglContext = eglContext;
		_eglSurface = eglSurface;
	}

	~CCGLDevice()
	{
		if (EGL_NO_SURFACE != _eglSurface)
		{
			eglDestroySurface(_eglDisplay, _eglSurface);
		}
		if (EGL_NO_CONTEXT != _eglContext)
		{
			eglDestroyContext(_eglDisplay, _eglContext);
		}
		eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglTerminate(_eglDisplay);
		if (_eglNativeDisplay)
		{
			ReleaseDC(_eglNativeWindow, _eglNativeDisplay);
		}
	}

	void swapBuffers()
	{
		if (EGL_NO_DISPLAY != _eglDisplay)
		{
			eglSwapBuffers(_eglDisplay, _eglSurface);
		}
	}

	bool isReady() {  return (_eglSurface != EGL_NO_SURFACE && _eglContext != EGL_NO_CONTEXT); }

	void* getNativeHandle() { return NULL; }
protected:
	EGLNativeWindowType     _eglNativeWindow;
	EGLNativeDisplayType    _eglNativeDisplay;
	EGLDisplay              _eglDisplay;
	EGLConfig               _eglConfig;
	EGLSurface              _eglSurface;
	EGLContext              _eglContext;
};

#elif RENDERER_D3D11
#include "DXGI.h" 
#include <d3d11.h>

#define SAFE_RELEASE(obj) if (obj) { obj->Release(); obj=NULL; }

class CCGLDevice
{
public:
	CCGLDevice(HWND hWnd)
	{
		HRESULT hr;

		_renderTargetView = NULL;
		_depthStencil = NULL;
		_depthStencilView = NULL;

		UINT createDeviceFlags = 0;
	#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
		};
		UINT numDriverTypes = ARRAYSIZE( driverTypes );

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};
		UINT numFeatureLevels = ARRAYSIZE( featureLevels );

		int winWidth = 1000;
		int winHeight = 1000;

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory( &sd, sizeof( sd ) );
		sd.BufferCount = 1;
		sd.BufferDesc.Width = winWidth;
		sd.BufferDesc.Height = winHeight;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		ID3D11DeviceContext*     immediateContext;

		D3D_FEATURE_LEVEL        featureLevel;

		for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
		{
			hr = D3D11CreateDeviceAndSwapChain( NULL, driverTypes[driverTypeIndex], NULL, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd, &_swapChain, &_d3dDevice, &featureLevel, &immediateContext );
			if( SUCCEEDED( hr ) )
				break;
		}
		if ( FAILED( hr ) ) return;

		resize( winWidth, winHeight );
	}

	~CCGLDevice()
	{
		if (_d3dDevice!=NULL)
			_d3dDevice->Release();
	}

	void swapBuffers()
	{
		if (_swapChain)
			_swapChain->Present(0,0);
	}

	void resize( int winWidth, int winHeight)
	{
		SAFE_RELEASE(_renderTargetView);
		SAFE_RELEASE(_depthStencil);
		SAFE_RELEASE(_depthStencilView);

		HRESULT hr;
		ID3D11DeviceContext*     immediateContext = NULL;
		_d3dDevice->GetImmediateContext(&immediateContext);

		// Create a render target view
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = _swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
		if ( FAILED( hr ) )  exit(-1);

		D3D11_TEXTURE2D_DESC desc;
		pBackBuffer->GetDesc(&desc);
		if (desc.Width != winWidth || desc.Height != winHeight)
		{
			immediateContext->OMSetRenderTargets(0,0,0);
			pBackBuffer->Release();

			_swapChain->ResizeBuffers(1, winWidth, winHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0 );

			hr = _swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
			if ( FAILED( hr ) )  exit(-1);
		}

		hr = _d3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &_renderTargetView );
		pBackBuffer->Release();
		if ( FAILED( hr ) ) return;

		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory( &descDepth, sizeof(descDepth) );
		descDepth.Width = winWidth;
		descDepth.Height = winHeight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = _d3dDevice->CreateTexture2D( &descDepth, NULL, &_depthStencil );
		if ( FAILED( hr ) ) return;

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory( &descDSV, sizeof(descDSV) );
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = _d3dDevice->CreateDepthStencilView( _depthStencil, &descDSV, &_depthStencilView );
		if ( FAILED( hr ) ) return;

		immediateContext->OMSetRenderTargets( 1, &_renderTargetView, _depthStencilView );

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)winWidth;
		vp.Height = (FLOAT)winHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		immediateContext->RSSetViewports( 1, &vp ); 
	}

	bool isReady() { return _d3dDevice != NULL; }

	void* getNativeHandle() { return _d3dDevice; }
protected:
	ID3D11Device*            _d3dDevice;
	IDXGISwapChain*          _swapChain;

	ID3D11RenderTargetView*  _renderTargetView;
	ID3D11Texture2D*         _depthStencil;
	ID3D11DepthStencilView*  _depthStencilView;
};

#endif

//////////////////////////////////////////////////////////////////////////
// impliment CCEGLView
//////////////////////////////////////////////////////////////////////////
static CCEGLView* s_pMainWindow = NULL;
static const WCHAR* kWindowClassName = L"Cocos2dxWin32";

static LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (s_pMainWindow && s_pMainWindow->getHWnd() == hWnd)
    {
        return s_pMainWindow->WindowProc(uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

CCEGLView::CCEGLView()
: m_bCaptured(false)
, m_hWnd(NULL)
, m_hDC(NULL)
, m_hRC(NULL)
, m_lpfnAccelerometerKeyHook(NULL)
, m_menu(NULL)
, m_wndproc(NULL)
, m_fFrameZoomFactor(1.0f)
, m_bSupportTouch(false)
{
    strcpy(m_szViewName, "Cocos2dxWin32");
}

CCEGLView::~CCEGLView()
{

}

bool CCEGLView::Create()
{
    bool bRet = false;
    do
    {
        CC_BREAK_IF(m_hWnd);

        HINSTANCE hInstance = GetModuleHandle( NULL );
        WNDCLASS  wc;        // Windows Class Structure

        // Redraw On Size, And Own DC For Window.
        wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc    = _WindowProc;                    // WndProc Handles Messages
        wc.cbClsExtra     = 0;                              // No Extra Window Data
        wc.cbWndExtra     = 0;                                // No Extra Window Data
        wc.hInstance      = hInstance;                        // Set The Instance
        wc.hIcon          = LoadIcon( NULL, IDI_WINLOGO );    // Load The Default Icon
        wc.hCursor        = LoadCursor( NULL, IDC_ARROW );    // Load The Arrow Pointer
        wc.hbrBackground  = NULL;                           // No Background Required For GL
        wc.lpszMenuName   = m_menu;                         //
        wc.lpszClassName  = kWindowClassName;               // Set The Class Name

        CC_BREAK_IF(! RegisterClass(&wc) && 1410 != GetLastError());

        // center window position
        RECT rcDesktop;
        GetWindowRect(GetDesktopWindow(), &rcDesktop);

        WCHAR wszBuf[50] = {0};
        MultiByteToWideChar(CP_UTF8, 0, m_szViewName, -1, wszBuf, sizeof(wszBuf));

        // create window
        m_hWnd = CreateWindowEx(
            WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,    // Extended Style For The Window
            kWindowClassName,                                    // Class Name
            wszBuf,                                                // Window Title
            WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX,        // Defined Window Style
            0, 0,                                                // Window Position
            //TODO: Initializing width with a large value to avoid getting a wrong client area by 'GetClientRect' function.
            1000,                                               // Window Width
            1000,                                               // Window Height
            NULL,                                                // No Parent Window
            NULL,                                                // No Menu
            hInstance,                                            // Instance
            NULL );

        CC_BREAK_IF(! m_hWnd);

		m_glDevice = new CCGLDevice(m_hWnd);

		if (!m_glDevice->isReady())
		{
			m_hWnd = NULL;
			return false;
		} 

        s_pMainWindow = this;
        bRet = true;
    } while (0);

#if(_MSC_VER >= 1600)
    m_bSupportTouch = CheckTouchSupport();
    if(m_bSupportTouch)
	{
	    m_bSupportTouch = (s_pfRegisterTouchWindowFunction(m_hWnd, 0) != 0);
    }
#endif /* #if(_MSC_VER >= 1600) */

    return bRet;
}

LRESULT CCEGLView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL bProcessed = FALSE;

    switch (message)
    {
    case WM_LBUTTONDOWN:
#if(_MSC_VER >= 1600)
        // Don't process message generated by Windows Touch
        if (m_bSupportTouch && (s_pfGetMessageExtraInfoFunction() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH) break;
#endif /* #if(_MSC_VER >= 1600) */

        if (m_pDelegate && MK_LBUTTON == wParam)
        {
            POINT point = {(short)LOWORD(lParam), (short)HIWORD(lParam)};
            CCPoint pt(point.x, point.y);
            pt.x /= m_fFrameZoomFactor;
            pt.y /= m_fFrameZoomFactor;
            CCPoint tmp = ccp(pt.x, m_obScreenSize.height - pt.y);
            if (m_obViewPortRect.equals(CCRectZero) || m_obViewPortRect.containsPoint(tmp))
            {
                m_bCaptured = true;
                SetCapture(m_hWnd);
                int id = 0;
                handleTouchesBegin(1, &id, &pt.x, &pt.y);
            }
        }
        break;

    case WM_MOUSEMOVE:
#if(_MSC_VER >= 1600)
        // Don't process message generated by Windows Touch
        if (m_bSupportTouch && (s_pfGetMessageExtraInfoFunction() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH) break;
#endif /* #if(_MSC_VER >= 1600) */
        if (MK_LBUTTON == wParam && m_bCaptured)
        {
            POINT point = {(short)LOWORD(lParam), (short)HIWORD(lParam)};
            CCPoint pt(point.x, point.y);
            int id = 0;
            pt.x /= m_fFrameZoomFactor;
            pt.y /= m_fFrameZoomFactor;
            handleTouchesMove(1, &id, &pt.x, &pt.y);
        }
        break;

    case WM_LBUTTONUP:
#if(_MSC_VER >= 1600)
        // Don't process message generated by Windows Touch
        if (m_bSupportTouch && (s_pfGetMessageExtraInfoFunction() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH) break;
#endif /* #if(_MSC_VER >= 1600) */
        if (m_bCaptured)
        {
            POINT point = {(short)LOWORD(lParam), (short)HIWORD(lParam)};
            CCPoint pt(point.x, point.y);
            int id = 0;
            pt.x /= m_fFrameZoomFactor;
            pt.y /= m_fFrameZoomFactor;
            handleTouchesEnd(1, &id, &pt.x, &pt.y);

            ReleaseCapture();
            m_bCaptured = false;
        }
        break;
#if(_MSC_VER >= 1600)
    case WM_TOUCH:
		{
            BOOL bHandled = FALSE;
            UINT cInputs = LOWORD(wParam);
            PTOUCHINPUT pInputs = new TOUCHINPUT[cInputs];
            if (pInputs)
            {
                if (s_pfGetTouchInputInfoFunction((HTOUCHINPUT)lParam, cInputs, pInputs, sizeof(TOUCHINPUT)))
                {
                    for (UINT i=0; i < cInputs; i++)
                    {
                        TOUCHINPUT ti = pInputs[i];
                        POINT input;
                        input.x = TOUCH_COORD_TO_PIXEL(ti.x);
                        input.y = TOUCH_COORD_TO_PIXEL(ti.y);
                        ScreenToClient(m_hWnd, &input);
                        CCPoint pt(input.x, input.y);
                        CCPoint tmp = ccp(pt.x, m_obScreenSize.height - pt.y);
                        if (m_obViewPortRect.equals(CCRectZero) || m_obViewPortRect.containsPoint(tmp))
                        {
                            pt.x /= m_fFrameZoomFactor;
                            pt.y /= m_fFrameZoomFactor;

                            if (ti.dwFlags & TOUCHEVENTF_DOWN)
                                handleTouchesBegin(1, reinterpret_cast<int*>(&ti.dwID), &pt.x, &pt.y);
                            else if (ti.dwFlags & TOUCHEVENTF_MOVE)
                                handleTouchesMove(1, reinterpret_cast<int*>(&ti.dwID), &pt.x, &pt.y);
                            else if (ti.dwFlags & TOUCHEVENTF_UP)
                                handleTouchesEnd(1, reinterpret_cast<int*>(&ti.dwID), &pt.x, &pt.y);
                         }
                     }
                     bHandled = TRUE;
                 }
                 delete [] pInputs;
             }
             if (bHandled)
             {
                 s_pfCloseTouchInputHandleFunction((HTOUCHINPUT)lParam);
             }
		}
      break;
#endif /* #if(_MSC_VER >= 1600) */
    case WM_SIZE:
        switch (wParam)
        {
        case SIZE_RESTORED:
            CCApplication::sharedApplication()->applicationWillEnterForeground();
            break;
        case SIZE_MINIMIZED:
            CCApplication::sharedApplication()->applicationDidEnterBackground();
            break;
        }
        break;
    case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
        if (wParam == VK_F1 || wParam == VK_F2)
        {
            CCDirector* pDirector = CCDirector::sharedDirector();
            if (GetKeyState(VK_LSHIFT) < 0 ||  GetKeyState(VK_RSHIFT) < 0 || GetKeyState(VK_SHIFT) < 0)
                pDirector->getKeypadDispatcher()->dispatchKeypadMSG(wParam == VK_F1 ? kTypeBackClicked : kTypeMenuClicked);
        }
        else if (wParam == VK_ESCAPE)
        {
            CCDirector::sharedDirector()->getKeypadDispatcher()->dispatchKeypadMSG(kTypeBackClicked);
        }
		handleKeyEvent(VirtualKeyCodeToCocos2D(wParam, lParam), true);

        if ( m_lpfnAccelerometerKeyHook!=NULL )
        {
            (*m_lpfnAccelerometerKeyHook)( message,wParam,lParam );
        }
        break;
    case WM_KEYUP:
	case WM_SYSKEYUP:
		handleKeyEvent(VirtualKeyCodeToCocos2D(wParam, lParam), false);

        if ( m_lpfnAccelerometerKeyHook!=NULL )
        {
            (*m_lpfnAccelerometerKeyHook)( message,wParam,lParam );
        }
        break;
    case WM_CHAR:
        {
            if (wParam < 0x20)
            {
                if (VK_BACK == wParam)
                {
                    CCIMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
                }
                else if (VK_RETURN == wParam)
                {
                    CCIMEDispatcher::sharedDispatcher()->dispatchInsertText("\n", 1);
                }
                else if (VK_TAB == wParam)
                {
                    // tab input
                }
                else if (VK_ESCAPE == wParam)
                {
                    // ESC input
                    //CCDirector::sharedDirector()->end();
                }
            }
            else if (wParam < 128)
            {
                // ascii char
                CCIMEDispatcher::sharedDispatcher()->dispatchInsertText((const char *)&wParam, 1);
            }
            else
            {
                char szUtf8[8] = {0};
                int nLen = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)&wParam, 1, szUtf8, sizeof(szUtf8), NULL, NULL);
                CCIMEDispatcher::sharedDispatcher()->dispatchInsertText(szUtf8, nLen);
            }
            if ( m_lpfnAccelerometerKeyHook!=NULL )
            {
                (*m_lpfnAccelerometerKeyHook)( message,wParam,lParam );
            }
        }
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;
        BeginPaint(m_hWnd, &ps);
        EndPaint(m_hWnd, &ps);
        break;

    case WM_CLOSE:
        CCDirector::sharedDirector()->end();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        if (m_wndproc)
        {

            m_wndproc(message, wParam, lParam, &bProcessed);
            if (bProcessed) break;
        }
        return DefWindowProc(m_hWnd, message, wParam, lParam);
    }

    if (m_wndproc && !bProcessed)
    {
        m_wndproc(message, wParam, lParam, &bProcessed);
    }
    return 0;
}

void CCEGLView::setAccelerometerKeyHook( LPFN_ACCELEROMETER_KEYHOOK lpfnAccelerometerKeyHook )
{
    m_lpfnAccelerometerKeyHook=lpfnAccelerometerKeyHook;
}


bool CCEGLView::isOpenGLReady()
{
	return m_glDevice!=NULL && m_glDevice->isReady(); 
}

void CCEGLView::end()
{
    if (m_hWnd)
    {
#if(_MSC_VER >= 1600)
        if(m_bSupportTouch)
		{
		    s_pfUnregisterTouchWindowFunction(m_hWnd);
		}
#endif /* #if(_MSC_VER >= 1600) */
        DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }
    s_pMainWindow = NULL;
    UnregisterClass(kWindowClassName, GetModuleHandle(NULL));
    delete this;
}

void CCEGLView::swapBuffers()
{
    if (m_glDevice != NULL)
		m_glDevice->swapBuffers(); 
}

void* CCEGLView::getDevice()
{
    if (m_glDevice != NULL)
		return m_glDevice->getNativeHandle();
	return NULL;
} 

void CCEGLView::setIMEKeyboardState(bool /*bOpen*/)
{

}

void CCEGLView::setMenuResource(LPCWSTR menu)
{
    m_menu = menu;
    if (m_hWnd != NULL)
    {
        HMENU hMenu = LoadMenu(GetModuleHandle(NULL), menu);
        SetMenu(m_hWnd, hMenu);
    }
}

void CCEGLView::setWndProc(CUSTOM_WND_PROC proc)
{
    m_wndproc = proc;
}

HWND CCEGLView::getHWnd()
{
    return m_hWnd;
}

void CCEGLView::resize(int width, int height)
{
    if (! m_hWnd)
    {
        return;
    }

    RECT rcWindow;
    GetWindowRect(m_hWnd, &rcWindow);

    RECT rcClient;
    GetClientRect(m_hWnd, &rcClient);

    // calculate new window width and height
    POINT ptDiff;
    ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
    rcClient.right = rcClient.left + width;
    rcClient.bottom = rcClient.top + height;

    const CCSize& frameSize = getFrameSize();
    if (frameSize.width > 0)
    {
        WCHAR wszBuf[MAX_PATH] = {0};
#ifdef _DEBUG
        char szBuf[MAX_PATH + 1];
        memset(szBuf, 0, sizeof(szBuf));
        snprintf(szBuf, MAX_PATH, "%s - %0.0fx%0.0f - %0.2f",
                   m_szViewName, frameSize.width, frameSize.height, m_fFrameZoomFactor);
        MultiByteToWideChar(CP_UTF8, 0, szBuf, -1, wszBuf, sizeof(wszBuf));
#else
        MultiByteToWideChar(CP_UTF8, 0, m_szViewName, -1, wszBuf, sizeof(wszBuf));
#endif
        SetWindowText(m_hWnd, wszBuf);
    }

    AdjustWindowRectEx(&rcClient, GetWindowLong(m_hWnd, GWL_STYLE), FALSE, GetWindowLong(m_hWnd, GWL_EXSTYLE));

    // change width and height
    SetWindowPos(m_hWnd, 0, 0, 0, width + ptDiff.x, height + ptDiff.y,
                 SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

#ifdef RENDERER_D3D11
	if (m_glDevice!=NULL)
		m_glDevice->resize(frameSize.width, frameSize.height);
#endif
}

void CCEGLView::setFrameZoomFactor(float fZoomFactor)
{
    m_fFrameZoomFactor = fZoomFactor;
    resize(m_obScreenSize.width * fZoomFactor, m_obScreenSize.height * fZoomFactor);
    centerWindow();
//    CCDirector::sharedDirector()->setProjection(CCDirector::sharedDirector()->getProjection());
}

float CCEGLView::getFrameZoomFactor()
{
    return m_fFrameZoomFactor;
}

void CCEGLView::setFrameSize(float width, float height)
{
    CCEGLViewProtocol::setFrameSize(width, height);

    resize(width, height); // adjust window size for menubar
    centerWindow();
}

void CCEGLView::centerWindow()
{
    if (! m_hWnd)
    {
        return;
    }

    RECT rcDesktop, rcWindow;
    GetWindowRect(GetDesktopWindow(), &rcDesktop);

    // substract the task bar
    HWND hTaskBar = FindWindow(TEXT("Shell_TrayWnd"), NULL);
    if (hTaskBar != NULL)
    {
        APPBARDATA abd;

        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hTaskBar;

        SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
        SubtractRect(&rcDesktop, &rcDesktop, &abd.rc);
    }
    GetWindowRect(m_hWnd, &rcWindow);

    int offsetX = rcDesktop.left + (rcDesktop.right - rcDesktop.left - (rcWindow.right - rcWindow.left)) / 2;
    offsetX = (offsetX > 0) ? offsetX : rcDesktop.left;
    int offsetY = rcDesktop.top + (rcDesktop.bottom - rcDesktop.top - (rcWindow.bottom - rcWindow.top)) / 2;
    offsetY = (offsetY > 0) ? offsetY : rcDesktop.top;

    SetWindowPos(m_hWnd, 0, offsetX, offsetY, 0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

CCEGLView* CCEGLView::sharedOpenGLView()
{
    static CCEGLView* s_pEglView = NULL;
    if (s_pEglView == NULL)
    {
        s_pEglView = new CCEGLView();
		if(!s_pEglView->Create())
		{
			delete s_pEglView;
			s_pEglView = NULL;
		}
    }

    return s_pEglView;
}

////////////////////////////////////////////////////////////
/// Convert a Win32 virtual key code to cocos2d key code
////////////////////////////////////////////////////////////
Key::Code VirtualKeyCodeToCocos2D(WPARAM VirtualKey, LPARAM Flags)
{
    switch (VirtualKey)
    {
        // Check the scancode to distinguish between left and right shift
        case VK_SHIFT :
        {
            static UINT LShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
            UINT scancode = (Flags & (0xFF << 16)) >> 16;
            return scancode == LShift ? Key::LShift : Key::RShift;
        }

        // Check the "extended" flag to distinguish between left and right alt
        case VK_MENU : return (HIWORD(Flags) & KF_EXTENDED) ? Key::RAlt : Key::LAlt;

        // Check the "extended" flag to distinguish between left and right control
        case VK_CONTROL : return (HIWORD(Flags) & KF_EXTENDED) ? Key::RControl : Key::LControl;

        // Other keys are reported properly
        case VK_LWIN :       return Key::LSystem;
        case VK_RWIN :       return Key::RSystem;
        case VK_APPS :       return Key::Menu;
        case VK_OEM_1 :      return Key::SemiColon;
        case VK_OEM_2 :      return Key::Slash;
        case VK_OEM_PLUS :   return Key::Equal;
        case VK_OEM_MINUS :  return Key::Dash;
        case VK_OEM_4 :      return Key::LBracket;
        case VK_OEM_6 :      return Key::RBracket;
        case VK_OEM_COMMA :  return Key::Comma;
        case VK_OEM_PERIOD : return Key::Period;
        case VK_OEM_7 :      return Key::Quote;
        case VK_OEM_5 :      return Key::BackSlash;
        case VK_OEM_3 :      return Key::Tilde;
        case VK_ESCAPE :     return Key::Escape;
        case VK_SPACE :      return Key::Space;
        case VK_RETURN :     return Key::Return;
        case VK_BACK :       return Key::Back;
        case VK_TAB :        return Key::Tab;
        case VK_PRIOR :      return Key::PageUp;
        case VK_NEXT :       return Key::PageDown;
        case VK_END :        return Key::End;
        case VK_HOME :       return Key::Home;
        case VK_INSERT :     return Key::Insert;
        case VK_DELETE :     return Key::Delete;
        case VK_ADD :        return Key::Add;
        case VK_SUBTRACT :   return Key::Subtract;
        case VK_MULTIPLY :   return Key::Multiply;
        case VK_DIVIDE :     return Key::Divide;
        case VK_PAUSE :      return Key::Pause;
        case VK_F1 :         return Key::F1;
        case VK_F2 :         return Key::F2;
        case VK_F3 :         return Key::F3;
        case VK_F4 :         return Key::F4;
        case VK_F5 :         return Key::F5;
        case VK_F6 :         return Key::F6;
        case VK_F7 :         return Key::F7;
        case VK_F8 :         return Key::F8;
        case VK_F9 :         return Key::F9;
        case VK_F10 :        return Key::F10;
        case VK_F11 :        return Key::F11;
        case VK_F12 :        return Key::F12;
        case VK_F13 :        return Key::F13;
        case VK_F14 :        return Key::F14;
        case VK_F15 :        return Key::F15;
        case VK_LEFT :       return Key::Left;
        case VK_RIGHT :      return Key::Right;
        case VK_UP :         return Key::Up;
        case VK_DOWN :       return Key::Down;
        case VK_NUMPAD0 :    return Key::Numpad0;
        case VK_NUMPAD1 :    return Key::Numpad1;
        case VK_NUMPAD2 :    return Key::Numpad2;
        case VK_NUMPAD3 :    return Key::Numpad3;
        case VK_NUMPAD4 :    return Key::Numpad4;
        case VK_NUMPAD5 :    return Key::Numpad5;
        case VK_NUMPAD6 :    return Key::Numpad6;
        case VK_NUMPAD7 :    return Key::Numpad7;
        case VK_NUMPAD8 :    return Key::Numpad8;
        case VK_NUMPAD9 :    return Key::Numpad9;
        case 'A' :           return Key::A;
        case 'Z' :           return Key::Z;
        case 'E' :           return Key::E;
        case 'R' :           return Key::R;
        case 'T' :           return Key::T;
        case 'Y' :           return Key::Y;
        case 'U' :           return Key::U;
        case 'I' :           return Key::I;
        case 'O' :           return Key::O;
        case 'P' :           return Key::P;
        case 'Q' :           return Key::Q;
        case 'S' :           return Key::S;
        case 'D' :           return Key::D;
        case 'F' :           return Key::F;
        case 'G' :           return Key::G;
        case 'H' :           return Key::H;
        case 'J' :           return Key::J;
        case 'K' :           return Key::K;
        case 'L' :           return Key::L;
        case 'M' :           return Key::M;
        case 'W' :           return Key::W;
        case 'X' :           return Key::X;
        case 'C' :           return Key::C;
        case 'V' :           return Key::V;
        case 'B' :           return Key::B;
        case 'N' :           return Key::N;
        case '0' :           return Key::Num0;
        case '1' :           return Key::Num1;
        case '2' :           return Key::Num2;
        case '3' :           return Key::Num3;
        case '4' :           return Key::Num4;
        case '5' :           return Key::Num5;
        case '6' :           return Key::Num6;
        case '7' :           return Key::Num7;
        case '8' :           return Key::Num8;
        case '9' :           return Key::Num9;
    }

    return Key::Code(0);
}



NS_CC_END
