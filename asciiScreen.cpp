//#include <wingdi.h>
#include <windows.h>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>


using namespace std;

int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
//HWND hDesktopWnd = GetDesktopWindow();
HWND hwnd = ::FindWindow(0,"Minecraft 1.16.5 - Multiplayer (3rd-party Server)");
//HWND hwnd = ::FindWindow(0,"Rocket League (64-bit, DX11, Cooked)");
HDC hDesktopDC = GetDC(NULL);
//HDC hDesktopDC =  CreateDC("DISPLAY", NULL, NULL, NULL);
HDC hMyDC = CreateCompatibleDC(hDesktopDC);
HBITMAP hMyBmp = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
HBITMAP hOlBmp = (HBITMAP)::SelectObject(hMyDC,hMyBmp);


RGBQUAD *pPixels = new RGBQUAD[nScreenWidth * nScreenHeight];
BITMAPINFO bmi = {0};

//Color For background
const COLORREF backgroundColor = 0x00000000;


void DrawAscii()
{
  // CaptureScreen
  HDC appDC = ::GetDC(hwnd);
  BitBlt(hMyDC, 0, 0, nScreenWidth, nScreenHeight, appDC, 0,0, SRCCOPY);

  GetDIBits(
      hMyDC,
      hMyBmp,
      0,
      nScreenHeight,
      pPixels,
      &bmi,
      DIB_RGB_COLORS
  );

  // Draw ASCII
  //LPCSTR s [nScreenWidth/8 * nScreenHeight/8];  //array to store characters

  char buffer [33];
  //LPCSTR s = itoa(254, buffer, 16);
  LPCSTR s = ".";

//BeginPaint(hDesktopWnd , &ps);

            // Because the default brush is white, select
            // a different brush into the device context

//SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
    SetBkColor(hMyDC, RGB(0, 0, 0));

  Rectangle(hMyDC,0,0,nScreenWidth,nScreenHeight);

//EndPaint(hDesktopWnd , &ps);


  // Parse pixels for color and then draw le text
    for(int y = 0; y < nScreenHeight; y+=12){
        for(int x = 0; x < nScreenWidth; x+=12){
            int p = (nScreenHeight-y-1)*nScreenWidth+x; // flip the right way, 0,0 in top left
            SetTextColor(hMyDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));
            //SetTextColor(hMyDC, RGB(1, y, 1));
            TextOutA(hMyDC, x, y, s, strlen(s));

        }
    }

    exitLoop:;


    //Draw Everything
    //HDC appDC = ::GetDC(hwnd);
    //::BitBlt(appDC,100,100,nScreenWidth,nScreenHeight, hMyDC,0,0,SRCCOPY);
    ::BitBlt(hDesktopDC,nScreenWidth/2,0,nScreenWidth/2,nScreenHeight/2+250, hMyDC,0,0,SRCCOPY);
    //::SelectObject(hMyDC,hOlBmp);
    //::DeleteObject(hMyBmp);
    //::DeleteDC(hMyDC);
    //::ReleaseDC(hDesktopWnd,hDesktopDC);

  //  delete [] pPixels;

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
  //setBackgroundColor
  SelectObject(hMyDC, CreateSolidBrush(backgroundColor));
  int where = 0;

  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = nScreenWidth;
  bmi.bmiHeader.biHeight = nScreenHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  start:
    DrawAscii();
    //Maybe take this pause out
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));

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
