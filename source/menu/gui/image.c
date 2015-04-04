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
* @brief 			Image functions
*/

#include <image.h>
#include <memory.h>
#include <loader.h>

// Internal functions
void gInsertPlane(uint8_t *SrcDataPtr, uint8_t *DestPlanePtr, uint32_t pixelCount);
void gSetPlane(uint8_t *DestPlanePtr, uint8_t Value, uint32_t pixelCount);
void gCopyPlane(uint8_t *SrcPlanePtr, uint8_t *DestPlanePtr, uint32_t pixelCount);

// ============================================================================
// Create a new image buffer
image_t*
gCreateImage(uint32_t width, uint32_t height, bool hasAlpha) {
	// Var for new image
	image_t *newImage = NULL;

	// Allocate de new structure
	newImage = (image_t *) malloc(sizeof(image_t));

	// Check if the newImage are allocated
	if (newImage == NULL) {
		return NULL;
	}

	// Allocate the buffer
	newImage->pixelData = (pixel_t *) malloc(width * height * sizeof(pixel_t));

	// Check if the space for buffer are allocated
	if (newImage->pixelData == NULL) {
		// Free new image and return NULL
		free(newImage);
		return NULL;
	}

	newImage->width = width;
	newImage->height = height;
	newImage->hasAlpha = hasAlpha;

	return newImage;
}

// ============================================================================
// Decompress icns RLE data
void
gDecompressIcnsRLE(uint8_t **ompData, uint32_t *compLen, uint8_t *pixelData,
	uint32_t pixelCount) {

    uint8_t *cp;
    uint8_t *cp_end;
    uint8_t *pp;
    uint32_t pp_left;
    uint32_t len, i;
    uint8_t value;

    // setup variables
    cp = *ompData;
    cp_end = cp + *compLen;
    pp = pixelData;
    pp_left = pixelCount;

    // decode
    while (cp + 1 < cp_end && pp_left > 0) {
        len = *cp++;
        if (len & 0x80) {   // compressed data: repeat next byte
            len -= 125;
            if (len > pp_left)
                break;
            value = *cp++;
            for (i = 0; i < len; i++) {
                *pp = value;
                pp += 4;
            }
        } else {            // uncompressed data: copy bytes
            len++;
            if (len > pp_left || cp + len > cp_end)
                break;
            for (i = 0; i < len; i++) {
                *pp = *cp++;
                pp += 4;
            }
        }
        pp_left -= len;
    }

    if (pp_left > 0) {
        dprintf("gDecompressIcnsRLE: still need %d bytes of pixel data\n", pp_left);
    }

    // record what's left of the compressed data stream
    *ompData = cp;
    *compLen = (uint32_t)(cp_end - cp);
}

// ============================================================================
// Prepare the embedded image to be used
image_t*
gPrepareEmbeddedImage(embedded_image_t *embeddedImage, bool wantAlpha) {
	image_t		*newImage;
	uint8_t		*compData;
	uint32_t	compLen;
	uint32_t	pixelCount;

	// sanity check
	if (embeddedImage->pixelMode > G_MAX_EIPIXELMODE ||
		(embeddedImage->compressMode != G_EICOMPMODE_NONE
			&& embeddedImage->compressMode != G_EICOMPMODE_RLE)) {
		return NULL;
	}

	// allocate image structure and pixel buffer
	newImage = gCreateImage(embeddedImage->width, embeddedImage->height, wantAlpha);
	if (newImage == NULL) {
		return NULL;
	}

	compData 	= (uint8_t *)embeddedImage->data;
	compLen 	= embeddedImage->dataLength;
	pixelCount 	= embeddedImage->width * embeddedImage->height;

	if (embeddedImage->pixelMode == G_EIPIXELMODE_GRAY ||
		embeddedImage->pixelMode == G_EIPIXELMODE_GRAY_ALPHA) {
		// Copy grayscale plane and expand
		if (embeddedImage->compressMode == G_EICOMPMODE_RLE) {
			gDecompressIcnsRLE(&compData, &compLen,  PLPTR(newImage, r), pixelCount);
		} else {
			gInsertPlane(compData, PLPTR(newImage, r), pixelCount);
			compData += pixelCount;
		}

		gCopyPlane(PLPTR(newImage, r), PLPTR(newImage, g), pixelCount);
		gCopyPlane(PLPTR(newImage, r), PLPTR(newImage, b), pixelCount);
	} else if (embeddedImage->pixelMode == G_EIPIXELMODE_COLOR ||
		embeddedImage->pixelMode == G_EIPIXELMODE_COLOR_ALPHA) {
		// copy color planes
		if (embeddedImage->compressMode == G_EICOMPMODE_RLE) {
			gDecompressIcnsRLE(&compData, &compLen, PLPTR(newImage, r), pixelCount);
			gDecompressIcnsRLE(&compData, &compLen, PLPTR(newImage, g), pixelCount);
			gDecompressIcnsRLE(&compData, &compLen, PLPTR(newImage, b), pixelCount);
		} else {
			gInsertPlane(compData, PLPTR(newImage, r), pixelCount);
			compData += pixelCount;
			gInsertPlane(compData, PLPTR(newImage, g), pixelCount);
			compData += pixelCount;
			gInsertPlane(compData, PLPTR(newImage, b), pixelCount);
			compData += pixelCount;
		}
	} else {
		// set color planes to black
		gSetPlane(PLPTR(newImage, r), 0, pixelCount);
		gSetPlane(PLPTR(newImage, g), 0, pixelCount);
		gSetPlane(PLPTR(newImage, b), 0, pixelCount);
	} // if

	if (wantAlpha && (embeddedImage->pixelMode == G_EIPIXELMODE_GRAY_ALPHA ||
		embeddedImage->pixelMode == G_EIPIXELMODE_COLOR_ALPHA ||
		embeddedImage->pixelMode == G_EIPIXELMODE_ALPHA)) {
		// copy alpha plane
		if (embeddedImage->compressMode == G_EICOMPMODE_RLE) {
			gDecompressIcnsRLE(&compData, &compLen, PLPTR(newImage, a), pixelCount);
		} else {
			gInsertPlane(compData, PLPTR(newImage, a), pixelCount);
			compData += pixelCount;
		} // if
	} // if

	return newImage;
}

// misc internal functions ////////////////////////////////////////////////////

void
gInsertPlane(uint8_t *SrcDataPtr, uint8_t *DestPlanePtr, uint32_t pixelCount) {
	uint32_t i;

	for (i = 0; i < pixelCount; i++) {
		*DestPlanePtr = *SrcDataPtr++;
		DestPlanePtr += 4;
	}
}

void
gSetPlane(uint8_t *DestPlanePtr, uint8_t Value, uint32_t pixelCount) {
	uint32_t i;

	for (i = 0; i < pixelCount; i++) {
		*DestPlanePtr = Value;
		DestPlanePtr += 4;
	}
}

void
gCopyPlane(uint8_t *SrcPlanePtr, uint8_t *DestPlanePtr, uint32_t pixelCount) {
	uint32_t i;

	for (i = 0; i < pixelCount; i++) {
		*DestPlanePtr = *SrcPlanePtr;
		DestPlanePtr += 4, SrcPlanePtr += 4;
	}
}
