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
 */

#ifndef __EFI_ARCH_API_H
#define __EFI_ARCH_API_H

/**
 * EFI calling convention attribute.
 *
 * On x86_64, EFI uses the Microsoft calling convention, therefore we must
 * mark all EFI APIs with the ms_abi attribute so that the right calling
 * convention is used.
 */
#define __efiapi       __attribute__((ms_abi))

/**
 * EFI call wrapper.
 *
 * We must wrap EFI calls to restore the firmware's GDT/IDT before calling, and
 * restore ours afterward. This is a slightly nasty hack to call functions via
 * a wrapper (in start.S), that keeps type safety and relies on the compiler to
 * put all arguments in the right place.
 */
#define efi_call(func, args...) \
       __extension__ \
       ({ \
               typeof(func) __wrapper = (typeof(func))__efi_call; \
               __efi_call_func = (void *)func; \
               __wrapper(args); \
       })

extern void *__efi_call_func;
extern unsigned long __efi_call(void) __efiapi;

#endif /* __EFI_ARCH_API_H */
