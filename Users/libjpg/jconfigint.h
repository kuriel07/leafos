/* libjpeg-turbo build number */
//#undef BUILD

/* Compiler's inline keyword */
//#undef inline

/* How to obtain function inlining. */
//#undef INLINE

/* Define to the full name of this package. */
//#undef PACKAGE_NAME

/* Version number of package */
//#undef VERSION

/* The size of `size_t', as computed by sizeof. */
//#undef SIZEOF_SIZE_T
#define BUILD				"0x1000"
#ifndef INLINE
#define INLINE				inline
#endif
#define PACKAGE_NAME		"libjpg-turbo"
#define VERSION				"62"
#define SIZEOF_SIZE_T		4

/* a function called through method pointers: */
#ifndef METHODDEF
#define METHODDEF(type)         static type
#endif
/* a function used only in its module: */
#ifndef LOCAL
#define LOCAL(type)             static type
#endif
/* a function referenced thru EXTERNs: */
#ifndef GLOBAL
#define GLOBAL(type)            type
#endif
/* a reference to a GLOBAL function: */
#ifndef EXTERN
#define EXTERN(type)            extern type
#endif
