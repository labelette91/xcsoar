/*
 * Copyright (C) 2011 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef XCSOAR_UTIL_TYPE_TRAITS_HPP
#define XCSOAR_UTIL_TYPE_TRAITS_HPP

#include "Compiler.h"

#include <type_traits>

/**
 * Check if the specified type is "trivial", but allow a non-trivial
 * default constructor.
 */
template<typename T>
struct has_trivial_copy_and_destructor
  : public std::integral_constant<bool,
#ifdef LIBCXX
                                  std::is_trivially_copy_constructible<T>::value &&
#else
                                  std::has_trivial_copy_constructor<T>::value &&
#endif
#ifdef LIBCXX
                                  std::is_trivially_copy_assignable<T>::value &&
                                  std::is_trivially_destructible<T>::value>
#else
#if GCC_VERSION >= 40600 || defined(__clang__)
                                  std::has_trivial_copy_assign<T>::value &&
#else
                                  std::has_trivial_assign<T>::value &&
#endif
                                  std::has_trivial_destructor<T>::value>
#endif
{
};

/**
 * Wrapper for std::is_trivial with a fallback for GCC 4.4.
 */
#if GCC_VERSION >= 40500 || defined(__clang__)
template<typename T>
struct is_trivial : public std::is_trivial<T> {};
#else
template<typename T>
struct is_trivial : public has_trivial_copy_and_destructor<T> {};
#endif

/**
 * Check if the specified type is "trivial" in the non-debug build,
 * but allow a non-trivial default constructor in the debug build.
 * This is needed for types that use #DebugFlag.
 */
#ifdef NDEBUG
template<typename T>
struct is_trivial_ndebug
  : public std::integral_constant<bool, is_trivial<T>::value>
{
};
#else
template<typename T>
struct is_trivial_ndebug
  : public std::integral_constant<bool,
                                  has_trivial_copy_and_destructor<T>::value>
{
};
#endif

#endif
