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
 * @brief		EFI API definitions.
 */

#ifndef __EFI_API_H
#define __EFI_API_H

#include <lib/utility.h>

#include <efi/arch/api.h>

//
// Basic EFI definitions.
//

// EFI calling convention attribute
#ifndef EFIAPI
# define EFIAPI
#endif

/** Basic integer types. */
typedef unsigned char efi_boolean_t;
#ifdef CONFIG_64BIT
typedef int64_t   efi_intn_t;
typedef uint64_t  efi_uintn_t;
#else
typedef int32_t   efi_intn_t;
typedef uint32_t  efi_uintn_t;
#endif
typedef int8_t    efi_int8_t;
typedef uint8_t   efi_uint8_t;
typedef int16_t   efi_int16_t;
typedef uint16_t  efi_uint16_t;
typedef int32_t   efi_int32_t;
typedef uint32_t  efi_uint32_t;
typedef int64_t   efi_int64_t;
typedef uint64_t  efi_uint64_t;

// Other basic types
typedef efi_intn_t    EFI_STATUS;
typedef efi_uint64_t  EFI_LBA;
typedef efi_uintn_t   EFI_TPL;
typedef void          *EFI_HANDLE;
typedef void          *EFI_EVENT;

typedef uint8_t     efi_char8_t;
typedef uint16_t    efi_char16_t;
typedef uint8_t efi_mac_address_t[32];
typedef uint8_t efi_ipv4_address_t[4];
typedef uint8_t efi_ipv6_address_t[16];
typedef uint8_t efi_ip_address_t[16] __aligned(4);
typedef efi_uint64_t efi_physical_address_t;
typedef efi_uint64_t efi_virtual_address_t;

//
// Prototype argument decoration for EFI parameters to indicate
// their direction
//
// IN 			- argument is passed into the function
// OUT 			- argument (pointer) is returned from the function
// OPTIONAL - argument is optional
//
#ifndef IN
    #define IN
    #define OUT
    #define OPTIONAL
#endif


/** EFI GUID structure. */
typedef struct efi_guid {
	efi_uint32_t data1;
	efi_uint16_t data2;
	efi_uint16_t data3;
	efi_uint8_t data4;
	efi_uint8_t data5;
	efi_uint8_t data6;
	efi_uint8_t data7;
	efi_uint8_t data8;
	efi_uint8_t data9;
	efi_uint8_t data10;
	efi_uint8_t data11;
} __aligned(8) efi_guid_t;

/**
 * EFI status codes.
 */

/** Define an EFI error code (high bit set). */
#define EFI_ERROR(value)		\
	((((EFI_STATUS)1) << (BITS(EFI_STATUS) - 1)) | (value))

/** Define an EFI warning code (high bit clear). */
#define EFI_WARNING(value)			(value)

/** EFI success codes. */
#define EFI_SUCCESS				0

/** EFI error codes. */
#define EFI_LOAD_ERROR				EFI_ERROR(1)
#define EFI_INVALID_PARAMETER			EFI_ERROR(2)
#define EFI_UNSUPPORTED				EFI_ERROR(3)
#define EFI_BAD_BUFFER_SIZE			EFI_ERROR(4)
#define EFI_BUFFER_TOO_SMALL			EFI_ERROR(5)
#define EFI_NOT_READY				EFI_ERROR(6)
#define EFI_DEVICE_ERROR			EFI_ERROR(7)
#define EFI_WRITE_PROTECTED			EFI_ERROR(8)
#define EFI_OUT_OF_RESOURCES			EFI_ERROR(9)
#define EFI_VOLUME_CORRUPTED			EFI_ERROR(10)
#define EFI_VOLUME_FULL				EFI_ERROR(11)
#define EFI_NO_MEDIA				EFI_ERROR(12)
#define EFI_MEDIA_CHANGED			EFI_ERROR(13)
#define EFI_NOT_FOUND				EFI_ERROR(14)
#define EFI_ACCESS_DENIED			EFI_ERROR(15)
#define EFI_NO_RESPONSE				EFI_ERROR(16)
#define EFI_NO_MAPPING				EFI_ERROR(17)
#define EFI_TIMEOUT				EFI_ERROR(18)
#define EFI_NOT_STARTED				EFI_ERROR(19)
#define EFI_ALREADY_STARTED			EFI_ERROR(20)
#define EFI_ABORTED				EFI_ERROR(21)
#define EFI_ICMP_ERROR				EFI_ERROR(22)
#define EFI_TFTP_ERROR				EFI_ERROR(23)
#define EFI_PROTOCOL_ERROR			EFI_ERROR(24)
#define EFI_INCOMPATIBLE_VERSION		EFI_ERROR(25)
#define EFI_SECURITY_VIOLATION			EFI_ERROR(26)
#define EFI_CRC_ERROR				EFI_ERROR(27)
#define EFI_END_OF_MEDIA			EFI_ERROR(28)
#define EFI_END_OF_FILE				EFI_ERROR(31)
#define EFI_INVALID_LANGUAGE			EFI_ERROR(32)
#define EFI_COMPROMISED_DATA			EFI_ERROR(33)
#define EFI_IP_ADDRESS_CONFLICT			EFI_ERROR(34)

