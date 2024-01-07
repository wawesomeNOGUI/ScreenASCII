#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <stdio.h>
#include <time.h>

// character(s) to draw on screen
const int CHARS = 6;
const char charString[CHARS] = "GAMER";
wchar_t myChar;
LPCWSTR c = &myChar;

const COLORREF TRANSPARENT_COLOR = RGB(0, 0, 0);
const COLORREF BACKGROUND_COLOR = RGB(1, 1, 1);

HANDLE keyCheckHandle;

int myWidth, myHeight;
int monitorWidth, monitorHeight;
HDC hMyDC, hdcMemDC;
// HRGN hMyRg;
HBITMAP hMyBmp;
RGBQUAD *pPixels;
BITMAPINFO bmi;

// Function prototypes (forward declarations)
void GetMonitorRealResolution(HMONITOR hmon, int* pixelsWidth, int* pixelsHeight, MONITORINFOEX *info);
HWND CreateFullscreenWindow(HMONITOR hmon, HINSTANCE *hInstance, MONITORINFOEX *info);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Wineventproc(
  HWINEVENTHOOK hWinEventHook,
  DWORD event,
  HWND hwnd,
  LONG idObject,
  LONG idChild,
  DWORD idEventThread,
  DWORD dwmsEventTime
);
LRESULT CALLBACK KeyboardProc(
  int    code,
  WPARAM wParam,
  LPARAM lParam
);
void DrawAscii();
DWORD WINAPI CheckKeyStateLoop(LPVOID lpParam);

int main()
{
    srand(time(0));
    HINSTANCE hInstance;

    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HMONITOR hmon = MonitorFromWindow(GetForegroundWindow(),
                        MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX info = { sizeof(MONITORINFOEX) };

    // get this monitor's width and height for drawing
    GetMonitorRealResolution(hmon, &monitorWidth, &monitorHeight, &info);
    printf("%d  %d\n", monitorWidth, monitorHeight);

    HWND hwnd = CreateFullscreenWindow(hmon, &hInstance, &info);

    // record this window's width and height
    // in my testing the windows width was sometimes different
    // than the monitor resolution even when window was fullscreen 
    // (presumably due to some windows scaling)
    RECT rect;
    GetClientRect(hwnd, &rect);
    myWidth = rect.right - rect.left;
    myHeight = rect.bottom - rect.top;

    // ShowWindow(hwnd, SW_NORMAL);

    // set window to be clickable through
    // https://stackoverflow.com/questions/13069717/letting-the-mouse-pass-through-windows-c
    // https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles?redirectedfrom=MSDN
    // WS_EX_APPWINDOW - forces taskbar icon to be shown, but also allows this window to be foreground?
    // WS_EX_NOACTIVATE - window cannot be foreground
    // WS_EX_TRANSPARENT | WS_EX_LAYERED
    // LONG cur_style = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_LAYERED);

    // Make black pixels transparent:
    SetLayeredWindowAttributes(hwnd, TRANSPARENT_COLOR, 0, LWA_COLORKEY);

    // set window to always on top
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


    // Set event listener for foreground window changing
    // no need to call UnhookWinEvent, automatically called 
    // when this thread ends
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-unhookwinevent#remarks
    SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,    // event min
        EVENT_SYSTEM_FOREGROUND,    // event max
        NULL,   // hook function not in DLL
        Wineventproc,   // hook function
        0,  // receive events from all process on current desktop
        0,  // hook function assciated with all existing threads on desktop
        WINEVENT_OUTOFCONTEXT
    );

    // Setup for drawing
    // https://learn.microsoft.com/en-us/windows/win32/gdi/capturing-an-image
	HDC hMyDC = GetDC(hwnd);
    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hMyDC);

    // This is the best stretch mode.
    SetStretchBltMode(hMyDC, HALFTONE);

	HBITMAP hMyBmp = CreateCompatibleBitmap(hMyDC, monitorWidth, monitorHeight);
    SelectObject(hdcMemDC, hMyBmp);

	// pPixels = new RGBQUAD[nScreenWidth * nScreenHeight];
	pPixels = (RGBQUAD*) malloc(myWidth * myHeight * sizeof(RGBQUAD));

	// printf("%d", nScreenWidth * nScreenHeight * sizeof(RGBQUAD));

	// SelectObject(hMyDC, CreateSolidBrush(backgroundColor));

	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = myWidth;
	bmi.bmiHeader.biHeight = myHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

    // Start keypress control thread
    keyCheckHandle = CreateThread(
        NULL,           // default security attributes
        0,              // use default stack size  
        CheckKeyStateLoop,  // function to run in new thread
        hwnd,   // thread function parameters
        0,      // thread runs immediately after creation
        NULL    // pointer to variable to receive thread id
    );

    // Run the message and update loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// https://stackoverflow.com/questions/4631292/how-to-detect-the-current-screen-resolution
