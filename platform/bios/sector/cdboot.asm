; BIOS CS boot sector

; ==================================================
; Useful macros for debugging
; ==================================================

DEBUG	EQU 1
	
%macro DebugCharMacro 1
    push  ax
    mov   al, %1
    call  putc
    pop ax
%endmacro
%macro DebugPauseMacro 0
    push ax
    call getc
    pop ax
%endmacro
	
%if DEBUG
%define DebugChar(x)  DebugCharMacro x
%define DebugPause(x)  DebugPauseMacro
%else
%define DebugChar(x)
%define DebugPause(x)
%endif

; Definitions
kBoot2Address           equ 0x0200     ; boot2 load address
kBoot2Segment           equ 0x2000      ; boot2 load segment

kBoot0Stack             equ 0xFFF0      ; boot0 stack pointer

kReadBuffer             equ 0x1000      ; disk data buffer address

kVolSectorOffset        equ  0x47       ; offset in buffer of sector number
                                        ; in volume descriptor
kBootSectorOffset       equ  0x28       ; offset in boot catalog
                                        ; of sector number to load boot file
kBootCountOffset        equ  0x26       ; offset in boot catalog 
                                        ; of number of sectors to read

kMaxSectorLoadCount     equ  32		    ; Load 32 sectors at a time
kMaxSectorCount         equ  256		; 512k should be enough for anyone

ORG 0x7C00
SECTION .text
USE16

;---------------------------------------------------
; Entry point
; 
; Arguments:
;   DL = BIOS boot drive
start:
    cli
    jmp 0:start1            ; far jump makes sure we canonicalize the address
    times 8-($-$$) nop      ; pad to file offset 8

; El Torito boot information table, filled in by the 
; mkisofts -boot-info-table, if used
bi_pvd:         dd 0			; LBA of primary volume descriptor
bi_file:        dd 0			; LBA of boot file
bi_length:	    dd 0			; Length of boot file
bi_csum:	    dd 0			; Checksum of boot file
bi_reserved:	times 10 dd 0	; Reserved

start1:
    xor ax, ax                  ; zero ax
    mov ss, ax                  ; setup stack segment
    mov sp, kBoot0Stack + 4     ; setup stack pointer
    sti
    cld                         ; increment SI after each lodsb call

    mov ds, ax                  ; setup the data segments
    mov es, ax                  ;

    ; Set up a "far return" to INT 18
    mov ax, [0x18 * 4 + 2]
    push ax
    mov ax, [0x18 * 4]
    push ax

    DebugChar('!')
    DebugPause()

    ; When we are in debug mode, print the second stage
    ; loader address
    %if DEBUG
    mov eax, kBoot2LoadAddr
    call print_hex
    DebugPause()
    %endif

    ;;
    ;; The BIOS likely didn't load the rest of the bootloader,
    ;; so we have to fetch it ourselves.
    ;;

    ; Load El Torito "Boot Record Volume Descriptor" from CD Sector 17
    xor ax, ax
    mov es, ax
    mov bx, kReadBuffer
    mov al, 1
    mov ecx, 17
    call read_lba
    jc near error

    DebugChar('A')

    ; Load the first sector of the "Boot Catalog"
    mov ecx, [kReadBuffer + kVolSectorOffset]

    %if DEBUG
    mov eax, ecx
    call print_hex
    DebugPause()
    %endif
    mov al, 1
    call read_lba
    jc near error

    ;; New we have the boot catalog in the buffer.
    ;; Really we should look at the validation entry, but oh well.

    DebugChar('B')

    ; Start ES at 2020: which is equivalent to 2000:0200h
    mov ax, kBoot2Segment + (kBoot2Address >> 4)
    mov es, ax

    ; CLear interrupts while we cheat and work with 32-bit registers
    cli

    mov DWORD eax, [bi_length]

    ; Length cannot be zero
    test DWORD eax, eax
    jnz have_nonzero_length

    hlt
    jmp $

have_nonzero_length:
    ; bi_length = (bi_length + 2047) >> 11
	add	dword eax, 2048-1
	shr	dword eax, 11
	; eax now contains the total number of sectors to read

	; Check that the number of sectors is sane (not too high)
	cmp	dword eax, kMaxSectorCount
	jbe	have_sane_sector_count

	; Clear EAX, reenable interrupts, and fail
	xor	dword eax, eax
	sti
	jmp	error_insane_sector_count