/** EFI warning codes. */
#define EFI_WARN_UNKNOWN_GLYPH			EFI_WARNING(1)
#define EFI_WARN_DELETE_FAILURE			EFI_WARNING(2)
#define EFI_WARN_WRITE_FAILURE			EFI_WARNING(3)
#define EFI_WARN_BUFFER_TOO_SMALL		EFI_WARNING(4)
#define EFI_WARN_STALE_DATA			EFI_WARNING(5)

/**
 * EFI device path protocol definitions.
 */

/** Device path protocol GUID. */
#define EFI_DEVICE_PATH_PROTOCOL_GUID \
	{ 0x09576e91, 0x6d3f, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

/** Device path protocol. */
typedef struct efi_device_path_protocol {
	efi_uint8_t type;
	efi_uint8_t subtype;
	efi_uint16_t length;
} efi_device_path_protocol_t;

/**
 * EFI console I/O protocol definitions.
 */

/** Simple text input protocol GUID. */
#define EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID \
	{ 0x387477c1, 0x69c7, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

/** Input key structure. */
typedef struct efi_input_key {
	efi_uint16_t scan_code;
	efi_char16_t unicode_char;
} efi_input_key_t;

/** Simple text input protocol. */
typedef struct efi_simple_text_input_protocol {
	EFI_STATUS (*reset)(struct efi_simple_text_input_protocol *this,
		efi_boolean_t extended_verification) EFIAPI;
	EFI_STATUS (*read_key_stroke)(
		struct efi_simple_text_input_protocol *this,
		efi_input_key_t *key) EFIAPI;

	EFI_EVENT wait_for_key;
} efi_simple_text_input_protocol_t;

/** Simple text output protocol GUID. */
#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
	{ 0x387477c2, 0x69c7, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

/** Text attribute definitions. */
#define EFI_BLACK				0x00
#define EFI_BLUE				0x01
#define EFI_GREEN				0x02
#define EFI_CYAN				0x03
#define EFI_RED					0x04
#define EFI_MAGENTA				0x05
#define EFI_BROWN				0x06
#define EFI_LIGHTGRAY				0x07
#define EFI_BRIGHT				0x08
#define EFI_DARKGRAY				0x08
#define EFI_LIGHTBLUE				0x09
#define EFI_LIGHTGREEN				0x0a
#define EFI_LIGHTCYAN				0x0b
#define EFI_LIGHTRED				0x0c
#define EFI_LIGHTMAGENTA			0x0d
#define EFI_YELLOW				0x0e
#define EFI_WHITE				0x0f

/** Calculate a text attribute value. */
#define EFI_TEXT_ATTR(fg, bg)			((fg) | ((bg) << 4))

/** Text output mode structure. */
typedef struct efi_simple_text_output_mode {
	efi_int32_t max_mode;

	/** Current settings. */
	efi_int32_t mode;
	efi_int32_t attribute;
	efi_int32_t cursor_column;
	efi_int32_t cursor_row;
	efi_boolean_t cursor_visible;
} efi_simple_text_output_mode_t;

/** Simple text output protocol. */
typedef struct efi_simple_text_output_protocol {
	EFI_STATUS (*reset)(struct efi_simple_text_output_protocol *this,
		efi_boolean_t extended_verification) EFIAPI;
	EFI_STATUS (*output_string)(
		struct efi_simple_text_output_protocol *this,
		const efi_char16_t *string) EFIAPI;
	EFI_STATUS (*test_string)(
		struct efi_simple_text_output_protocol *this,
		const efi_char16_t *string) EFIAPI;
	EFI_STATUS (*query_mode)(struct efi_simple_text_output_protocol *this,
		efi_uintn_t mode_number, efi_uintn_t *columns,
		efi_uintn_t *rows) EFIAPI;
	EFI_STATUS (*set_mode)(struct efi_simple_text_output_protocol *this,
		efi_uintn_t mode_number) EFIAPI;
	EFI_STATUS (*set_attributes)(
		struct efi_simple_text_output_protocol *this,
		efi_uintn_t attribute) EFIAPI;
	EFI_STATUS (*clear_screen)(
		struct efi_simple_text_output_protocol *this) EFIAPI;
	EFI_STATUS (*set_cursor_position)(
		struct efi_simple_text_output_protocol *this,
		efi_uintn_t column, efi_uintn_t row) EFIAPI;
	EFI_STATUS (*enable_cursor)(
		struct efi_simple_text_output_protocol *this,
		efi_boolean_t visible) EFIAPI;

	efi_simple_text_output_mode_t *mode;
} efi_simple_text_output_protocol_t;

/** EFI serial I/O protocol GUID. */
#define EFI_SERIAL_IO_PROTOCOL_GUID \
	{ 0xbb25cf6f, 0xf1d4, 0x11d2, 0x9a, 0x0c, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0xfd }

/** Serial control bits. */
#define EFI_SERIAL_DATA_TERMINAL_READY		0x1
#define EFI_SERIAL_REQUEST_TO_SEND		0x2
#define EFI_SERIAL_CLEAR_TO_SEND		0x10
#define EFI_SERIAL_DATA_SET_READY		0x20
#define EFI_SERIAL_RING_INDICATE		0x40
#define EFI_SERIAL_CARRIER_DETECT		0x80
#define EFI_SERIAL_INPUT_BUFFER_EMPTY		0x100
#define EFI_SERIAL_OUTPUT_BUFFER_EMPTY		0x200
#define EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE	0x1000
#define EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE	0x2000
#define EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE	0x4000

/** Serial parity type. */
typedef enum efi_parity_type {
	EFI_DEFAULT_PARITY,
	EFI_NO_PARITY,
	EFI_EVEN_PARITY,
	EFI_ODD_PARITY,
	EFI_MARK_PARITY,
	EFI_SPACE_PARITY,
} efi_parity_type_t;

/** Serial stop-bits type. */
typedef enum efi_stop_bits_type {
	EFI_DEFAULT_STOP_BITS,
	EFI_ONE_STOP_BIT,
	EFI_ONE_FIVE_STOP_BITS,
	EFI_TWO_STOP_BITS,
} efi_stop_bits_type_t;

/** Serial I/O protocol. */
typedef struct efi_serial_io_protocol {
	uint32_t revision;

	EFI_STATUS (*reset)(struct efi_serial_io_protocol *this) EFIAPI;
	EFI_STATUS (*set_attributes)(struct efi_serial_io_protocol *this,
		efi_uint64_t baud_rate, efi_uint32_t receive_fifo_depth,
		efi_uint32_t timeout, efi_parity_type_t parity,
		efi_uint8_t data_bits, efi_stop_bits_type_t stop_bits) EFIAPI;
	EFI_STATUS (*set_control)(struct efi_serial_io_protocol *this,
		efi_uint32_t control) EFIAPI;
	EFI_STATUS (*get_control)(struct efi_serial_io_protocol *this,
		efi_uint32_t *control) EFIAPI;
	EFI_STATUS (*write)(struct efi_serial_io_protocol *this,
		efi_uintn_t *buffer_size, void *buffer) EFIAPI;
	EFI_STATUS (*read)(struct efi_serial_io_protocol *this,
		efi_uintn_t *buffer_size, void *buffer) EFIAPI;
} efi_serial_io_protocol_t;

/**
 * EFI boot services definitions.
 */

/** Type of allocation to perform. */
typedef enum efi_allocate_type {
	EFI_ALLOCATE_ANY_PAGES,
	EFI_ALLOCATE_MAX_ADDRESS,
	EFI_ALLOCATE_ADDRESS,
	EFI_MAX_ALLOCATE_TYPE,
} efi_allocate_type_t;

/** Memory type definitions. */
typedef enum efi_memory_type {
	EFI_RESERVED_MEMORY_TYPE,
	EFI_LOADER_CODE,
	EFI_LOADER_DATA,
	EFI_BOOT_SERVICES_CODE,
	EFI_BOOT_SERVICES_DATA,
	EFI_RUNTIME_SERVICES_CODE,
	EFI_RUNTIME_SERVICES_DATA,
	EFI_CONVENTIONAL_MEMORY,
	EFI_UNUSABLE_MEMORY,
	EFI_ACPI_RECLAIM_MEMORY,
	EFI_ACPI_MEMORY_NVS,
	EFI_MEMORY_MAPPED_IO,
	EFI_MEMORY_MAPPED_IO_PORT_SPACE,
	EFI_PAL_CODE,
	EFI_MAX_MEMORY_TYPE,

	// Start of OS-defined memory type range
	EFI_OS_MEMORY_TYPE = 0x80000000,
} efi_memory_type_t;

/** Memory range descriptor. */
typedef struct efi_memory_descriptor {
	efi_uint32_t type;
	efi_physical_address_t physical_start;
	efi_virtual_address_t virtual_start;
	efi_uint64_t num_pages;
	efi_uint64_t attribute;
} efi_memory_descriptor_t;

/** Memory attribute definitions. */
#define EFI_MEMORY_UC				0x1ll
#define EFI_MEMORY_WC				0x2ll
#define EFI_MEMORY_WT				0x4ll
#define EFI_MEMORY_WB				0x8ll
#define EFI_MEMORY_UCE				0x10ll
#define EFI_MEMORY_WP				0x1000ll
#define EFI_MEMORY_RP				0x2000ll
#define EFI_MEMORY_XP				0x4000ll
#define EFI_MEMORY_RUNTIME			0x8000000000000000ll

/** Memory descriptor version number. */
#define EFI_MEMORY_DESCRIPTOR_VERSION	1

/** Event notification function. */
typedef void (*efi_event_notify_t)(EFI_EVENT event, void *context) EFIAPI;

/** EFI event types. */
#define EFI_EVT_TIMER				0x80000000
#define EFI_EVT_RUNTIME				0x40000000
#define EFI_EVT_NOTIFY_WAIT			0x00000100
#define EFI_EVT_NOTIFY_SIGNAL			0x00000200
#define EFI_EVT_SIGNAL_EXIT_BOOT_SERVICES	0x00000201
#define EFI_EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE	0x60000202

/** Timer delay type. */
typedef enum efi_timer_delay {
	EFI_TIMER_CANCEL,
	EFI_TIMER_PERIODIC,
	EFI_TIMER_RELATIVE,
} efi_timer_delay_t;

/** Interface types. */
typedef enum efi_interface_type {
	EFI_NATIVE_INTERFACE,
} efi_interface_type_t;

/** Handle search types. */
typedef enum efi_locate_search_type {
	EFI_ALL_HANDLES,
	EFI_BY_REGISTER_NOTIFY,
	EFI_BY_PROTOCOL,
} efi_locate_search_type_t;

/** Open protocol information entry. */
typedef struct efi_open_protocol_information_entry {
	EFI_HANDLE agent_handle;
	EFI_HANDLE controller_handle;
	efi_uint32_t attributes;
	efi_uint32_t open_count;
} efi_open_protocol_information_entry_t;

/** Attributes for OpenProtocol(). */
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL	0x1
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL		0x2
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL		0x4
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER	0x8
#define EFI_OPEN_PROTOCOL_BY_DRIVER		0x10
#define EFI_OPEN_PROTOCOL_EXCLUSIVE		0x20

/**
 * EFI runtime services definitions.
 */

/** Current time information. */
typedef struct efi_time {
	efi_uint16_t year;
	efi_uint8_t month;
	efi_uint8_t day;
	efi_uint8_t hour;
	efi_uint8_t minute;
	efi_uint8_t second;
	efi_uint8_t _pad1;
	efi_uint32_t nanosecond;
	efi_int16_t time_zone;
	efi_uint8_t daylight;
	efi_uint8_t _pad2;
} efi_time_t;

/** Capabilities of the real time clock. */
typedef struct efi_time_capabilities {
	efi_uint32_t resolution;
	efi_uint32_t accuracy;
	efi_boolean_t sets_to_zero;
} efi_time_capabilities_t;

/** Reset type. */
typedef enum efi_reset_type {
	EFI_RESET_COLD,
	EFI_RESET_WARM,
	EFI_RESET_SHUTDOWN,
	EFI_RESET_PLATFORM_SPECIFIC,
} efi_reset_type_t;

/**
 * EFI tables.
 */

/** EFI table header. */
typedef struct efi_table_header {
	efi_uint64_t signature;
	efi_uint32_t revision;
	efi_uint32_t header_size;
	efi_uint32_t crc32;
	efi_uint32_t reserved;
} efi_table_header_t;

/** EFI boot services table. */
typedef struct efi_boot_services {
	efi_table_header_t hdr;

	/** Task priority services. */
	EFI_TPL (*raise_tpl)(EFI_TPL new_tpl) EFIAPI;
	void (*restore_tpl)(EFI_TPL old_tpl) EFIAPI;

	/** Memory services. */
	EFI_STATUS (*allocate_pages)(efi_allocate_type_t type,
		efi_memory_type_t memory_type, efi_uintn_t pages,
		efi_physical_address_t *memory) EFIAPI;
	EFI_STATUS (*free_pages)(efi_physical_address_t memory,
		efi_uintn_t pages) EFIAPI;
	EFI_STATUS (*get_memory_map)(efi_uintn_t *memory_map_size,
		efi_memory_descriptor_t *memory_map, efi_uintn_t *map_key,
		efi_uintn_t *descriptor_size, efi_uint32_t *descriptor_version)
		EFIAPI;
	EFI_STATUS (*allocate_pool)(efi_memory_type_t pool_type,
		efi_uintn_t size, void **buffer) EFIAPI;
	EFI_STATUS (*free_pool)(void *buffer) EFIAPI;

	/** Event and timer services. */
	EFI_STATUS (*create_event)(efi_uint32_t type, EFI_TPL notify_tpl,
		efi_event_notify_t notify_func, void *notify_context,
		EFI_EVENT *event) EFIAPI;
	EFI_STATUS (*set_timer)(EFI_EVENT event, efi_timer_delay_t type,
		efi_uint64_t trigger_time) EFIAPI;
	EFI_STATUS (*wait_for_event)(efi_uintn_t num_events,
		EFI_EVENT *event, efi_uintn_t *index) EFIAPI;
	EFI_STATUS (*signal_event)(EFI_EVENT event) EFIAPI;
	EFI_STATUS (*close_event)(EFI_EVENT event) EFIAPI;
	EFI_STATUS (*check_event)(EFI_EVENT event) EFIAPI;

	/** Protocol handler services. */
	EFI_STATUS (*install_protocol_interface)(EFI_HANDLE *handle,
		efi_guid_t *protocol, efi_interface_type_t interface_type,
		void *interface) EFIAPI;
	EFI_STATUS (*reinstall_protocol_interface)(EFI_HANDLE handle,
		efi_guid_t *protocol, void *old_interface, void *new_interface)
		EFIAPI;
	EFI_STATUS (*uninstall_protocol_interface)(EFI_HANDLE handle,
		efi_guid_t *protocol, void *interface) EFIAPI;
	EFI_STATUS (*handle_protocol)(EFI_HANDLE handle,
		efi_guid_t *protocol, void **interface) EFIAPI;
	void *reserved;
	EFI_STATUS (*register_protocol_notify)(efi_guid_t *protocol,
		EFI_EVENT event, void **registration) EFIAPI;
	EFI_STATUS (*locate_handle)(efi_locate_search_type_t search_type,
		efi_guid_t *protocol, void *search_key,
		efi_uintn_t *buffer_size, EFI_HANDLE *buffer) EFIAPI;
	EFI_STATUS (*locate_device_path)(efi_guid_t *protocol,
		efi_device_path_protocol_t **device_path, EFI_HANDLE *device)
		EFIAPI;
	EFI_STATUS (*install_configuration_table)(efi_guid_t *guid,
		void *table) EFIAPI;

	/** Image services. */
	EFI_STATUS (*load_image)(efi_boolean_t boot_policy,
		EFI_HANDLE parent_image_handle,
		efi_device_path_protocol_t *device_path, void *source_buffer,
		efi_uintn_t source_size, EFI_HANDLE *image_handle) EFIAPI;
	EFI_STATUS (*start_image)(EFI_HANDLE image_handle,
		efi_uintn_t *exit_data_size, efi_char16_t **exit_data)
		EFIAPI;
	EFI_STATUS (*exit)(EFI_HANDLE image_handle,
		EFI_STATUS exit_status, efi_uintn_t exit_data_size,
		efi_char16_t *exit_data) EFIAPI;
	EFI_STATUS (*unload_image)(EFI_HANDLE image_handle) EFIAPI;
	EFI_STATUS (*exit_boot_services)(EFI_HANDLE image_handle,
		efi_uintn_t map_key) EFIAPI;

	/** Miscellaneous services. */
	EFI_STATUS (*get_next_monotonic_count)(efi_uint64_t *count) EFIAPI;
	EFI_STATUS (*stall)(efi_uintn_t microseconds) EFIAPI;
	EFI_STATUS (*set_watchdog_timer)(efi_uintn_t timeout,
		efi_uint64_t watchdog_code, efi_uintn_t data_size,
		efi_char16_t *watchdog_data) EFIAPI;

	/** Driver support services. */
	EFI_STATUS (*connect_controller)(EFI_HANDLE controller_handle,
		EFI_HANDLE *driver_image_handle,
		efi_device_path_protocol_t *remaining_device_path,
		efi_boolean_t recursive) EFIAPI;
	EFI_STATUS (*disconnect_controller)(EFI_HANDLE controller_handle,
		EFI_HANDLE driver_image_handle, EFI_HANDLE child_handle)
		EFIAPI;

	/** Open and close protocol services. */
	EFI_STATUS (*open_protocol)(EFI_HANDLE handle, efi_guid_t *protocol,
		void **interface, EFI_HANDLE agent_handle,
		EFI_HANDLE controller_handle, efi_uint32_t attributes)
		EFIAPI;
	EFI_STATUS (*close_protocol)(EFI_HANDLE handle,
		efi_guid_t *protocol, EFI_HANDLE agent_handle,
		EFI_HANDLE controller_handle) EFIAPI;
	EFI_STATUS (*open_protocol_information)(EFI_HANDLE handle,
		efi_guid_t *protocol,
		efi_open_protocol_information_entry_t **entry_buffer,
		efi_uintn_t *entry_count) EFIAPI;

	/** Library services. */
	EFI_STATUS (*protocols_per_handle)(EFI_HANDLE handle,
		efi_guid_t ***protocol_buffer,
		efi_uintn_t *protocol_buffer_count) EFIAPI;
	EFI_STATUS (*locate_handle_buffer)(
		efi_locate_search_type_t search_type, efi_guid_t *protocol,
		void *search_key, efi_uintn_t *num_handles,
		EFI_HANDLE **buffer) EFIAPI;
	EFI_STATUS (*locate_protocol)(efi_guid_t *protocol,
		void *registration, void **interface) EFIAPI;
	EFI_STATUS (*install_multiple_protocol_interfaces)(
		EFI_HANDLE *handle, ...) EFIAPI;
	EFI_STATUS (*uninstall_multiple_protocol_interfaces)(
		EFI_HANDLE handle, ...) EFIAPI;

	/** 32-bit CRC services. */
	EFI_STATUS (*calculate_crc32)(void *data, efi_uintn_t data_size,
		efi_uint32_t *crc32) EFIAPI;

	/** Miscellaneous services. */
	void (*copy_mem)(void *destination, void *source, efi_uintn_t length)
		EFIAPI;
	void (*set_mem)(void *buffer, efi_uintn_t size, efi_uint8_t value)
		EFIAPI;
	EFI_STATUS (*create_event_ex)(efi_uint32_t type, EFI_TPL notify_tpl,
		efi_event_notify_t notify_func, const void *notify_context,
		const efi_guid_t *event_group, EFI_EVENT *event) EFIAPI;
} efi_boot_services_t;

/** EFI boot services table signature. */
#define EFI_BOOT_SERVICES_SIGNATURE		0x56524553544f4f42ll

/** EFI runtime services table. */
typedef struct efi_runtime_services {
	efi_table_header_t hdr;

	/** Time services. */
	EFI_STATUS (*get_time)(efi_time_t *time,
		efi_time_capabilities_t *capabilities) EFIAPI;
	EFI_STATUS (*set_time)(efi_time_t *time) EFIAPI;
	EFI_STATUS (*get_wakeup_time)(efi_boolean_t *enabled,
		efi_boolean_t *pending, efi_time_t *time) EFIAPI;
	EFI_STATUS (*set_wakeup_time)(efi_boolean_t enabled,
		efi_time_t *time) EFIAPI;

	/** Virtual memory services. */
	EFI_STATUS (*set_virtual_address_map)(efi_uintn_t memory_map_size,
		efi_uintn_t descriptor_size, efi_uint32_t descriptor_version,
		efi_memory_descriptor_t *virtual_map) EFIAPI;
	EFI_STATUS (*convert_pointer)(efi_uintn_t debug_disposition,
		void **address) EFIAPI;

	/** Variable services. */
	EFI_STATUS (*get_variable)(efi_char16_t *variable_name,
		efi_guid_t *vendor_guid, efi_uint32_t *attributes,
		efi_uintn_t *data_size, void *data) EFIAPI;
	EFI_STATUS (*get_next_variable_name)(efi_uintn_t *variable_name_size,
		efi_char16_t *variable_name, efi_guid_t *vendor_guid) EFIAPI;
	EFI_STATUS (*set_variable)(efi_char16_t *variable_name,
		efi_guid_t *vendor_guid, efi_uint32_t attributes,
		efi_uintn_t data_size, void *data) EFIAPI;

	/** Miscellaneous services. */
	EFI_STATUS (*get_next_high_monotonic_count)(efi_uint32_t *high_count)
		EFIAPI;
	void (*reset_system)(efi_reset_type_t reset_type,
		EFI_STATUS reset_status, efi_uintn_t data_size,
		efi_char16_t *reset_data) EFIAPI;
} efi_runtime_services_t;

/** EFI runtime services table signature. */
#define EFI_RUNTIME_SERVICES_SIGNATURE		0x56524553544e5552ll

/** EFI configuration table. */
typedef struct efi_configuration_table {
	efi_guid_t vendor_guid;
	void *vendor_table;
} efi_configuration_table_t;

/** EFI system table. */
typedef struct efi_system_table {
	efi_table_header_t hdr;
	efi_char16_t *firmware_vendor;
	efi_uint32_t firmware_revision;
	EFI_HANDLE con_in_handle;
	efi_simple_text_input_protocol_t *con_in;
	EFI_HANDLE con_out_handle;
	efi_simple_text_output_protocol_t *con_out;
	EFI_HANDLE stderr_handle;
	efi_simple_text_output_protocol_t *stderr;
	efi_runtime_services_t *runtime_services;
	efi_boot_services_t *boot_services;
	efi_uintn_t num_table_entries;
	efi_configuration_table_t *config_table;
} efi_system_table_t;

/** EFI system table signature. */
#define EFI_SYSTEM_TABLE_SIGNATURE		0x5453595320494249ll

#endif /* __EFI_API_H */
