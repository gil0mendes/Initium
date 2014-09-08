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
* @brief		EFI Graphics API definitions.
*/

#ifndef __EFI_PROTOCOL_GRAPHICS_H
#define __EFI_PROTOCOL_GRAPHICS_H

#include <efi/api.h>

// Graphics output protocol
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
	{ 0x9042a9de, 0x23dc, 0x4a38, 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a }

// Typedef for efi_graphics_output_protocol structure
typedef struct efi_graphics_output_protocol efi_graphics_output_protocol_t;

// Type of pixel format
typedef enum {
	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask,
	PixelBltOnly,
	PixelFormatMax
} efi_graphics_pixel_format_t;

typedef struct {
	efi_uint32_t            RedMask;
	efi_uint32_t            GreenMask;
	efi_uint32_t            BlueMask;
	efi_uint32_t            ReservedMask;
} efi_pixel_bitmask_t;

// Type for pixel
typedef struct {
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
    uint8_t Reserved;
} efi_uga_pixel_t;

typedef struct {
	efi_uint32_t 					version;
	efi_uint32_t 					horizontalResolution;
	efi_uint32_t 					verticalResolution;
	efi_graphics_pixel_format_t 	pixelFormat;
	efi_pixel_bitmask_t				pixelInformation;
	efi_uint32_t 					pixelsPerScanLine;
} efi_graphics_output_mode_information_t;

// Graphics output mode type defenition
typedef struct {
	efi_uint32_t 							maxMode;
	efi_uint32_t 							mode;
	efi_graphics_output_mode_information_t 	*info;
	efi_uintn_t 							sizeOfInfo;
	efi_physical_address_t					frameBufferBase;
	efi_uintn_t								frameBufferSize;
} efi_graphics_output_protocol_mode_t;

// Type defenition for BLT pixel
typedef struct {
	efi_uint8_t blue;
	efi_uint8_t green;
	efi_uint8_t red;
	efi_uint8_t reserved;
} efi_graphics_output_blt_pixel_t;

// Enum for BLT operations
typedef enum {
	EfiBltVideoFill,
	EfiBltVideoToBltBuffer,
	EfiBltBufferToVideo,
	EfiBltVideoToVideo,
	EfiGraphicsOutputBltOperationMax
} efi_graphics_output_blt_operation_t;

/**
  The following table defines actions for BltOperations:

  <B>EfiBltVideoFill</B> - Write data from the  BltBuffer pixel (SourceX, SourceY)
  directly to every pixel of the video display rectangle
  (DestinationX, DestinationY) (DestinationX + Width, DestinationY + Height).
  Only one pixel will be used from the BltBuffer. Delta is NOT used.

  <B>EfiBltVideoToBltBuffer</B> - Read data from the video display rectangle
  (SourceX, SourceY) (SourceX + Width, SourceY + Height) and place it in
  the BltBuffer rectangle (DestinationX, DestinationY )
  (DestinationX + Width, DestinationY + Height). If DestinationX or
  DestinationY is not zero then Delta must be set to the length in bytes
  of a row in the BltBuffer.

  <B>EfiBltBufferToVideo</B> - Write data from the  BltBuffer rectangle
  (SourceX, SourceY) (SourceX + Width, SourceY + Height) directly to the
  video display rectangle (DestinationX, DestinationY)
  (DestinationX + Width, DestinationY + Height). If SourceX or SourceY is
  not zero then Delta must be set to the length in bytes of a row in the
  BltBuffer.

  <B>EfiBltVideoToVideo</B> - Copy from the video display rectangle (SourceX, SourceY)
  (SourceX + Width, SourceY + Height) .to the video display rectangle
  (DestinationX, DestinationY) (DestinationX + Width, DestinationY + Height).
  The BltBuffer and Delta  are not used in this mode.

  @param  This         Protocol instance pointer.
  @param  BltBuffer    Buffer containing data to blit into video buffer. This
                       buffer has a size of Width*Height*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
  @param  BltOperation Operation to perform on BlitBuffer and video memory
  @param  SourceX      X coordinate of source for the BltBuffer.
  @param  SourceY      Y coordinate of source for the BltBuffer.
  @param  DestinationX X coordinate of destination for the BltBuffer.
  @param  DestinationY Y coordinate of destination for the BltBuffer.
  @param  Width        Width of rectangle in BltBuffer in pixels.
  @param  Height       Hight of rectangle in BltBuffer in pixels.
  @param  Delta        OPTIONAL

  @retval EFI_SUCCESS           The Blt operation completed.
  @retval EFI_INVALID_PARAMETER BltOperation is not valid.
  @retval EFI_DEVICE_ERROR      A hardware error occured writting to the video buffer.
**/
typedef
EFI_STATUS
(EFIAPI *efi_graphics_output_protocol_blt) (
	efi_graphics_output_protocol_t			*this,
	efi_graphics_output_blt_pixel_t			*bltBuffer,
	efi_graphics_output_blt_operation_t		bltOperation,
	efi_uintn_t								sourceX,
	efi_uintn_t								sourceY,
	efi_uintn_t								destinationX,
	efi_uintn_t								destinationY,
	efi_uintn_t								width,
	efi_uintn_t								height,
	efi_uintn_t								delta
);

/**
  Return the current video mode information.

  @param  this              Protocol instance pointer.
  @param  modeNumber        The mode number to be set.

  @retval EFI_SUCCESS       Graphics mode was changed.
  @retval EFI_DEVICE_ERROR  The device had an error and could not complete the request.
  @retval EFI_UNSUPPORTED   ModeNumber is not supported by this device.
**/
typedef
EFI_STATUS
(EFIAPI *efi_graphics_output_protocol_set_mode) (
	efi_graphics_output_protocol_t	*this,
	efi_uint32_t					modeNumber
);

/**
  Return the current video mode information.

  @param  This       Protocol instance pointer.
  @param  ModeNumber The mode number to return information on.
  @param  SizeOfInfo A pointer to the size, in bytes, of the Info buffer.
  @param  Info       A pointer to callee allocated buffer that returns information about ModeNumber.

  @retval EFI_SUCCESS           Mode information returned.
  @retval EFI_BUFFER_TOO_SMALL  The Info buffer was too small.
  @retval EFI_DEVICE_ERROR      A hardware error occurred trying to retrieve the video mode.
  @retval EFI_NOT_STARTED       Video display is not initialized. Call SetMode ()
  @retval EFI_INVALID_PARAMETER One of the input args was NULL.
**/
typedef
EFI_STATUS
(EFIAPI *efi_graphics_output_protocol_query_mode) (
	efi_graphics_output_protocol_t 			*this,
	efi_uint32_t 							ModeNumber,
	efi_uintn_t 							*SizeOfInfo,
	efi_graphics_output_mode_information_t 	**Info
);

// Graphics protocol
struct efi_graphics_output_protocol {
	efi_graphics_output_protocol_query_mode 	queryMode;
	efi_graphics_output_protocol_set_mode		setMode;
	efi_graphics_output_protocol_blt			blt;
	efi_graphics_output_protocol_mode_t 		*mode;
};

#endif // __EFI_PROTOCOL_GRAPHICS_H
