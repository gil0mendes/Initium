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
 * @brief 			EFI Screen implementation
 */

#include <efi/efi.h>

#include <screen.h>
#include <loader.h>

// Graphics protocol GUID
static efi_guid_t graphics_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

// Graphics output protocol
static efi_graphics_output_protocol_t *graphics_output = NULL;

// Var to informe if has graphics
static bool hasGraphics = false;

// Screen resolution vars
static efi_uintn_t screen_width  = 800;
static efi_uintn_t screen_height = 600;

// ============================================================================
// Has graphics
//
// @return BOOLEAN
bool
hasGraphicsMode(void) {
	return hasGraphics;
}

// ============================================================================
// Get screen resolution.
//
// @param &uint32_t Reference for screen width
// @param &uint32_t Reference for screen height
void
getScreenSize(uint32_t *screenWidth, uint32_t *screenHeight) {
    if (screenWidth != NULL) {
		*screenWidth = screen_width;
	}
    if (screenHeight != NULL) {
		*screenHeight = screen_height;
	}
}

// ============================================================================
// Sets the screen resolution to the specified value, if possible.
// If the specified value is not valid, displays a warning with the valid
// modes on UEFI systems, or silently fails on EFI 1.x systems. Note that
// this function attempts to set ANY screen resolution, even 0x0 or
// ridiculously large values.
//
// Returns TRUE if successful, FALSE if not.
bool
setScreenSize(uint32_t width_ts, uint32_t height_ts) {
	efi_status_t status = EFI_SUCCESS;
	efi_graphics_output_mode_information_t *info;
	uint32_t modeNum = 0;
	efi_uintn_t size;
	bool modeSet = false;

	if (hasGraphicsMode()) {  // GOP mode UEFI
		// Do a loop through the modes to see if the specified one is available
		// and if so, switch to it...
		while ((status == EFI_SUCCESS) && (!modeSet)) {
			status = efi_call(graphics_output->queryMode, graphics_output, modeNum,
				&size, &info);

				if ((status == EFI_SUCCESS) && (size >= sizeof(*info)) &&
					(info->horizontalResolution == width_ts) &&
					(info->verticalResolution == height_ts)) {
					status = efi_call(graphics_output->setMode, graphics_output, modeNum);
					modeSet = (status == EFI_SUCCESS);
				} // if
				modeNum++;
		} // while

		if (modeSet) {
			screen_width = width_ts;
			screen_height = height_ts;
		} else {
			// If unsuccessful, display an error message for the user....
			dprintf("screen: Error setting mode %dx%d; using default mode!\nAvailable modes are:\n",
				screen_width, screen_height);
			modeNum = 0;
			status = EFI_SUCCESS;

			while (status == EFI_SUCCESS) {
				status = efi_call(graphics_output->queryMode, graphics_output,
					modeNum, &size, &info);
					if ((status == EFI_SUCCESS) && (size >= sizeof(*info))) {
						dprintf("screen: Mode %d: %dx%d\n", modeNum,
							info->horizontalResolution, info->verticalResolution);
					} // if
					modeNum++;
			} // while()
		}// if
	}

	return (modeSet);
}


// ============================================================================
// Initialize the EFI graphics
void
screenInit(void) {
	// For handles
	efi_handle_t *handles;
	efi_uintn_t num_handles;
	efi_status_t status = EFI_SUCCESS;

	// Look for a graphics capability
	status = efi_locate_handle(EFI_BY_PROTOCOL, &graphics_guid, NULL, &handles,
		&num_handles);
	if (status != EFI_SUCCESS) {
		dprintf("screen: Graphics not supported");
		return;
	}

	// Just use the first handle
	status = efi_open_protocol(handles[0], &graphics_guid,
		EFI_OPEN_PROTOCOL_GET_PROTOCOL, (void **)&graphics_output);
	if (status != EFI_SUCCESS) {
		dprintf("screen: handler can't be used");
		return;
	}

	// Get screen info
	hasGraphics = false;
	if (graphics_output != NULL) {
		screen_width = graphics_output->mode->info->horizontalResolution;
		screen_height = graphics_output->mode->info->verticalResolution;
		hasGraphics = true;
	}
}

// Drawing to the screen //////////////////////////////////////////////////////

// ============================================================================
// Clear the screen
void
clearScreen(pixel_t *color) {
	efi_uga_pixel_t fill_color;

	// Check if have graphics mode
	if (!hasGraphicsMode()) {
		return;
	}

	// Make efi uga pixel
	fill_color.Red 			= color->r;
	fill_color.Green 		= color->g;
	fill_color.Blue 		= color->b;
	fill_color.Reserved 	= 1;

	/* TODO: Fix this on future
	efi_call(graphics_output->blt, graphics_output,
		(efi_graphics_output_blt_pixel_t *)&fill_color,
		EfiBltVideoFill, 0, 0, 0, 0, screen_width, screen_height, 0);*/
}

// ============================================================================
// Draw image
//
// @param image_t 		Image buffer
// @param uint32_t		X screen position
// @param uint32_t		Y screen position
void
drawImage(image_t *image, uint32_t posX, uint32_t posY) {
	// Check if have graphics mode
	if (!hasGraphicsMode()) {
		return;
	}

	// Check if have Alpha
	if (image->hasAlpha) {
		image->hasAlpha = false;
		// TODO: Set color for pane
	}

	// Call EFI function
	/* TODO: Fix this on future
	efi_call(graphics_output->blt, graphics_output,
		(efi_graphics_output_blt_pixel_t *)image->pixelData, EfiBltBufferToVideo,
		0, 0, posX, posY, image->width, image->height, 0);*/
}
