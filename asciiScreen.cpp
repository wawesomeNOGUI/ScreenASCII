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
HWND hwnd = ::FindWindow(0,"Minecraft 1.16.5");
//HWND hwnd = ::FindWindow(0,"Command Prompt - a.exe");
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

//wchar_t b = 0x25A0;  //square
//wchar_t b = '\u2532';
wchar_t b = 'X';
//wchar_t b = 0x6F22;

void search(){
  b++;
}

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

  //char buffer [33];
  //itoa(120, buffer, 8);

  LPCWSTR s = &b;

//SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
  //SetBkColor(hMyDC, RGB(0, 0, 0));

  Rectangle(hMyDC,0,0,nScreenWidth,nScreenHeight);

//EndPaint(hDesktopWnd , &ps);

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

SetTextColor(hMyDC, RGB(50, 1, 1));


auto t1 = high_resolution_clock::now();

  // Parse pixels for color and then draw le text
//  for(int i = 0; i<1000; i++){
//    TextOutW(hMyDC, i*10, 25, s, 1);
    //cout << *s;
//  }
    for(int y = 0; y < nScreenHeight; y+=12){
        for(int x = 0; x < nScreenWidth; x+=12){
            int p = (nScreenHeight-y-1)*nScreenWidth+x; // flip the right way, 0,0 in top left
            SetTextColor(hMyDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));
            //SetTextColor(hMyDC, RGB(1, 1, 1));
            //TextOutA(hMyDC, x, y, s, strlen(s));
            //TextOutW(hMyDC, x, y, s, wcslen(s));  //only use wcslen for a string, not a single character

            //TextOutW(hMyDC, x, y, s, 1);
            SetBkColor(hMyDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));
            ExtTextOutW(hMyDC, x, y, ETO_OPAQUE, NULL, s, 1, NULL);


       }
    }

auto t2 = high_resolution_clock::now();

  //  auto ms_int = duration_cast<milliseconds>(t2 - t1);
duration<double, std::milli> ms_double = t2 - t1;
    //cout << ms_double.count() << "ms\n";


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
    //search();
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
