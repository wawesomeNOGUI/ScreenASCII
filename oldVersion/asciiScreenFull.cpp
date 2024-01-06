//#include <wingdi.h>
#include <windows.h>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>


using namespace std;

int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
HWND hDesktopWnd = GetDesktopWindow();
HDC hDesktopDC = GetDC(NULL);
//HDC hDesktopDC =  CreateDC("DISPLAY", NULL, NULL, NULL);
HDC hMyDC = CreateCompatibleDC(hDesktopDC);
HBITMAP hMyBmp = CreateCompatibleBitmap(hMyDC, nScreenWidth, nScreenHeight);
HBITMAP hOlBmp = (HBITMAP)::SelectObject(hMyDC,hMyBmp);

RGBQUAD *pPixels = new RGBQUAD[nScreenWidth * nScreenHeight];

void DrawAscii()
{
  // CaptureScreen
  BITMAPINFO bmi = {0};
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = nScreenWidth;
  bmi.bmiHeader.biHeight = nScreenHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
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

  LPCSTR s = "H";

  Rectangle(hMyDC,0,0,nScreenWidth,nScreenHeight);
  SetBkColor(hDesktopDC, RGB(0, 26, 0));

  // Parse pixels for color and then draw le text
    for(int y = 0; y < nScreenHeight; y+=12){
        for(int x = 0; x < nScreenWidth; x+=12){
            int p = (nScreenHeight-y-1)*nScreenWidth+x; // flip the right way, 0,0 in top left

            SetTextColor(hDesktopDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));
            //SetTextColor(hDesktopDC, RGB(1, 25, 1));
            TextOutA(hMyDC, x, y, s, strlen(s));

        }
    }

    exitLoop:;

    //Draw Everything
    ::BitBlt(hDesktopDC,0,0,nScreenWidth,nScreenHeight, hMyDC,0,0,SRCCOPY);

    ::SelectObject(hMyDC,hOlBmp);
    ::DeleteObject(hMyBmp);
    ::DeleteDC(hMyDC);
    ::ReleaseDC(hDesktopWnd,hDesktopDC);

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
