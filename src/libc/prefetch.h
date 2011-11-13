/*
 *  Generic cache management functions. Everything is arch-specific,  
 *  but this header exists to make sure the defines/functions can be
 *  used in a generic way.
 *
 *  2000-11-13  Arjan van de Ven   <arjan@fenrus.demon.nl>
 *
 */

#ifndef PREFETCH_H
#define PREFETCH_H

#include "types.h"

/*
	prefetch(x) attempts to pre-emptively get the memory pointed to
	by address "x" into the CPU L1 cache. 
	prefetch(x) should not cause any kind of exception, prefetch(0) is
	specifically ok.

	prefetch() should be defined by the architecture, if not, the 
	#define below provides a no-op define.	
	
	There are 3 prefetch() macros:
	
	prefetch(x)  	- prefetches the cacheline at "x" for read
	prefetchw(x)	- prefetches the cacheline at "x" for write
	
	there is also PREFETCH_STRIDE which is the architecure-preferred 
	"lookahead" size for prefetching streamed operations.
	
*/

#define prefetch(x) __builtin_prefetch(x)

#define prefetchw(x) __builtin_prefetch(x,1)

#endif /* PREFETCH_H */
