#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <stdio.h>
#include <time.h>

// character(s) to draw on screen
const char charString[] = "HIJKHKL";

// length of charString will be set by strlen(charString) in main
const int CHARS = strlen(charString); 

const COLORREF TRANSPARENT_COLOR = RGB(0, 0, 0);
const COLORREF BACKGROUND_COLOR = RGB(1, 1, 1);

HANDLE keyCheckHandle;

int myWidth, myHeight;
int monitorWidth, monitorHeight;
HDC hMyDC;

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

    // Setup for drawing
	HDC hMyDC = GetDC(hwnd);
    
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        {
            printf("Goodbye!");
            CloseHandle(keyCheckHandle);
            PostQuitMessage(0);
        }
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            hMyDC = BeginPaint(hwnd, &ps); 

            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hMyDC, &rect, CreateSolidBrush(BACKGROUND_COLOR));

            DrawAscii();

            EndPaint(hwnd, &ps); 
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void DrawAscii()
{
    SetTextColor(hMyDC, TRANSPARENT_COLOR);
    SetBkColor(hMyDC, BACKGROUND_COLOR);

    int i = 0;
    wchar_t myChar;

    for(int y = 0; y < myHeight; y += 13) {
        for(int x = 0; x < myWidth; x += 12){
            // printable ascii range is 32 (space) through 126 (~)
            // myChar = (int) rand() % (126 - 32 + 1) + 32;
            // myChar = charString[(int) rand() % (CHARS)];
            myChar = charString[i++ % (CHARS)]; // print character string in a row
            ExtTextOutW(hMyDC, x, y, ETO_OPAQUE, NULL, &myChar, 1, NULL);
        }
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