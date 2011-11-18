#ifndef UNITTEST_H_
#define UNITTEST_H_

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#pragma warning(disable : 4996) // 'function': was declared deprecated
#endif

#include "gtest/gtest.h"

template <typename Ch>
inline size_t StrLen(const Ch* s) {
	const Ch* p = s;
	while (*p) p++;
	return p - s;
}

template<typename Ch>
inline int StrCmp(const Ch* s1, const Ch* s2) {
	while(*s1 && (*s1 == *s2)) { s1++; s2++; }
	return (unsigned)*s1 < (unsigned)*s2 ? -1 : (unsigned)*s1 > (unsigned)*s2;
}

template <typename Ch>
inline Ch* StrDup(const Ch* str) {
	size_t bufferSize = sizeof(Ch) * (StrLen(str) + 1);
	Ch* buffer = (Ch*)malloc(bufferSize);
	memcpy(buffer, str, bufferSize);
	return buffer;
}

#endif // UNITTEST_H_