void GetMonitorRealResolution(HMONITOR hmon, int* pixelsWidth, int* pixelsHeight, MONITORINFOEX *info)
{
    GetMonitorInfo(hmon, info);
    DEVMODE devmode = {};
    devmode.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(info->szDevice, ENUM_CURRENT_SETTINGS, &devmode);
    *pixelsWidth = devmode.dmPelsWidth;
    *pixelsHeight = devmode.dmPelsHeight;
}

// https://stackoverflow.com/questions/2382464/win32-full-screen-and-hiding-taskbar
HWND CreateFullscreenWindow(HMONITOR hmon, HINSTANCE *hInstance, MONITORINFOEX *info)
{
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    
    return CreateWindowEx(
    0,
    CLASS_NAME,
    L"ASCII Screen",
    WS_POPUP | WS_MAXIMIZE | WS_VISIBLE,
    info->rcMonitor.left,
    info->rcMonitor.top,
    0,
    0,
    NULL,       // Parent window    
    NULL,       // Menu
    *hInstance,  // Instance handle
    NULL        // Additional application data
    );
}

void Wineventproc(
  HWINEVENTHOOK hWinEventHook,
  DWORD event,
  HWND hwnd,
  LONG idObject,
  LONG idChild,
  DWORD idEventThread,
  DWORD dwmsEventTime
)
{
    printf("Foreground window changed\n");
    // InvalidateRect(WindowFromDC(hMyDC), NULL, true);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        {
            printf("Goodbye!");
            CloseHandle(keyCheckHandle);
            free(pPixels);
            PostQuitMessage(0);
        }
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            hMyDC = BeginPaint(hwnd, &ps); 
  
            // static int x = 0;

            // printf("%d\n", x);

            RECT rect;
            GetClientRect(hwnd, &rect);

            // RGB(0, 0, 0) is the transparent color defined at top
            // FillRect(hMyDC, &rect, CreateSolidBrush(TRANSPARENT_COLOR));
            FillRect(hMyDC, &rect, CreateSolidBrush(BACKGROUND_COLOR));

            DrawAscii();

            EndPaint(hwnd, &ps); 

            // RECT rect;
            // GetClientRect(hwnd, &rect);

            // Sleep(0.5); // thread sleep in milliseconds

            // Sleep(1500);

            // mark window for redrawing
            // InvalidateRect(hwnd, NULL, true);

            // printf("hi\n");

            


            // maybe put this in function whenever something changes
            // or just once when app starts
            // probably don't need to be called every update?
            // HDC appDC = GetDC(GetForegroundWindow());
            
            // DrawAscii();

            //Draw Everything / and/or update window position/size
            // BLENDFUNCTION blend = { 0 };
            // blend.BlendOp = AC_SRC_OVER;
            // blend.SourceConstantAlpha = 255;
            // blend.AlphaFormat = AC_SRC_ALPHA;

            // UpdateLayeredWindow(hwnd, NULL, NULL, NULL,
            // hMyDC, NULL, RGB(0, 0, 0), &blend, ULW_OPAQUE);
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// void DrawAscii(HWND hwnd, HDC hMyDC, HBITMAP hMyBmp, RGBQUAD *pPixels)
void DrawAscii()
{
    // https://stackoverflow.com/questions/53379482/bitblt-captured-pixels-are-all-zero-bgra-when-using-hdc-of-another-app
    // can't use DC of other apps :()
    // so maybe make this window transparent, take full screenshot, and then turn to ascii?
    // HDC appDC = GetDC(GetForegroundWindow());
    // HDC desktopDC = GetDC(NULL);   // gets screenshot of whole screen
    // if (desktopDC == NULL)
    // {
    //     printf("ruh roh\n");
    //     return;
    // }
    // if (hMyDC == NULL)
    // {
    //     printf("ruh roh2\n");
    //     return;
    // }

    // The source DC is the whole screen, and the destination DC is the asci window (hwnd).
    // RECT    rcCli;          
    // GetClientRect(WindowFromDC(hMyDC), &rcCli);

    // if (!StretchBlt(hMyDC,
    //     0, 0,
    //     myWidth, myHeight,
    //     desktopDC,
    //     0, 0,
    //     monitorWidth, monitorHeight,
    //     SRCCOPY))
    // {
    //     printf("StretchBlt failed.\n");
    //     return;
    // }

    // if (!BitBlt(hMyDC,
    //     0, 0, 
    //     GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 
    //     appDC, 
    //     0, 0, 
    //     SRCCOPY))
    // {
    //     printf("BitBlt failed.\n");
    //     return;
    // }

    // printf("hi2");

    // get pixel data into pPixels
    // GetDIBits(
    //     hMyDC,
    //     hMyBmp,
    //     0,
    //     myHeight,
    //     pPixels,
    //     &bmi,
    //     DIB_RGB_COLORS
    // );
    // Rectangle(hMyDC, 0, 0, myWidth, myHeight);
    SetTextColor(hMyDC, TRANSPARENT_COLOR);
    SetBkColor(hMyDC, BACKGROUND_COLOR);

    // Parse pixels for color and then draw le text
    int i = 0;

    for(int y = 0; y < myHeight; y += 13) {
        for(int x = 0; x < myWidth; x += 12){
            int p = (myHeight-y-1)*myWidth+x; // flip the right way, 0,0 in top left
            // SetTextColor(hMyDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));
            //TextOutA(hMyDC, x, y, s, strlen(s));
            //TextOutW(hMyDC, x, y, s, wcslen(s));  //only use wcslen for a string, not a single character

            //TextOutW(hMyDC, x, y, s, 1);
            // SetBkColor(hMyDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));
            // ExtTextOutW(hMyDC, x, y, ETO_OPAQUE | ETO_IGNORELANGUAGE, NULL, s, 1, NULL);

            // printable ascii range is 32 (space) through 126 (~)
            // myChar = (int) rand() % (126 - 32 + 1) + 32;
            myChar = charString[i++ % (CHARS - 1)];
            ExtTextOutW(hMyDC, x, y, ETO_OPAQUE, NULL, c, 1, NULL);
        }
    }

    

    //HDC appDC = ::GetDC(hwnd);
    // BitBlt(appDC,100,100,nScreenWidth,nScreenHeight, hMyDC,0,0,SRCCOPY);
    // BitBlt(hMyDC, nScreenWidth/2, 0, nScreenWidth/2, nScreenHeight/2+250, hMyDC, 0, 0, SRCCOPY);
}

// used for checking if user presses bound key to exit program
// or other specified function
// only call this function once
DWORD WINAPI CheckKeyStateLoop(LPVOID lpParam)
{
    bool kKeyDown = false;

    HWND hwnd = (HWND) lpParam;

    while(true)
    {
        Sleep(10); // thread sleep in milliseconds

        // check keys
        // GetAsyncKeyState is < 0 if key down
        // GetAsyncKeyState uses virtual key codes:
        // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes


        // toggle ASCIIScreen display
        if (GetAsyncKeyState(0x4B) < 0 && !kKeyDown) // 0x4B == K
        {
            kKeyDown = true;

            // check if window is minimized (iconic).
            if (IsIconic(hwnd))
            {
                ShowWindow(hwnd, SW_MAXIMIZE);
            }
            else
            {
                ShowWindow(hwnd, SW_MINIMIZE);
            }
        }
        else if (GetAsyncKeyState(0x4B) == 0 && kKeyDown)
        {
            kKeyDown = false;
        }
    }
}