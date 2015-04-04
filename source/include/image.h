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
* @brief 			Image functions declaration
*/

#ifndef __IMAGE_H
#define __IMAGE_H

#include <types.h>

// Type for pixel
typedef struct {
	uint8_t r, g, b, a;
} pixel_t;

// Type for image
typedef struct {
	uint32_t     width;
	uint32_t     height;
	bool         hasAlpha;
	pixel_t      *pixelData;
} image_t;

// Type for embedded image
typedef struct {
	uint32_t       width;
	uint32_t       height;
	uint32_t       pixelMode;
	uint32_t       compressMode;
	const uint8_t *data;
	uint32_t       dataLength;
} embedded_image_t;

#define G_EIPIXELMODE_GRAY         (0)
#define G_EIPIXELMODE_GRAY_ALPHA   (1)
#define G_EIPIXELMODE_COLOR        (2)
#define G_EIPIXELMODE_COLOR_ALPHA  (3)
#define G_EIPIXELMODE_ALPHA        (4)
#define G_MAX_EIPIXELMODE          G_EIPIXELMODE_ALPHA

#define G_EICOMPMODE_NONE          (0)
#define G_EICOMPMODE_RLE           (1)
#define G_EICOMPMODE_EFICOMPRESS   (2)

#define PLPTR(imagevar, colorname) ((uint8_t *) &((imagevar)->pixelData->colorname))

extern image_t* 	gCreateImage(uint32_t width, uint32_t height, bool hasAlpha);
extern void 		gDecompressIcnsRLE(uint8_t **compData, uint32_t *compLen, uint8_t *pixelData, uint32_t pixelCount);
extern image_t * 	gPrepareEmbeddedImage(embedded_image_t *embeddedImage, bool wantAlpha);

#endif // __IMAGE_H
