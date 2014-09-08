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
// To export & import functions in the EFI emulator environment
//

#ifdef EFI_NT_EMULATOR
    #define EXPORTAPI           __declspec( dllexport )
#else
    #define EXPORTAPI
#endif

#if defined(GNU_EFI_USE_MS_ABI)
    #if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
        #define HAVE_USE_MS_ABI 1
    #else
        #error Compiler is too old for GNU_EFI_USE_MS_ABI
    #endif
#else
  #define HAVE_USE_MS_ABI 1
#endif

//
// EFIAPI           - prototype calling convention for EFI function pointers
// BOOTSERVICE      - prototype for implementation of a boot service interface
// RUNTIMESERVICE   - prototype for implementation of a runtime service interface
// RUNTIMEFUNCTION  - prototype for implementation of a runtime function that is not a service
// RUNTIME_CODE     - pragma macro for declaring runtime code
//

#ifndef EFIAPI                  // Forces EFI calling conventions reguardless of compiler options
    #ifdef _MSC_EXTENSIONS
        #define EFIAPI __cdecl  // Force C calling convention for Microsoft C compiler
    #elif defined(HAVE_USE_MS_ABI)
        // Force amd64/ms calling conventions.
        #define EFIAPI __attribute__((ms_abi))
    #else
        #define EFIAPI          // Substitute expresion to force C calling convention
    #endif
#endif

#define BOOTSERVICE
#define RUNTIMESERVICE
#define RUNTIMEFUNCTION


#define RUNTIME_CODE(a)         alloc_text("rtcode", a)
#define BEGIN_RUNTIME_DATA()    data_seg("rtdata")
#define END_RUNTIME_DATA()      data_seg("")

#define VOLATILE    volatile

#define MEMORY_FENCE()

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

//
// EFI call wrapper.
//
// We must wrap EFI calls to restore the firmware's GDT/IDT before calling, and
// restore ours afterward. This is a slightly nasty hack to call functions via
// a wrapper (in start.S), that keeps type safety and relies on the compiler to
// put all arguments in the right place.
//
#define efi_call(func, args...) \
       __extension__ \
       ({ \
               typeof(func) __wrapper = (typeof(func))__efi_call; \
               __efi_call_func = (void *)func; \
               __wrapper(args); \
       })

extern void *__efi_call_func;
extern unsigned long __efi_call(void) EFIAPI;

#endif /* __EFI_ARCH_API_H */