have_sane_sector_count:
	; high bits of eax are guaranteed 0 because it's < kMaxSectorCount.
	sti
	dec	eax		; skip the first sector which is what we are in

error_insane_sector_count:
	mov	si, msg_insane_sector_count
	call	puts
	jmp error

; TODO: convert to a string
error:
    mov al, 'E'
    call putc

;-----------------------------------------------------------------
; read_lba - Read sectors from CD using LBA addressing.
;
; Arguments:
;   AL  = number of 2048-byte sectors to read (valid from 1-127).
;   ES:BX = pointer to where the sectors should be stored.
;   ECX = sector offset in partition 
;   DL = drive number (0x80 + unit number)
;
; Returns:
;   CF = 0  success
;        1 error
;
read_lba:
    pushad                          ; save all registers
    mov     bp, sp                  ; save current SP

    ;
    ; Create the Disk Address Packet structure for the
    ; INT13/F42 (Extended Read Sectors) on the stack.
    ;

    ; push    DWORD 0               ; offset 12, upper 32-bit LBA
    push    ds                      ; For sake of saving memory,
    push    ds                      ; push DS register, which is 0.

    push    ecx

    push    es                      ; offset 6, memory segment

    push    bx                      ; offset 4, memory offset

    xor     ah, ah                  ; offset 3, must be 0
    push    ax                      ; offset 2, number of sectors

    push    WORD 16                 ; offset 0-1, packet size

    ;
    ; INT13 Func 42 - Extended Read Sectors
    ;
    ; Arguments:
    ;   AH    = 0x42
    ;   DL    = drive number (80h + drive unit)
    ;   DS:SI = pointer to Disk Address Packet
    ;
    ; Returns:
    ;   AH    = return status (sucess is 0)
    ;   carry = 0 success
    ;           1 error
    ;
    ; Packet offset 2 indicates the number of sectors read
    ; successfully.
    ;
    mov     si, sp
    mov     ah, 0x42
    int     0x13

    jnc     .exit

    DebugChar('R')                  ; indicate INT13/F42 error

    ;
    ; Issue a disk reset on error.
    ; Should this be changed to Func 0xD to skip the diskette controller
    ; reset?
    ;
    xor     ax, ax                  ; Func 0
    int     0x13                    ; INT 13
    stc                             ; set carry to indicate error

.exit:
    mov     sp, bp                  ; restore SP
    popad
    ret

; ================================================================
; Input/Output functions
; ================================================================

;-----------------------------------------------------------------
; Write an asciiz string from DS:SI
;
puts:
	pushf
	push	si
puts_next_char:
	lodsb			; AL <- DS[SI]; INC SI
	test	al,al
	jz	puts_done
	call	putc
	jmp	puts_next_char
puts_done:
	pop	si
	popf
	ret

;-----------------------------------------------------------------
; Display a single character from AL
;
; Arguments
;   AL = Character to print
;
putc:
    pushad
    mov bx, 0x1         ; attribute for output
    mov ah, 0xe         ; BIOS: put_char
    int 0x10            ; call BIOS, print char in %al
    popad
    ret

;-----------------------------------------------------------------
; Get a acharacter from the keyboard and return it 
; in AL
;
getc:
    int 0x16
    ret

%if DEBUG
;-----------------------------------------------------------------
; Write the 4-byte value to the console in hex.
;
; Arguments:
;   EAX = Value to be displayed in hex.
;
print_hex:
    pushad
    mov     cx, WORD 4
    bswap   eax                 ; swap the bytes
.loop
    push    ax
    ror     al, 4
    call    print_nibble        ; display upper nibble
    pop     ax
    call    print_nibble        ; display lower nibble
    ror     eax, 8
    loop    .loop

    mov     al, 10              ; carriege return
    call    putc                ;
    mov     al, 13              ;
    call    putc                ;
    popad
    ret

print_nibble:
    and     al, 0x0f
    add     al, '0'
    cmp     al, '9'
    jna     .print_ascii
    add     al, 'A' - '9' - 1

.print_ascii:
    call    putc
    ret
%endif  ; DEBUG

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; String table
align	16
msg_insane_sector_count:
        db      "cdboot: cannot load more than 256 sectors"
        db      13, 10, 0

; Pad this file to a size of 2048 bytes (one CD sector).
pad:
    times 510-($-$$) db 0
    db 0x55
    db 0xaa

kBoot2LoadAddr  equ $

END