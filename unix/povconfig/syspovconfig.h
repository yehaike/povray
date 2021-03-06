//******************************************************************************
///
/// @file unix/povconfig/syspovconfig.h
///
/// Unix-specific general POV-Ray compile-time configuration.
///
/// This header file configures module-independent aspects of POV-Ray for
/// running properly on a Unix platform.
///
/// @copyright
/// @parblock
///
/// Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
/// Copyright 1991-2016 Persistence of Vision Raytracer Pty. Ltd.
///
/// POV-Ray is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Affero General Public License as
/// published by the Free Software Foundation, either version 3 of the
/// License, or (at your option) any later version.
///
/// POV-Ray is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Affero General Public License for more details.
///
/// You should have received a copy of the GNU Affero General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
///
/// ----------------------------------------------------------------------------
///
/// POV-Ray is based on the popular DKB raytracer version 2.12.
/// DKBTrace was originally written by David K. Buck.
/// DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
///
/// @endparblock
///
//******************************************************************************

#ifndef POVRAY_UNIX_SYSPOVCONFIG_H
#define POVRAY_UNIX_SYSPOVCONFIG_H

#define _FILE_OFFSET_BITS 64 // Required for some Unix flavors to get a 64-bit off_t type on 32-bit systems.

// C++ variants of C standard headers
#include <cmath>
#include <cstdarg>

// C++ standard headers
#include <algorithm>
#include <exception>
#include <limits>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

// boost headers
#include <boost/tr1/memory.hpp>

#ifdef HAVE_CONFIG_H
// from directory "." (generated by ./configure)
# include "config.h"
#else
# error "!!!!! config.h is required !!!!!"
#endif

#ifdef HAVE_STDINT_H
#include "stdint.h"
#endif

using std::max;
using std::min;

#ifndef STD_TYPES_DECLARED
#define STD_TYPES_DECLARED

// the following types are used extensively throughout the POV source and hence are
// included and named here for reasons of clarity and convenience.

// when we say 'string' we mean std::string
using std::string;

// and vector is a std::vector
using std::vector;

// yup, list too
using std::list;

// runtime_error is the base of our Exception class, plus is referred
// to in a few other places.
using std::runtime_error;

// these may actually be the boost implementations, depending on what boost/tr1/memory.hpp has pulled in
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;
using std::tr1::dynamic_pointer_cast;
using std::tr1::static_pointer_cast;
using std::tr1::const_pointer_cast;

#endif // STD_POV_TYPES_DECLARED

// After Stroustrop in _The C++ Programming Language, 3rd Ed_ p. 88
#ifndef NULL
const int NULL=0;
#endif

#define DELETE_FILE(name)  unlink(name)

#if defined (PATH_MAX)
# define FILE_NAME_LENGTH   PATH_MAX
#elif defined (_POSIX_PATH_MAX)
# define FILE_NAME_LENGTH   _POSIX_PATH_MAX
#else
# define FILE_NAME_LENGTH   200
#endif
#define MAX_PATH FILE_NAME_LENGTH  // FIXME: remove later

#define DEFAULT_OUTPUT_FORMAT       kPOVList_FileType_PNG
#define DEFAULT_DISPLAY_GAMMA_TYPE  kPOVList_GammaType_SRGB
#define DEFAULT_DISPLAY_GAMMA       2.2

#define METADATA_PLATFORM_STRING BUILD_ARCH
#define METADATA_COMPILER_STRING COMPILER_VERSION

#ifdef HAVE_NAN
    #if defined(HAVE_STD_ISNAN)
        #define POV_ISNAN(x) std::isnan(x)
    #elif defined(HAVE_ISNAN)
        #define POV_ISNAN(x) isnan(x)
    #elif defined(HAVE_EMULATED_ISNAN)
        template<typename T>
        inline bool pov_isnan(T x) { volatile T v = x; return (v != x); }
        #define POV_ISNAN(x) pov_isnan(x)
    #else
        #error Someone must have found an alternative way of identifying NaNs but failed to implement it here.
    #endif
#endif

