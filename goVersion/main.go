// ASCIIScreen redraws the screen with ascii characters.
package main

// #cgo  LDFLAGS: -lgdi32
/*
#include "main_c.c"
*/
import "C"

import (
	"time"
	"fmt"
	"sync"
	// "unsafe"
)

const drawRoutines C.int = 8
const charHeight C.int = 13
const charWidth C.int = 12
const kKeyVirt C.int = 0x4B	// k virtual key code

var mu sync.Mutex
var stepAmount C.int

// used for checking if user presses bound key to exit program
// or other specified function
// only call this function once
func checkKeyStateLoop() {
	var kKeyDown bool

	for {
		time.Sleep(time.Millisecond * 10)

		if C.GetAsyncKeyState(kKeyVirt) < 0 && !kKeyDown {
            kKeyDown = true;

			fmt.Println("Key pressed")

            // check if window is minimized (iconic).
            if (C.IsIconic(C.hwnd) != 0) {
                C.ShowWindow(C.hwnd, C.SW_MAXIMIZE);
            } else {
                C.ShowWindow(C.hwnd, C.SW_MINIMIZE);
            }
        } else if C.GetAsyncKeyState(kKeyVirt) == 0 && kKeyDown {
            kKeyDown = false;
        }
	}
}

// draws ascii from specified start and stop scanline
// can call this function concurrently
func drawASCIIFromPixels(scanStart, scanStop C.int, done chan<- bool) {
 // // Parse pixels for color and then draw le text
	var i C.int
	var myChar C.wchar_t

	for y := scanStart; y < scanStop; y += charHeight {
		for x := C.int(0); x < C.myWidth; x += charWidth {
			var p C.int = (y * C.myWidth) + x;

			// int avgColor = (pPixels[p].rgbRed + pPixels[p].rgbGreen + pPixels[p].rgbBlue) * 0.33;
			// COLORREF colorBlackAndWhite = RGB(avgColor, avgColor, avgColor);
			// SetTextColor(hdcMemDC, colorBlackAndWhite);


			// C.SetTextColor(C.hdcMemDC, C.RGB(C.pPixels[p].rgbRed, C.pPixels[p].rgbGreen, C.pPixels[p].rgbBlue))

			// set background here to make cool large pixely look
			// SetBkColor(hdcMemDC, RGB(pPixels[p].rgbRed, pPixels[p].rgbGreen, pPixels[p].rgbBlue));

			// printable ascii range is 32 (space) through 126 (~)
			// myChar = (int) rand() % (126 - 32 + 1) + 32;
			myChar = C.wchar_t(C.charString[i % (C.CHARS)])

			mu.Lock()
			C.DrawCharacter(&myChar, x, y, C.GetColorFromBuffer(C.pPixels, p))
			mu.Unlock()

			i++
		}
	}

	done <- true
}

func DrawASCII(doneChan chan bool) {
	// take screenshot
	C.StretchBltToMemDC()
	C.GetPixelDataFromMemDC()

	// fill background
    C.SetBkColor(C.hdcMemDC, C.BACKGROUND_COLOR);
    C.Rectangle(C.hdcMemDC, 0, 0, C.myWidth, C.myHeight);

	// redraw screen as ASCII
	for i := C.int(0); i < C.myHeight; i += stepAmount {
		if i + stepAmount < C.myHeight {
			go drawASCIIFromPixels(i, i + stepAmount, doneChan)
		} else {
			go drawASCIIFromPixels(i, C.myHeight, doneChan)
		}
	}

	// wait for draws to finish
	for i := C.int(0); i < C.myHeight; i += stepAmount {
		<-doneChan
	}

	// copy ASCII drawing to this window
	C.BitBltToWindowDC()
}

func main() {
	// wait until C.init done to start drawing / keycheck
	go func () {
		for {
			time.Sleep(time.Millisecond)
			if C.readyToDraw {
				break
			}
		}

		go checkKeyStateLoop()

		var doneChan chan bool = make(chan bool)
		stepAmount = C.myHeight / drawRoutines
		
		// make sure step amount increments by multiple of character height
		if stepAmount % charHeight != 0 {
			stepAmount -= stepAmount % charHeight
		} 

		for {
			if (C.IsIconic(C.hwnd) == 0) {
				DrawASCII(doneChan)
				time.Sleep(time.Millisecond)
			} else {
				time.Sleep(time.Millisecond * 10)
			}
		}
	}()

	// init blocks until user closes window
	C.init()
}