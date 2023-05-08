/**
 *
 * @file common.h
 * @brief Common utility routines and definitions
 *
 */
#ifndef _CDEBCONF_COMMON_H_
#define _CDEBCONF_COMMON_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "debug.h"
#include "constants.h"



#define DIE(fmt, args...) 					\
 	do {							\
		fprintf(stderr, "%s:%d (%s): ", __FILE__, __LINE__, __FUNCTION__); \
		fprintf(stderr, fmt, ##args);			\
		fprintf(stderr, "\n");				\
		exit(EXIT_FAILURE);					\
	} while(0)

#ifndef NODEBUG
#define FLINFO(fmt,args...)                         \
	do{debug_printf(INFO_VERBOSE,"[%s:%d]",__FILE__,__LINE__);debug_printf(INFO_VERBOSE,fmt,##args);} while(0)
#define INFO(level, fmt, args...)					\
	debug_printf(level, fmt, ##args)
#define ASSERT(cond) do { if (!(cond)) DIE("%s:%d (%s): Assertion failed: %s", __FILE__, __LINE__, __FUNCTION__, #cond); } while (0)
#else
#define INFO(level, fmt, args...)	/* nothing */
#define ASSERT(cond)
#endif


#ifndef NODEBUG
#include <execinfo.h>
#include <string.h>

#define APPEND_STR(_ret,_totalerr,_strv,_strsize,...)                                             \
do{                                                                                               \
	if (_totalerr != 0) {                                                                         \
		char* __rtmp = NULL;                                                                      \
		char* __htmp = NULL;                                                                      \
		_ret = asprintf(&__rtmp,__VA_ARGS__);                                                     \
		if (_ret < 0) {                                                                           \
			_totalerr = 1;                                                                        \
		} else {                                                                                  \
			int _slen = 0;                                                                        \
			int _tlen = 0;                                                                        \
			int _clen = strlen(__rtmp);                                                           \
			if (_strsize == 0) {                                                                  \
				_tlen = _clen + 1;                                                                \
			} else {                                                                              \
				_slen = strlen(_strv);                                                            \
				_tlen = _slen + _clen + 1;                                                        \
			}                                                                                     \
			if (_tlen > _strsize) {                                                               \
				_strsize = _tlen;                                                                 \
				__htmp = malloc(_strsize);                                                        \
				if (__htmp != NULL) {                                                             \
					memset(__htmp,0,_strsize);                                                    \
					if (_slen > 0) {                                                              \
						memcpy(__htmp,_strv,_slen);                                               \
					}                                                                             \
					if (_clen > 0) {                                                              \
						memcpy(&(__htmp[_slen]), __rtmp,_clen);                                   \
					}                                                                             \
					if (_strv) {                                                                  \
						free(_strv);                                                              \
						_strv = NULL;                                                             \
					}                                                                             \
					_strv = __htmp;                                                               \
					__htmp = NULL;				                                                  \
				} else {                                                                          \
					_totalerr = 1;                                                                \
				}                                                                                 \
			} else {                                                                              \
				if (_clen > 0) {                                                                  \
					memcpy(&(_strv[_slen]), __rtmp,_clen);                                        \
				}                                                                                 \
			}                                                                                     \
		}                                                                                         \
		if (__htmp) {                                                                             \
			free(__htmp);                                                                         \
			__htmp = NULL;                                                                        \
		}                                                                                         \
		if (__rtmp) {                                                                             \
			free(__rtmp);                                                                         \
			__rtmp = NULL;                                                                        \
		}                                                                                         \
	}                                                                                             \
} while(0)

#define  BACKTRACE_LOG(...)                                                                       \
do{                                                                                               \
	void** __pbuf=NULL;                                                                           \
	char** __sym=NULL;                                                                            \
	char*  __str=NULL;                                                                            \
	int __strsize = 0;                                                                            \
	int __size=4;                                                                                 \
	int __len=0;                                                                                  \
	int __ret;                                                                                    \
	int __i;                                                                                      \
	int __output = 0;                                                                             \
	int __totalerr = 0;                                                                           \
                                                                                                  \
	__pbuf = (void**)malloc(sizeof(*__pbuf) * __size);                                            \
	if (__pbuf != NULL ) {                                                                        \
		while(1) {                                                                                \
			__ret = backtrace(__pbuf, __size);                                                    \
			if (__ret < __size) {                                                                 \
				__len = __ret;                                                                    \
				break;                                                                            \
			}                                                                                     \
			__size <<= 1;                                                                         \
			if (__pbuf) {                                                                         \
				free(__pbuf);                                                                     \
				__pbuf = NULL;                                                                    \
			}                                                                                     \
			__pbuf = (void**) malloc(sizeof(*__pbuf) * __size);                                   \
			if (__pbuf == NULL) {                                                                 \
				break;                                                                            \
			}                                                                                     \
		}                                                                                         \
                                                                                                  \
		if (__pbuf != NULL) {                                                                     \
			__sym = backtrace_symbols(__pbuf,__len);                                              \
			if (__sym != NULL) {                                                                  \
				APPEND_STR(__ret,__totalerr,__str,__strsize,                                      \
					"[%s:%d]SYMBOLSFUNC <DEBUG> ",__FILE__,__LINE__);                             \
				APPEND_STR(__ret,__totalerr,__str,__strsize,__VA_ARGS__);                         \
				for(__i=0;__i < __len; __i ++) {                                                  \
					APPEND_STR(__ret,__totalerr,__str,__strsize,                                  \
						"\nFUNC[%d] [%s] [%p]",__i, __sym[__i],__pbuf[__i]);                      \
					FLINFO("FUNC[%d] [%s] [%p]",__i, __sym[__i],__pbuf[__i]);                     \
				}                                                                                 \
				__output = 1;                                                                     \
			}                                                                                     \
		}                                                                                         \
	}                                                                                             \
                                                                                                  \
	if (__output == 0) {                                                                          \
		APPEND_STR(__ret,__totalerr,__str,__strsize,                                              \
			"[%s:%d]no symbols dump <DEBUG> ",__FILE__,__LINE__);                                 \
	    APPEND_STR(__ret,__totalerr,__str,__strsize,__VA_ARGS__);                                 \
		__output = 1;                                                                             \
	}                                                                                             \
	if (__output != 0) {                                                                          \
		if (__totalerr == 0 && __str != NULL) {                                                   \
			debug_printf(INFO_VERBOSE,"%s",__str);                                                \
		} else {		                                                                          \
			FLINFO("null __str");                                                                 \
		}                                                                                         \
		__output = 0;                                                                             \
	}                                                                                             \
	if (__sym) {                                                                                  \
		free(__sym);                                                                              \
		__sym = NULL;                                                                             \
	}                                                                                             \
	if (__pbuf) {                                                                                 \
		free(__pbuf);                                                                             \
		__pbuf = NULL;                                                                            \
	}                                                                                             \
	if (__str) {                                                                                  \
		free(__str);                                                                              \
		__str = NULL;                                                                             \
	}                                                                                             \
} while(0)

#else

#define  BACK_TRACE_DUMP(...)  do { fprintf(stderr,"[%s:%d] <DEBUG> ",__FILE__,__LINE__); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n"); } while(0)

#endif


/* Be careful with these macros; they evaluate the string arguments multiple
   times!
 */
#define STRDUP(s) ((s) == NULL ? NULL : strdup(s))
#define STRDUP_NOTNULL(s) ((s) == NULL ? strdup("") : strdup(s))
#define STRLEN(s) ((s) == NULL ? 0 : strlen(s))
#define STRCPY(d,s) strcpy(d,((s) == NULL ? "" : (s)))
#define DIM(ar) (sizeof(ar)/sizeof(ar[0]))

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#endif
