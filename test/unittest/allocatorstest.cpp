// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#include "unittest.h"

#include "rapidjson/allocators.h"

using namespace rapidjson;

template <typename Allocator>
void TestAllocator(Allocator& a) {
    EXPECT_TRUE(a.Malloc(0) == 0);

    uint8_t* p = static_cast<uint8_t*>(a.Malloc(100));
    EXPECT_TRUE(p != 0);
    for (size_t i = 0; i < 100; i++)
        p[i] = static_cast<uint8_t>(i);

    // Expand
    uint8_t* q = static_cast<uint8_t*>(a.Realloc(p, 100, 200));
    EXPECT_TRUE(q != 0);
    for (size_t i = 0; i < 100; i++)
        EXPECT_EQ(i, q[i]);
    for (size_t i = 100; i < 200; i++)
        q[i] = static_cast<uint8_t>(i);

    // Shrink
    uint8_t *r = static_cast<uint8_t*>(a.Realloc(q, 200, 150));
    EXPECT_TRUE(r != 0);
    for (size_t i = 0; i < 150; i++)
        EXPECT_EQ(i, r[i]);

    Allocator::Free(r);

    // Realloc to zero size
    EXPECT_TRUE(a.Realloc(a.Malloc(1), 1, 0) == 0);
}

TEST(Allocator, CrtAllocator) {
    CrtAllocator a;
    TestAllocator(a);
}

TEST(Allocator, MemoryPoolAllocator) {
    MemoryPoolAllocator<> a;
    TestAllocator(a);

    for (size_t i = 1; i < 1000; i++) {
        EXPECT_TRUE(a.Malloc(i) != 0);
        EXPECT_LE(a.Size(), a.Capacity());
    }
}

TEST(Allocator, Alignment) {
    if (sizeof(size_t) >= 8) {
        EXPECT_EQ(RAPIDJSON_UINT64_C2(0x00000000, 0x00000000), RAPIDJSON_ALIGN(0));
        for (uint64_t i = 1; i < 8; i++) {
            EXPECT_EQ(RAPIDJSON_UINT64_C2(0x00000000, 0x00000008), RAPIDJSON_ALIGN(i));
            EXPECT_EQ(RAPIDJSON_UINT64_C2(0x00000000, 0x00000010), RAPIDJSON_ALIGN(RAPIDJSON_UINT64_C2(0x00000000, 0x00000008) + i));
            EXPECT_EQ(RAPIDJSON_UINT64_C2(0x00000001, 0x00000000), RAPIDJSON_ALIGN(RAPIDJSON_UINT64_C2(0x00000000, 0xFFFFFFF8) + i));
            EXPECT_EQ(RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0xFFFFFFF8), RAPIDJSON_ALIGN(RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0xFFFFFFF0) + i));
        }
    }

    EXPECT_EQ(0u, RAPIDJSON_ALIGN(0u));
    for (uint32_t i = 1; i < 8; i++) {
        EXPECT_EQ(8u, RAPIDJSON_ALIGN(i));
        EXPECT_EQ(0xFFFFFFF8u, RAPIDJSON_ALIGN(0xFFFFFFF0u + i));
    }
}

TEST(Allocator, Issue399) {
    MemoryPoolAllocator<> a;
    void* p = a.Malloc(100);
    void* q = a.Realloc(p, 100, 200);
    EXPECT_EQ(p, q);

    // exhuasive testing
    for (size_t j = 1; j < 32; j++) {
        a.Clear();
        a.Malloc(j); // some unaligned size
        p = a.Malloc(1);
        for (size_t i = 1; i < 1024; i++) {
            q = a.Realloc(p, i, i + 1);
            EXPECT_EQ(p, q);
            p = q;
        }
    }
}
