/**
* The MIT License (MIT)
*
* Copyright (c) 2014 Gil Mendes
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/**
 * @file
 * @brief 			Screen functions
 */

#ifndef __SCREEN_H
#define __SCREEN_H

#include <types.h>
#include <image.h>

#define LAYOUT_TOTAL_HEIGHT (368)
#define LAYOUT_LOGO_HEIGHT (32)
#define LAYOUT_LOGO_YOFFSET (LAYOUT_LOGO_HEIGHT + 32)

extern void drawImage(image_t *image, uint32_t posX, uint32_t posY);
extern void clearScreen(pixel_t *color);
extern void getScreenSize(uint32_t *screenWidth, uint32_t *screenHeight);
extern bool setScreenSize(uint32_t screen_width, uint32_t screen_height);
extern bool hasGraphicsMode(void);
extern void screenInit(void);

// -----------
extern void defineScreenResolution(uint32_t width, uint32_t height);
extern void setupScreen(void);

#endif // __SCREEN_H
