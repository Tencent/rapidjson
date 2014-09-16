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

#ifndef RAPIDJSON_BIGINTEGER_H_
#define RAPIDJSON_BIGINTEGER_H_

#include "../rapidjson.h"

namespace rapidjson {
namespace internal {

class BigInteger {
public:
    typedef uint64_t Type;

    explicit BigInteger(uint64_t u) : count_(1) {
        digits_[0] = u;
    }

    BigInteger(const char* decimals, size_t length) : count_(1) {
        RAPIDJSON_ASSERT(length > 0);
        digits_[0] = 0;
        size_t i = 0;
        const size_t kMaxDigitPerIteration = 19;  // 2^64 = 18446744073709551616 > 10^19
        while (length >= kMaxDigitPerIteration) {
            AppendDecimal64(decimals + i, decimals + i + kMaxDigitPerIteration);
            length -= kMaxDigitPerIteration;
            i += kMaxDigitPerIteration;
        }

        if (length > 0)
            AppendDecimal64(decimals + i, decimals + i + length);
    }

    BigInteger& operator=(uint64_t u) {
        digits_[0] = u;            
        count_ = 1;
        return *this;
    }

    BigInteger& operator+=(uint64_t u) {
        Type backup = digits_[0];
        digits_[0] += u;
        for (size_t i = 0; i < count_ - 1; i++) {
            if (digits_[i] >= backup)
                return *this; // no carry
            backup = digits_[i + 1];
            digits_[i + 1] += 1;
        }

        // Last carry
        if (digits_[count_ - 1] < backup)
            PushBack(1);

        return *this;
    }

    BigInteger& operator*=(uint64_t u) {
        if (u == 0) return *this = 0;
        if (u == 1) return *this;
        uint64_t k = 0;
        for (size_t i = 0; i < count_; i++) {
            uint64_t hi;
            digits_[i] = MulAdd64(digits_[i], u, k, &hi);
            k = hi;
        }
        
        if (k > 0)
            PushBack(k);

        return *this;
    }

    BigInteger& operator*=(uint32_t u) {
        if (u == 0) return *this = 0;
        if (u == 1) return *this;
        uint32_t k = 0;
        for (size_t i = 0; i < count_; i++) {
            const uint64_t c = digits_[i] >> 32;
            const uint64_t d = digits_[i] & 0xFFFFFFFF;
            const uint64_t uc = u * c;
            const uint64_t ud = u * d;
            const uint64_t p0 = ud + k;
            const uint64_t p1 = uc + (p0 >> 32);
            digits_[i] = (p0 & 0xFFFFFFFF) | (p1 << 32);
            k = p1 >> 32;
        }
        
        if (k > 0)
            PushBack(k);

        return *this;
    }

    BigInteger& operator<<=(size_t shift) {
        if (IsZero()) return *this;

        if (shift >= kTypeBit) {
            size_t offset = shift / kTypeBit;
            RAPIDJSON_ASSERT(count_ + offset <= kCapacity);
            for (size_t i = count_; i > 0; i--)
                digits_[i - 1 + offset] = digits_[i - 1];
            for (size_t i = 0; i < offset; i++)
                digits_[i] = 0;
            count_ += offset;
            shift -= offset * kTypeBit;
        }

        if (shift > 0) {
            // Inter-digit shifts
            Type carry = 0;
            for (size_t i = 0; i < count_; i++) {
                Type newCarry = digits_[i] >> (kTypeBit - shift);
                digits_[i] = (digits_[i] << shift) | carry;
                carry = newCarry;
            }

            // Last carry
            if (carry)
                PushBack(carry);
        }

        return *this;
    }

    bool operator==(const BigInteger& rhs) const {
        return count_ == rhs.count_ && memcmp(digits_, rhs.digits_, count_ * sizeof(Type)) == 0;
    }

    BigInteger& MultiplyPow5(unsigned exp) {
        static const uint32_t kPow5[12] = {
            5,
            5 * 5,
            5 * 5 * 5,
            5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5
        };
        if (exp == 0) return *this;
        unsigned e = exp;
        for (; e >= 27; e -= 27) *this *= RAPIDJSON_UINT64_C2(0X6765C793, 0XFA10079D); // 5^27
        for (; e >= 13; e -= 13) *this *= 1220703125u; // 5^13
        if (e > 0)               *this *= kPow5[e - 1];
        return *this;
    }

