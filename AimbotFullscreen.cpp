//#include <wingdi.h>
#include <windows.h>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>


using namespace std;

void MouseMove (int x, int y )
{
  //double fScreenWidth    = ::GetSystemMetrics( SM_CXSCREEN )-1;
  //double fScreenHeight  = ::GetSystemMetrics( SM_CYSCREEN )-1;
  //double fx = x*(65535.0f/fScreenWidth);
  //double fy = y*(65535.0f/fScreenHeight);
  INPUT  Input={0};
  Input.type      = INPUT_MOUSE;
  Input.mi.dwFlags  = MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE;
  Input.mi.dx = x;
  Input.mi.dy = y;
  ::SendInput(1,&Input,sizeof(INPUT));
}

void DrawAscii()
{

    int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    HWND hDesktopWnd = GetDesktopWindow();
    HDC hDesktopDC = GetDC(NULL);
    //HDC hDesktopDC =  CreateDC("DISPLAY", NULL, NULL, NULL);
    HDC hMyDC = CreateCompatibleDC(hDesktopDC);

    HBITMAP hMyBmp =::CreateCompatibleBitmap(hMyDC, nScreenWidth, nScreenHeight);
    HBITMAP hOlBmp = (HBITMAP)::SelectObject(hMyDC,hMyBmp);
    Rectangle(hMyDC,0,0,nScreenWidth,nScreenHeight);
    SetBkColor(hDesktopDC, RGB(0, 0, 0));
    SetTextColor(hDesktopDC, RGB(25, 1, 1));

    LPCSTR s = "Hello";
    TextOutA(hMyDC, 0, 0, s, strlen(s));

    //HDC hCalcDC = ::GetDC(hwnd);
    ::BitBlt(hDesktopDC,0,0,nScreenWidth,nScreenHeight, hMyDC,0,0,SRCCOPY);

    ::SelectObject(hMyDC,hOlBmp);
    ::DeleteObject(hMyBmp);
    ::DeleteDC(hMyDC);
    ::ReleaseDC(hDesktopWnd,hDesktopDC);

    /*
    HWND hwnd = ::FindWindow(0,"Minecraft 1.16.5 - Singleplayer");
      if (hwnd )
      {
        HDC hdc =  ::GetDC(0);
        HDC hMyDC = ::CreateCompatibleDC(hdc);
        HBITMAP hMyBmp =::CreateCompatibleBitmap(hMyDC, 100,100);
        HBITMAP hOlBmp = (HBITMAP)::SelectObject(hMyDC,hMyBmp);
      //  ::Rectangle(hMyDC,0,0,100,100);
        SetTextColor(hMyDC, RGB(1, 1, 1));
        SetBkColor(hMyDC, RGB(0, 255, 0));

        HDC hCalcDC = ::GetDC(hwnd);
        ::BitBlt(hCalcDC,0,0,100,100, hMyDC,0,0,SRCCOPY);

        ::SelectObject(hMyDC,hOlBmp);
        ::DeleteObject(hMyBmp);
        ::DeleteDC(hMyDC);
        ::ReleaseDC(hwnd,hCalcDC);
        ::ReleaseDC(0,hdc);

      }
*/
  //  return pX, pY;
}

int main(){
  int where = 0;

  start:
    DrawAscii();
    //Maybe take this pause out
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

      if(GetAsyncKeyState(75) < 0)   //75 = 'k'
        {
          //cout << "stop";
          goto inBetween;
        }else{
          goto start;
        }


        inBetween:
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          if(GetAsyncKeyState(75) == 0){
            //CaptureScreen();
            if(where == 0){
              where = 1;
              goto stop;
            }else{
              where = 0;
              goto start;
            }

          }
        goto inBetween;




  stop:
    if(GetAsyncKeyState(75) < 0)
      {
        //cout << "start";
        goto inBetween;
      }else{
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        goto stop;
      }






}
