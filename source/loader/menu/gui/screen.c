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
* @brief 			Screen functions for GUI
*/

#include <screen.h>
#include <loader.h>
#include <image.h>
#include "logo.h"

// Screen size preferences
static uint32_t guiScreenWidth = 800;
static uint32_t guiScreenHeight = 600;

// Background pixel
pixel_t guiMenuBackgroundPixel = { 0xbf, 0xbf, 0xbf, 0 };

// Embedded logo image
static embedded_image_t guiLogoImage = {135, 64, G_EIPIXELMODE_COLOR, G_EICOMPMODE_NONE, logo_ppm, 25934};

// ============================================================================
// Clear screen and draw logo
void guiClearScreen(void) {
	static image_t *banner = NULL;

	// Load banner on first call
	if (banner == NULL) {
		banner = gPrepareEmbeddedImage(&guiLogoImage, false);
	}

	// Clear screen
	clearScreen(&guiMenuBackgroundPixel);

	// Draw logo
	drawImage(banner, (guiScreenWidth - banner->width) >> 1,
		((guiScreenHeight - LAYOUT_TOTAL_HEIGHT) >> 1) + LAYOUT_LOGO_HEIGHT - banner->height);
}

// ============================================================================
// Setup screen for use graphics
void
setupScreen(void) {
	// TODO: Get the config value for screen resolution

	// Get resolution
	getScreenSize(&guiScreenWidth, &guiScreenHeight);

	// Clear screen
	guiClearScreen();
}
