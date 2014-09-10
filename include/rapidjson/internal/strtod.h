// Copyright (C) 2011 Milo Yip
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef RAPIDJSON_STRTOD_
#define RAPIDJSON_STRTOD_

#include "pow10.h"

namespace rapidjson {
namespace internal {

inline double StrtodFastPath(double significand, int exp) {
    if (exp < -308)
        return 0.0;
    else if (exp >= 0)
        return significand * internal::Pow10(exp);
    else
        return significand / internal::Pow10(-exp);
}

inline double NormalPrecision(double d, int p) {
    if (p < -308) {
        // Prevent expSum < -308, making Pow10(p) = 0
        d = StrtodFastPath(d, -308);
        d = StrtodFastPath(d, p + 308);
    }
    else
        d = StrtodFastPath(d, p);
    return d;
}

inline double FullPrecision(bool useStrtod, double d, int p, const char* str) {
    // Use fast path for string-to-double conversion if possible
    // see http://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/
    if (!useStrtod && p > 22) {
        if (p < 22 + 16) {
            // Fast Path Cases In Disguise
            d *= internal::Pow10(p - 22);
            p = 22;
        }
        else
            useStrtod = true;
    }

    if (!useStrtod && p >= -22 && d <= 9007199254740991.0) // 2^53 - 1
        d = StrtodFastPath(d, p);
    else {
        printf("s=%s p=%d\n", str, p);
        double guess = NormalPrecision(d, p);
        d = guess;
    }
    return d;
}

} // namespace internal
} // namespace rapidjson

#endif // RAPIDJSON_STRTOD_
