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

//
// Provides a minimal interface for file-type access to a device
//

// GUID
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
	{0x0964e5b22, 0x6459, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}

// Revision Number
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION 0x00010000

// Protocol Interface Structure
INTERFACE_DECL(_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL);
INTERFACE_DECL(_EFI_FILE_PROTOCOL);

// ============================================================================
// Opens the root directory on a volume
//
// @param This 			A pointer to the volume to open the root directory of
// @param Root 			A pointer to the location to return the opened file handle
// 		for the root directory
//
// @retval EFI_SUCCESS 					The file volume was opened.
// @retval EFI_UNSUPPORTED 			The volume does not support the requested file
// 		system type.
// @retval EFI_NO_MEDIA 				The device has no medium.
// @retval EFI_DEVICE_ERROR 		The device reported an error.
// @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
// @retval EFI_ACCESS_DENIED 		The service denied access to the file.
// @retval EFI_OUT_OF_RESOURCES The file volume was not opened.
// @retval EFI_MEDIA_CHANGED 		The device has a different medium in it or the
// 		medium is no longer supported. Any existing file handles for this volume
// 		are no longer valid. To access the files on the new medium, the volume
// 		must be reopened with OpenVolume().
typedef
efi_status_t
(__efiapi *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (
	IN struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
	OUT struct _EFI_FILE_PROTOCOL **Root
	);

// ============================================================================
// Provides a minimal interface for file-type access to a device. This protocol
// is only supported on some devices
//
// @param Revision			The version of the EFI_FILE_PROTOCOL
// @param OpenVolume		Opens the volume for file I/O access
typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
	UINT64 Revision;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

//
// Provides file based access to support file systems
//

#define EFI_FILE_PROTOCOL_REVISION					0x00010000
#define EFI_FILE_PROTOCOL_REVISION2					0x00020000
#define EFI_FILE_PROTOCOL_LATEST_REVISION 	EFI_FILE_PROTOCOL_REVISION2

// Open modes
#define EFI_FILE_MODE_READ      0x0000000000000001
#define EFI_FILE_MODE_WRITE     0x0000000000000002
#define EFI_FILE_MODE_CREATE    0x8000000000000000

// File attributes
#define EFI_FILE_READ_ONLY      0x0000000000000001
#define EFI_FILE_HIDDEN         0x0000000000000002
#define EFI_FILE_SYSTEM         0x0000000000000004
#define EFI_FILE_RESERVIED      0x0000000000000008
#define EFI_FILE_DIRECTORY      0x0000000000000010
#define EFI_FILE_ARCHIVE        0x0000000000000020
#define EFI_FILE_VALID_ATTR     0x0000000000000037

// Token type definition
typedef struct {
	EFI_EVENT 		Event;
	efi_status_t		Status;
	UINTN					BufferSize;
	VOID					*Buffer;
} EFI_FILE_IO_TOKEN;

// ============================================================================
// Opens a new file relative to the source file’s location
//
// @param This					A pointer to the EFI_FILE_PROTOCOL instance that is the
// 		file handle to the source location
// @param NewHandle			A pointer to the location to return the opened handle
//		for the new file
// @param FileName			The Null-terminated string of the name of the file to be
//		opened. The file name may contain the following path modifiers: “\”, “.”,
//		and “..”
// @param OpenMode			The mode to open the file. The only valid combinations
//		that the file may be opened with are: Read, Read/Write, or Create/Read/
//		Write.
// @param Attributes		Only valid for EFI_FILE_MODE_CREATE, in which case
//		these are the attribute bits for the newly created file
//
// @retval EFI_SUCCESS 					The file was opened.
// @retval EFI_NOT_FOUND 				The specified file could not be found on
//		the device.
// @retval EFI_NO_MEDIA 				The device has no medium.
// @retval EFI_MEDIA_CHANGED 		The device has a different medium in it or the
//		medium is no longer supported.
// @retval EFI_DEVICE_ERROR 		The device reported an error.
// @retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
// @retval EFI_WRITE_PROTECTED 	An attempt was made to create a file, or open a
//		file for write when the media is write-protected.
// @retval EFI_ACCESS_DENIED 		The service denied access to the file.
// @retval EFI_OUT_OF_RESOURCES Not enough resources were available to open
//		the file.
// @retval EFI_VOLUME_FULL 			The volume is full.
typedef
efi_status_t
(__efiapi *EFI_FILE_OPEN) (
	IN struct _EFI_FILE_PROTOCOL  *This,
	OUT struct _EFI_FILE_PROTOCOL **NewHandle,
	IN CHAR16                   	*FileName,
	IN UINT64                   	OpenMode,
	IN UINT64                   	Attributes
	);

// ============================================================================
// Closes a specified file handle
//
// @param This					A pointer to the EFI_FILE_PROTOCOL instance that is
//		the file handle to close
//
// @retval EFI_SUCCESS The file was closed.
typedef
efi_status_t
(__efiapi *EFI_FILE_CLOSE) (
    IN struct _EFI_FILE_PROTOCOL  *This
    );

// ============================================================================
// Closes and deletes a file
//
// @param This 					A pointer to the EFI_FILE_PROTOCOL instance that is the
//		handle to the file to delete
//
// @param EFI_SUCCESS 							The file was closed and deleted, and the
//		handle was closed.
// @param EFI_WARN_DELETE_FAILURE 	The handle was closed, but the file was
//		not deleted.
typedef
efi_status_t
(__efiapi *EFI_FILE_DELETE) (
    IN struct _EFI_FILE_PROTOCOL  *This
    );

// ============================================================================
// Reads data from a file
//
// @param This				A pointer to the EFI_FILE_PROTOCOL instance that is the
// 		file handle to read data from
// @param BufferSize	On input, the size of the Buffer. On output, the amount
//		of data returned in Buffer. In both cases, the size is measured in bytes
// @param Buffer			The buffer into which the data is read
//
// @retval EFI_SUCCESS 						The data was read.
// @retval EFI_NO_MEDIA 					The device has no medium.
// @retval EFI_DEVICE_ERROR 			The device reported an error.
// @retval EFI_DEVICE_ERROR 			An attempt was made to read from a
//		deleted file.
// @retval EFI_DEVICE_ERROR 			On entry, the current file position is
//		beyond the end of the file.
// @retval EFI_VOLUME_CORRUPTED 	The file system structures are corrupted.
// @retval EFI_BUFFER_TOO_SMALL 	The BufferSize is too small to read the
//		current directory entry. BufferSize has been updated with the size
//		needed to complete the request.
typedef
efi_status_t
(__efiapi *EFI_FILE_READ) (
    IN struct _EFI_FILE_PROTOCOL  *This,
    IN OUT UINTN                	*BufferSize,
    OUT VOID                    	*Buffer
    );

// ============================================================================
// Writes data to a file
//
// @param This				A pointer to the EFI_FILE_PROTOCOL instance that is the
//		file handle to write data to
// @param BufferSize	On input, the size of the Buffer. On output, the amount
//		of data actually written. In both cases, the size is measured in bytes
// @param Buffer			The buffer of data to write
//
// @retval EFI_SUCCESS 						The data was written.
// @retval EFI_UNSUPPORT 					Writes to open directory files are not supported.
// @retval EFI_NO_MEDIA 					The device has no medium.
// @retval EFI_DEVICE_ERROR 			The device reported an error.
// @retval EFI_DEVICE_ERROR 			An attempt was made to write to a deleted file.
// @retval EFI_VOLUME_CORRUPTED 	The file system structures are corrupted.
// @retval EFI_WRITE_PROTECTED 		The file or medium is write-protected.
// @retval EFI_ACCESS_DENIED 			The file was opened read only.
// @retval EFI_VOLUME_FULL 				The volume is full.
typedef
efi_status_t
(__efiapi *EFI_FILE_WRITE) (
    IN struct _EFI_FILE_PROTOCOL  *This,
    IN OUT UINTN                	*BufferSize,
    IN VOID                     	*Buffer
    );

// ============================================================================
// Opens a new file relative to the source directory’s location
//
// @param This				A pointer to the EFI_FILE_PROTOCOL instance that is the
//		file handle to read data from
// @param NewHandle		A pointer to the location to return the opened handle for
//		the new file.SeethetypeEFI_FILE_PROTOCOLdescription. For asynchronous I/O,
//		this pointer must remain valid for the duration of the
//		asynchronous operation
// @param FileName		The Null-terminated string of the name of the file to
//		be opened. The file name may contain the following path
//		modifiers: “\”, “.”, and “..”
// @param OpenMode		The mode to open the file. The only valid combinations
//		that the file may be opened with are: Read, Read/Write, or
//		Create/Read/ Write
// @param Attributes	Only valid for EFI_FILE_MODE_CREATE, in which case
//		these are the attribute bits for the
// @param Token				A pointer to the token associated with the transaction
//
// @retval EFI_SUCCESS 						Returned from the call OpenEx() If Event is
//		NULL (blocking I/O): The file was opened successfully. If Event is not
//		NULL (asynchronous I/O): The request was successfully queued for
//		processing. Event will be signaled upon completion. Returned in the
//		token after signaling Event The file was opened successfully.
// @retval EFI_NOT_FOUND 					The device has no medium.
// @retval EFI_NO_MEDIA 					The specified file could not be found
//		on the device.
// @retval EFI_VOLUME_CORRUPTED 	The file system structures are corrupted.
// @retval EFI_WRITE_PROTECTED 		An attempt was made to create a file,
//		or open a file for write when the media is write-protected.
// @retval EFI_ACCESS_DENIED 			The service denied access to the file.
// @retval EFI_OUT_OF_RESOURCES 	Unable to queue the request or open the
//		file due to lack of resources.
// @retval EFI_VOLUME_FULL 				The volume is full.
typedef
efi_status_t
(__efiapi *EFI_FILE_OPEN_EX) (
	IN struct _EFI_FILE_PROTOCOL			*This,
	OUT struct _EFI_FILE_PROTOCOL			**NewHandle,
	IN CHAR16 												*FileName,
	IN UINT64 												OpenMode,
	IN UINT64 												Attributes,
	IN OUT EFI_FILE_IO_TOKEN 					*Token
	);

// ============================================================================
//
typedef
efi_status_t
(__efiapi *EFI_FILE_READ_EX) (
	IN struct _EFI_FILE_PROTOCOL 	*This,
	IN OUT EFI_FILE_IO_TOKEN 	  	*Token
	);

// ============================================================================
//
typedef
efi_status_t
(__efiapi *EFI_FILE_WRITE_EX) (
	IN struct _EFI_FILE_PROTOCOL 			*This,
	IN OUT EFI_FILE_IO_TOKEN 	*Token
	);

// ============================================================================
//
typedef
efi_status_t
(__efiapi *EFI_FILE_FLUSH_EX) (
	IN struct _EFI_FILE_PROTOCOL 			*This,
	IN OUT EFI_FILE_IO_TOKEN 	*Token
	);

// ============================================================================
//
//
// @param This
// @param Position
typedef
efi_status_t
(__efiapi *EFI_FILE_SET_POSITION) (
    IN struct _EFI_FILE_PROTOCOL  *This,
    IN UINT64                   	Position
    );

// ============================================================================
//
typedef
efi_status_t
(__efiapi *EFI_FILE_GET_POSITION) (
    IN struct _EFI_FILE_PROTOCOL  *This,
    OUT UINT64                  	*Position
    );

// ============================================================================
//
typedef
efi_status_t
(__efiapi *EFI_FILE_GET_INFO) (
    IN struct _EFI_FILE_PROTOCOL  *This,
    IN EFI_GUID                 	*InformationType,
    IN OUT UINTN                	*BufferSize,
    OUT VOID                    	*Buffer
    );

// ============================================================================
//
typedef
efi_status_t
(__efiapi *EFI_FILE_SET_INFO) (
    IN struct _EFI_FILE_PROTOCOL  *This,
    IN EFI_GUID                 	*InformationType,
    IN UINTN                    	BufferSize,
    IN VOID                     	*Buffer
    );

// ============================================================================
//
typedef
efi_status_t
(__efiapi *EFI_FILE_FLUSH) (
    IN struct _EFI_FILE_PROTOCOL  *This
    );

typedef struct _EFI_FILE_PROTOCOL {
    UINT64                  Revision;
    EFI_FILE_OPEN           Open;
    EFI_FILE_CLOSE          Close;
    EFI_FILE_DELETE         Delete;
    EFI_FILE_READ           Read;
    EFI_FILE_WRITE          Write;
    EFI_FILE_GET_POSITION   GetPosition;
    EFI_FILE_SET_POSITION   SetPosition;
    EFI_FILE_GET_INFO       GetInfo;
    EFI_FILE_SET_INFO       SetInfo;
    EFI_FILE_FLUSH          Flush;
		EFI_FILE_OPEN_EX				OpenEx;				// Added for revision 2
		EFI_FILE_READ_EX 				ReadEx;				// Added for revision 2
		EFI_FILE_WRITE_EX				WriteEx;			// Added for revision 2
		EFI_FILE_FLUSH_EX 			FlushEx;			// Added for revision 2
} EFI_FILE, *EFI_FILE_PROTOCOL;

//
// EFI_FILE_INFO
//
// Provides a GUID and a data structure that can be used with
// EFI_FILE_PROTOCOL.SetInfo() and EFI_FILE_PROTOCOL.GetInfo()
//

#define EFI_FILE_INFO_ID \
	{0x09576e92, 0x6d3f, 0x11d2, 0x8e39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}

typedef struct {
	UINT64 					Size;
	UINT64 					FileSize;
	UINT64 					PhysicalSize;
	EFI_TIME 				CreateTime;
	EFI_TIME 				LastAcessTime;
	EFI_TIME 				ModificationTime;
	UINT64					Attribute;
	CHAR16 					FileName[];
} EFI_FILE_INFO;

//
// EFI_FILE_SYSTEM_INFO
//
// Provides a GUID and a data structure that can be used with
// EFI_FILE_PROTOCOL.SetInfo() and EFI_FILE_PROTOCOL.GetInfo()
// to set the system volume's volume label
//

//
// The FileName field of the EFI_FILE_INFO data structure is variable length.
// Whenever code needs to know the size of the EFI_FILE_INFO data structure, it needs to
// be the size of the data structure without the FileName field.  The following macro
// computes this size correctly no matter how big the FileName array is declared.
// This is required to make the EFI_FILE_INFO data structure ANSI compilant.
//

#define SIZE_OF_EFI_FILE_INFO EFI_FIELD_OFFSET(EFI_FILE_INFO,FileName)

#define EFI_FILE_SYSTEM_INFO_ID \
	{0x09576e93, 0x6d3f, 0x11d2, 0x8e39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}

typedef struct {
	UINT64 					Size;
	BOOLEAN 				ReadOnly;
	UINT64 					VolumeSize;
	UINT64 					FreeSpace;
	UINT32 					BlockSize;
	CHAR16 					VolumeLabel[];
} EFI_FILE_SYSTEM_INFO;

//
// EFI_FILE_SYSTEM_VOLUME_LABEL
//
// Provides a GUID and a data strucutre that can be used with
// EFI_FILE_PROTOCOL.SetInfo() and EFI_FILE_PROTOCOL.GetInfo()
// to get or set information about system's volume label.
//

//
// The VolumeLabel field of the EFI_FILE_SYSTEM_INFO data structure is variable length.
// Whenever code needs to know the size of the EFI_FILE_SYSTEM_INFO data structure, it needs
// to be the size of the data structure without the VolumeLable field.  The following macro
// computes this size correctly no matter how big the VolumeLable array is declared.
// This is required to make the EFI_FILE_SYSTEM_INFO data structure ANSI compilant.
//

#define SIZE_OF_EFI_FILE_SYSTEM_INFO EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_INFO,VolumeLabel)

#define EFI_FILE_SYSTEM_VOLUME_LABEL_ID \
	{0xDB47D7D3, 0xFE81, 0x11d3, 0x9A35, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}

typedef struct {
	CHAR16 			VolumeLabel[1];
} EFI_FILE_SYSTEM_VOLUME_LABEL;

#define SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL_INFO EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_VOLUME_LABEL,VolumeLabel)
