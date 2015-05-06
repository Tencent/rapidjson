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

    for (int i = 1; i < 1000; i++) {
        EXPECT_TRUE(a.Malloc(i) != 0);
        EXPECT_LE(a.Size(), a.Capacity());
    }
}