    // Compute absolute difference of this and rhs.
    // Return false if this < rhs
    bool Difference(const BigInteger& rhs, BigInteger* out) const {
        int cmp = Compare(rhs);
        if (cmp == 0) {
            *out = BigInteger(0);
            return false;
        }
        const BigInteger *a, *b;  // Makes a > b
        bool ret;
        if (cmp < 0) { a = &rhs; b = this; ret = true; }
        else         { a = this; b = &rhs; ret = false; }

        Type borrow = 0;
        for (size_t i = 0; i < a->count_; i++) {
            Type d = a->digits_[i] - borrow;
            if (i < b->count_)
                d -= b->digits_[i];
            borrow = (d > a->digits_[i]) ? 1 : 0;
            out->digits_[i] = d;
            if (d != 0)
                out->count_ = i + 1;
        }

        return ret;
    }

    int Compare(const BigInteger& rhs) const {
        if (count_ != rhs.count_)
            return count_ < rhs.count_ ? -1 : 1;

        for (size_t i = count_; i-- > 0;)
            if (digits_[i] != rhs.digits_[i])
                return digits_[i] < rhs.digits_[i] ? -1 : 1;

        return 0;
    }

    size_t GetCount() const { return count_; }
    Type GetDigit(size_t index) const { RAPIDJSON_ASSERT(index < count_); return digits_[index]; }
    bool IsZero() const { return count_ == 1 && digits_[0] == 0; }

private:
    void AppendDecimal64(const char* begin, const char* end) {
        uint64_t u = ParseUint64(begin, end);
        if (IsZero())
            *this = u;
        else {
            unsigned exp = end - begin;
            (MultiplyPow5(exp) <<= exp) += u;   // *this = *this * 10^exp + u
        }
    }

    void PushBack(Type digit) {
        RAPIDJSON_ASSERT(count_ < kCapacity);
        digits_[count_++] = digit;
    }

    static uint64_t ParseUint64(const char* begin, const char* end) {
        uint64_t r = 0;
        for (const char* p = begin; p != end; ++p) {
            RAPIDJSON_ASSERT(*p >= '0' && *p <= '9');
            r = r * 10 + (*p - '0');
        }
        return r;
    }

    // Assume a * b + k < 2^128
    static uint64_t MulAdd64(uint64_t a, uint64_t b, uint64_t k, uint64_t* outHigh) {
#if defined(_MSC_VER) && defined(_M_AMD64)
        uint64_t low = _umul128(a, b, outHigh) + k;
        if (low < k)
            (*outHigh)++;
        return low;
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && defined(__x86_64__)
        unsigned __int128 p = static_cast<unsigned __int128>(a) * static_cast<unsigned __int128>(b);
        p += k;
        *outHigh = p >> 64;
        return static_cast<uint64_t>(p);
#else
        const uint64_t a0 = a & 0xFFFFFFFF, a1 = a >> 32, b0 = b & 0xFFFFFFFF, b1 = b >> 32;
        uint64_t x0 = a0 * b0, x1 = a0 * b1, x2 = a1 * b0, x3 = a1 * b1;
        x1 += (x0 >> 32); // can't give carry
        x1 += x2;
        if (x1 < x2)
            x3 += (static_cast<uint64_t>(1) << 32);
        uint64_t lo = (x1 << 32) + (x0 & 0xFFFFFFFF);
        uint64_t hi = x3 + (x1 >> 32);

        lo += k;
        if (lo < k)
            hi++;
        *outHigh = hi;
        return lo;
#endif
    }

    static Type FullAdd(Type a, Type b, bool inCarry, bool* outCarry) {
        Type c = a + b + (inCarry ? 1 : 0);
        *outCarry = c < a;
        return c;
    }

    static const size_t kBitCount = 3328;  // 64bit * 54 > 10^1000
    static const size_t kCapacity = kBitCount / sizeof(Type);
    static const size_t kTypeBit = sizeof(Type) * 8;

    Type digits_[kCapacity];
    size_t count_;
};
} // namespace internal
} // namespace rapidjson

#endif // RAPIDJSON_BIGINTEGER_H_
