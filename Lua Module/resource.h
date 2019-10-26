//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by Metadata.rc

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        101
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

#ifndef VERSION_H
#define VERSION_H

#ifdef _DEBUG
#	define	LIB_VERSION_MODE "D"
#else
#	define	LIB_VERSION_MODE
#endif

#define LIB_STR(s) #s

#define LIB_VERSION_MAJOR	0
#define LIB_VERSION_MINOR	8
#define LIB_VERSION_BUILD	0
#define LIB_VERSION_PATCH	2

#define LIB_VERSION	\
	LIB_STR(LIB_VERSION_MAJOR) "." \
	LIB_STR(LIB_VERSION_MINOR) "." \
	LIB_STR(LIB_VERSION_BUILD) "." \
	LIB_STR(LIB_VERSION_PATCH) LIB_VERSION_MODE

#endif
