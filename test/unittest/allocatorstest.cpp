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

#include "unittest.h"

#include "rapidjson/allocators.h"

using namespace rapidjson;

template <typename Allocator>
void TestAllocator(Allocator& a) {
    uint8_t* p = (uint8_t*)a.Malloc(100);
    EXPECT_TRUE(p != 0);
    for (size_t i = 0; i < 100; i++)
        p[i] = (uint8_t)i;

    // Expand
    uint8_t* q = (uint8_t*)a.Realloc(p, 100, 200);
    EXPECT_TRUE(q != 0);
    for (size_t i = 0; i < 100; i++)
        EXPECT_EQ(i, q[i]);
    for (size_t i = 100; i < 200; i++)
        q[i] = (uint8_t)i;

    // Shrink
    uint8_t *r = (uint8_t*)a.Realloc(q, 200, 150);
    EXPECT_TRUE(r != 0);
    for (size_t i = 0; i < 150; i++)
        EXPECT_EQ(i, r[i]);

    Allocator::Free(r);
}

TEST(Allocator, CrtAllocator) {
    CrtAllocator a;
    TestAllocator(a);
}

TEST(Allocator, MemoryPoolAllocator) {
    MemoryPoolAllocator<> a;
    TestAllocator(a);

    for (int i = 1; i < 1000; i++) {
        EXPECT_TRUE(a.Malloc(i) != 0);
        EXPECT_LE(a.Size(), a.Capacity());
    }
}
