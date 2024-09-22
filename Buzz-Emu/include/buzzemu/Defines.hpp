#pragma once


#if defined(__GNUC__) || defined(__clang__)
	#define	BZMU_FORCEINLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
	#define BZMU_FORCEINLINE __forceinline
#else
	#define BZMU_FORCEINLINE inline
#endif