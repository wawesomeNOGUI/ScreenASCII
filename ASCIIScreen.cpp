#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// character(s) to draw on screen
const char charString[] = "GAMER";

// length of charString will be set by strlen(charString) in main
const int CHARS = strlen(charString); 

const COLORREF TRANSPARENT_COLOR = RGB(0, 0, 0);
const COLORREF BACKGROUND_COLOR = RGB(1, 1, 1);

HANDLE keyCheckHandle;

int myWidth, myHeight;
int monitorWidth, monitorHeight;
HDC hDesktopDC, hMyDC, hdcMemDC;
// HRGN hMyRg;
HBITMAP hMyBmp;
RGBQUAD *pPixels;
BITMAPINFO bmi;

// Function prototypes (forward declarations)
void GetMonitorRealResolution(HMONITOR hmon, int* pixelsWidth, int* pixelsHeight, MONITORINFO  *info);
HWND CreateFullscreenWindow(HMONITOR hmon, HINSTANCE *hInstance, MONITORINFO  *info);
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

    MONITORINFO info;

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

    // Transparency settings for window
    // SetLayeredWindowAttributes(hwnd, 
    //     TRANSPARENT_COLOR, // color that will be rendered fully transparent
    //     255,    // 0 - 255 controls overall trnasparency of window (alpha value)
    //     LWA_COLORKEY | LWA_ALPHA
    // );

    // set window to always on top
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // set window to not be included in screen capture
    // WDA_EXCLUDEFROMCAPTURE requires at least win 10
    SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE); 

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
	hMyDC = GetDC(hwnd);
    hDesktopDC = GetDC(NULL);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hDesktopDC);
	hMyBmp = CreateCompatibleBitmap(hDesktopDC, myWidth, myHeight);
    SelectObject(hdcMemDC, hMyBmp);

    //set Background Color
    SelectObject(hdcMemDC, CreateSolidBrush(BACKGROUND_COLOR));

    // This is the best stretch mode.
    SetStretchBltMode(hMyDC, HALFTONE);
    SetStretchBltMode(hdcMemDC, HALFTONE);

	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = myWidth;
	bmi.bmiHeader.biHeight = -myHeight;  // negative sets origin in top left
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	pPixels = (RGBQUAD*) malloc(myWidth * myHeight * sizeof(RGBQUAD));

    // Start keypress control thread
    // could also look into using hot keys:
    // https://learn.microsoft.com/en-us/windows/win32/Controls/hot-key-controls
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
void GetMonitorRealResolution(HMONITOR hmon, int* pixelsWidth, int* pixelsHeight, MONITORINFO *info)
{
    GetMonitorInfo(hmon, info);
    DEVMODE devmode = {};
    devmode.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
    *pixelsWidth = devmode.dmPelsWidth;
    *pixelsHeight = devmode.dmPelsHeight;
}

// https://stackoverflow.com/questions/2382464/win32-full-screen-and-hiding-taskbar
HWND CreateFullscreenWindow(HMONITOR hmon, HINSTANCE *hInstance, MONITORINFO  *info)
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
            BeginPaint(hwnd, &ps); 

            // FillRect(hMyDC, &rect, CreateSolidBrush(TRANSPARENT_COLOR));
            DrawAscii();

            EndPaint(hwnd, &ps); 

            // RECT rect;
            // GetClientRect(hwnd, &rect);

            Sleep(10.5); // thread sleep in milliseconds

            // Sleep(1500);

            // mark window for redrawing if not minimized
            if (!IsIconic(hwnd))
                InvalidateRect(hwnd, NULL, true);
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

    // The source DC is the whole screen, and the destination DC is buffer dc (hdcMemDC).
    if (!StretchBlt(hdcMemDC,
        0, 0,
        myWidth, myHeight,
        hDesktopDC,
        0, 0,
        monitorWidth, monitorHeight,
        SRCCOPY))
    {
        printf("StretchBlt failed.\n");
        return;
    }

    // get pixel data into pPixels
    GetDIBits(
        hdcMemDC,
        hMyBmp,
        0,
        myHeight,
        pPixels,
        &bmi,
        DIB_RGB_COLORS
    );

    // fill background
    SetBkColor(hdcMemDC, BACKGROUND_COLOR);
    Rectangle(hdcMemDC, 0, 0, myWidth, myHeight);

    // // Parse pixels for color and then draw le text
    int i = 0;
    wchar_t myChar;

    for(int y = 0; y < myHeight; y += 13) {
        for(int x = 0; x < myWidth; x += 12){
            int p = (y * myWidth) + x;

            int avgColor = (pPixels[p].rgbRed + pPixels[p].rgbGreen + pPixels[p].rgbBlue) * 0.33;
            COLORREF colorBlackAndWhite = RGB(avgColor, avgColor, avgColor);
            SetTextColor(hdcMemDC, colorBlackAndWhite);


            // SetTextColor(hdcMemDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));

            // set background here to make cool large pixely look
            // SetBkColor(hdcMemDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));
            
            // printable ascii range is 32 (space) through 126 (~)
            // myChar = (int) rand() % (126 - 32 + 1) + 32;
            myChar = charString[i++ % (CHARS)];
            ExtTextOutW(hdcMemDC, x, y, ETO_OPAQUE, NULL, &myChar, 1, NULL);
        }
    }

    
    // Bit block transfer onto window dc
    if (!BitBlt(hMyDC,
        0, 0,
        myWidth, myHeight,
        hdcMemDC,
        0, 0,
        SRCCOPY))
    {
        // MessageBox(hwnd, L"BitBlt has failed", L"Failed", MB_OK);
    }
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