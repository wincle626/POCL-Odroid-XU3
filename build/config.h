/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* The normal alignment of `double16', in bytes. */
#define ALIGNOF_DOUBLE16 128

/* The normal alignment of `float16', in bytes. */
#define ALIGNOF_FLOAT16 64

/* Absolute path of build directory. */
#define BUILDDIR "/root/enpower/pocl/pocl-git-icd/build"

/* Defined to 1 in case the HSA driver should be built. */
/* #undef BUILD_HSA */

/* "Build with ICD" */
#define BUILD_ICD /**/

/* clang executable. */
#define CLANG "/root/enpower/llvm/llvm_37/install/bin/clang"

/* clang++ executable */
#define CLANGXX "/root/enpower/llvm/llvm_37/install/bin/clang++"

/* clang++ compiler flags */
#define CLANGXX_FLAGS "--target=armv7l-unknown-linux-gnueabihf "

/* "Using a SPIR generator Clang from Khronos." */
/* #undef CLANG_SPIR */

/* Additional CL compiler flags. */
#define CLFLAGS " -Xclang -cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"

/* "Export OpenCL symbols" */
/* #undef DIRECT_LINKAGE */

/* Always used flags for Clang when compiling OpenCL C code. */
#define FORCED_CLFLAGS "-Xclang -ffake-address-space-map -fno-math-errno -fblocks -fno-builtin -fasm -Wno-format"

/* define if the Boost library is available */
/* #undef HAVE_BOOST */

/* Have clock_gettime */
#define HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Defined if The OpenGL Extension Wrangler library is found */
/* #undef HAVE_GLEW */

/* Have AMD-specific HSA headers */
/* #undef HAVE_HSA_EXT_AMD_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Defined to 1 if ocl-icd is found on system */
#define HAVE_OCL_ICD 1

/* Defined if system cl.hpp found to be working */
#define HAVE_OPENCL_HPP 

/* Defined if posix_memalign is available. */
#define HAVE_POSIX_MEMALIGN /**/

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `vfork' function. */
#define HAVE_VFORK 1

/* Define to 1 if __fp16 supports arithmetic operations float. */
#define HAVE_WORKING_HALF 0

/* The host triple. */
#define HOST "armv7l-unknown-linux-gnueabihf"

/* Parameters to as for code generation in the host. */
#define HOST_AS_FLAGS " -mfloat-abi=hard"

/* Parameters to for host compilation. */
#define HOST_CLANG_FLAGS " -mfloat-abi=hard --target=armv7-unknown-linux-gnueabihf"

/* The host CPU type. */
#define HOST_CPU "armv7l"

/* OpenCL major version supported by host device */
#define HOST_DEVICE_CL_VERSION_MAJOR 2

/* OpenCL minor version supported by host device */
#define HOST_DEVICE_CL_VERSION_MINOR 0

/* OpenCL device extensions implemented by the host devices basic and pthreads
   */
#define HOST_DEVICE_EXTENSIONS "cl_khr_byte_addressable_store cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_spir cl_khr_int64 cl_khr_fp64 cl_khr_int64_base_atomics cl_khr_int64_extended_atomics"

/* "basic and pthreads devices use soft-float ABI" */
/* #undef HOST_FLOAT_SOFT_ABI */

/* Parameter to compiler to generate loadable module. */
#define HOST_LD_FLAGS "-shared"

/* Parameters to llc for code generation in the host. */
#define HOST_LLC_FLAGS "-relocation-model=pic -float-abi=hard -mtriple=armv7-unknown-linux-gnueabihf"

/* Path to HSAILasm executable */
/* #undef HSAIL_ASM */

/* OpenCL major version supported by HSA device */
/* #undef HSA_DEVICE_CL_VERSION_MAJOR */

/* OpenCL major version supported by HSA device */
/* #undef HSA_DEVICE_CL_VERSION_MINOR */

/* aoeuaoe */
#define HSA_DEVICE_EXTENSIONS ""

/* Use the libkernel from lib/kernel/$KERNEL_DIR/ */
#define KERNEL_DIR "host"

/* "Using LLVM 3.7" */
#define LLVM_3_7 /**/

/* "Using LLVM 3.8" */
/* #undef LLVM_3_8 */

/* "LLVM was built with Assertions on." */
#define LLVM_BUILT_WITH_ASSERTS /**/

/* LLVM compiler executable. */
#define LLVM_LLC "/root/enpower/llvm/llvm_37/install/bin/llc"

/* "LLVM version as a string." */
#define LLVM_VERSION "3.7.1"

/* Defined to 1 if lttng-ust is found on system */
/* #undef LTTNG_UST_AVAILABLE */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Defined to greatest expected alignment for extended types, in bytes. */
#define MAX_EXTENDED_ALIGNMENT ALIGNOF_DOUBLE16

/* The kernel architecture. */
#define OCL_KERNEL_ARCH ""

/* The kernel target triplet. */
#define OCL_KERNEL_TARGET "armv7-unknown-linux-gnueabihf"

/* The kernel target CPU variant. */
#define OCL_KERNEL_TARGET_CPU "(unknown)"

/* Name of package */
#define PACKAGE "pocl"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "pocl-devel@lists.sourceforge.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "pocl"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "pocl 0.14-pre"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "pocl"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.14-pre"

/* Defined to 1 if compiling for Android */
/* #undef POCL_ANDROID */

/* Directory where pocl files are installed in android */
/* #undef POCL_ANDROID_PREFIX */

/* Timestamp of build */
#define POCL_BUILD_TIMESTAMP "201603301351816201346"

/* OpenCL version string as reported by clGetPlatformInfo */
#define POCL_CL_VERSION "2.0"

/* "Build pocl in debug mode" */
/* #undef POCL_DEBUG_BUILD */

/* Printout debug messages in case POCL_DEBUG env is set */
#define POCL_DEBUG_MESSAGES 1

/* Value based on host processor, for basic and pthreads devices */
#define POCL_DEVICE_ADDRESS_BITS 32

/* "Disabled kernel cache feature" */
#define POCL_KERNEL_CACHE_DEFAULT 1

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 4

/* The size of `__fp16', as computed by sizeof. */
#define SIZEOF___FP16 2

/* Root for the project sources. */
#define SRCDIR "/root/enpower/pocl/pocl-git-icd"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* The target triple. */
#define TARGET "armv7l-unknown-linux-gnueabihf"

/* Parameters to for target compilation. */
#define TARGET_CLANG_FLAGS ""

/* Parameters to llc for code generation in the target. */
#define TARGET_LLC_FLAGS ""

/* Defined to 1 if TCEMC libraries and tools are available */
/* #undef TCEMC_AVAILABLE */

/* Defined to 1 if TCE libraries and tools are available */
/* #undef TCE_AVAILABLE */

/* OpenCL major version supported by TCE device */
/* #undef TCE_DEVICE_CL_VERSION_MAJOR */

/* OpenCL minor version supported by TCE device */
/* #undef TCE_DEVICE_CL_VERSION_MINOR */

/* TCE device supported extension list */
/* #undef TCE_DEVICE_EXTENSIONS */

/* TCE device supported extension list */
/* #undef TCE_DEVICE_EXTENSION_DEFINES */

/* "Use vecmathlib if available for the target." */
#define USE_VECMATHLIB /**/

/* Version number of package */
#define VERSION "0.14-pre"

/* Endianness */
#define WORDS_BIGENDIAN 0

/* Disable cl_khr_fp16 on host based devices. */
#define _CL_DISABLE_HALF /**/
