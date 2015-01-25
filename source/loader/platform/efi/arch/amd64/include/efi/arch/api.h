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
 * @brief		AMD64 EFI API definitions.
 *
 * The implementation is based on https://github.com/vathpela/gnu-efi
 */

#ifndef __EFI_ARCH_API_H
#define __EFI_ARCH_API_H

#ifndef __GNUC__
#pragma pack()
#endif

//
// __efiapi           - prototype calling convention for EFI function pointers
// BOOTSERVICE      - prototype for implementation of a boot service interface
// RUNTIMESERVICE   - prototype for implementation of a runtime service interface
// RUNTIMEFUNCTION  - prototype for implementation of a runtime function that is not a service
// RUNTIME_CODE     - pragma macro for declaring runtime code
//

/**
 * EFI calling convention attribute.
 *
 * On x86_64, EFI uses the Microsoft calling convention. We could use the ms_abi
 * attribute, however support for it in clang does not work right, so for now
 * we use the normal calling convention and convert in a wrapper function.
 */
#define __efiapi

#define BOOTSERVICE
#define RUNTIMESERVICE
#define RUNTIMEFUNCTION

#define RUNTIME_CODE(a)         alloc_text("rtcode", a)
#define BEGIN_RUNTIME_DATA()    data_seg("rtdata")
#define END_RUNTIME_DATA()      data_seg("")

#define VOLATILE    volatile

#define MEMORY_FENCE()

/* Mostly borrowed from here:
 *
 * http://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
 */
#define __VA_NARG(...) __VA_NARG_I(_0, ## __VA_ARGS__, __RSEQ_N())
#define __VA_NARG_I(...) __VA_NARG_N(__VA_ARGS__)
#define __VA_NARG_N(_0, _1, _2, _3, _4, _5, _6, _7, N, ...) N
#define __RSEQ_N() 7, 6, 5, 4, 3, 2, 1, 0

#define __efi_vcall_(n) __efi_call##n
#define __efi_vcall(n) __efi_vcall_(n)

//
// Some compilers don't support the forward reference construct:
// typedef struct XXXXXX
//
// The folliwing macro provide a workaround for such cases
//
#ifdef NO_INTERFACE_DECL
#define INTERFACE_DECL(x)
#else
#ifdef __GNUC__
#define INTERFACE_DECL(x) struct x
#else
#define INTERFACE_DECL(x) typedef struct x
#endif
#endif

/**
 * EFI call wrapper.
 *
 * We wrap EFI calls both to convert between calling conventions and to restore
 * the firmware's GDT/IDT before calling, and restore ours afterward. This is a
 * horrible bundle of preprocessor abuse that forwards EFI calls to the right
 * wrapper for the number of arguments, while keeping type safety.
 */
#define efi_call(func, ...) \
    __extension__ \
    ({ \
        typeof(func) __wrapper = (typeof(func)) __efi_vcall(__VA_NARG(__VA_ARGS__)); \
        __efi_call_func = (void *)func; \
        __wrapper(__VA_ARGS__); \
    })

extern void *__efi_call_func;

extern uint64_t __efi_call0(void);
extern uint64_t __efi_call1(uint64_t);
extern uint64_t __efi_call2(uint64_t, uint64_t);
extern uint64_t __efi_call3(uint64_t, uint64_t, uint64_t);
extern uint64_t __efi_call4(uint64_t, uint64_t, uint64_t, uint64_t);
extern uint64_t __efi_call5(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
extern uint64_t __efi_call6(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
extern uint64_t __efi_call7(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

#endif /* __EFI_ARCH_API_H */