#ifdef HAVE_INF
    #if defined(HAVE_STD_ISINF)
        #define POV_ISINF(x) std::isinf(x)
    #elif defined(HAVE_ISINF)
        #define POV_ISINF(x) isinf(x)
    #elif defined(HAVE_EMULATED_ISINF)
        template<typename T>
        inline bool pov_isinf(T x) { volatile T v = std::numeric_limits<T>::max(); return std::fabs(x) > v; }
        #define POV_ISINF(x) pov_isinf(x)
    #else
        #error Someone must have found an alternative way of identifying infinities but failed to implement it here.
    #endif
#endif

#if defined INT8_MAX || defined int8_t
    #define POV_INT8 int8_t
#else
    // Autoconf is convinced that there's no signed integer type exactly 8 bits wide. We won't pick a type here.
#endif

#if defined UINT8_MAX || defined uint8_t
    #define POV_UINT8 uint8_t
#else
    // Autoconf is convinced that there's no unsigned integer type exactly 8 bits wide. We won't pick a type here.
#endif

#if defined INT16_MAX || defined int16_t
    #define POV_INT16 int16_t
#else
    // Autoconf is convinced that there's no signed integer type exactly 16 bits wide. We won't pick a type here.
#endif

#if defined UINT16_MAX || defined uint16_t
    #define POV_UINT16 uint16_t
#else
    // Autoconf is convinced that there's no unsigned integer type exactly 16 bits wide. We won't pick a type here.
#endif

#if defined INT32_MAX || defined int32_t
    #define POV_INT32 int32_t
#else
    // Autoconf is convinced that there's no signed integer type exactly 32 bits wide. We won't pick a type here.
#endif

#if defined UINT32_MAX || defined uint32_t
    #define POV_UINT32 uint32_t
#else
    // Autoconf is convinced that there's no unsigned integer type exactly 32 bits wide. We won't pick a type here.
#endif

#if defined INT64_MAX || defined int64_t
    #define POV_INT64 int64_t
#else
    // Autoconf is convinced that there's no signed integer type exactly 64 bits wide. We won't pick a type here.
#endif

#if defined UINT64_MAX || defined uint64_t
    #define POV_UINT64 uint64_t
#else
    // Autoconf is convinced that there's no unsigned integer type exactly 64 bits wide. We won't pick a type here.
#endif

// Pull in additional settings depending on Unix flavor

#if defined(_AIX)
    // IBM AIX detected.
    // Not officially supported yet; comment-out the following line to try with default POSIX settings.
    #error IBM AIX detected, but not explicitly supported yet; proceed at your own risk.
    #include "syspovconfig_posix.h"
#elif defined(__hpux)
    // Hewlett-Packard HP-UX detected.
    // Not officially supported yet; comment-out the following line to try with default POSIX settings.
    #error Hewlett-Packard HP-UX detected, but not explicitly supported yet; proceed at your own risk.
    #include "syspovconfig_posix.h"
#elif defined(__linux__)
    // GNU/Linux detected.
    #include "syspovconfig_gnu.h"
#elif defined(__APPLE__) && defined(__MACH__)
    // Apple Mac OS X detected.
    #include "syspovconfig_osx.h"
#elif defined(__sun) && defined(__SVR4)
    // Sun/Oracle Solaris detected.
    // Not officially supported yet; comment-out the following line to try with default POSIX settings.
    #error Sun/Oracle Solaris detected, but not explicitly supported yet; proceed at your own risk.
    #include "syspovconfig_posix.h"
#elif defined(__CYGWIN__)
    // Cygwin detected.
    // Not officially supported yet; comment-out the following line to try with default POSIX settings.
    #error Cygwin detected, but not explicitly supported yet; proceed at your own risk.
    #include "syspovconfig_posix.h"
#elif defined(__unix__)
    // Some Unix other than the above detected.
    #include <sys/param.h>
    #if defined(BSD)
        // BSD-style Unix detected.
        #include "syspovconfig_bsd.h"
    #else
        // Not officially supported yet; comment-out the following line to try with default POSIX settings.
        #error Unix detected, but flavor not identified; proceed at your own risk.
        #include "syspovconfig_posix.h"
    #endif
#else
    // Doesn't look like a Unix at all.
    // Comment-out the following line to try with default POSIX settings.
    #error No Unix detected; proceed at your own risk.
    #include "syspovconfig_posix.h"
#endif

#endif // POVRAY_UNIX_SYSPOVCONFIG_H
