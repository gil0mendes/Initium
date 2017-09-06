ORG 0x7C00
SECTION .text
USE16

boot:
    hlt

; Pad the file to 2KB
times 510-($-$$) db 0
db 0x55
db 0xaa