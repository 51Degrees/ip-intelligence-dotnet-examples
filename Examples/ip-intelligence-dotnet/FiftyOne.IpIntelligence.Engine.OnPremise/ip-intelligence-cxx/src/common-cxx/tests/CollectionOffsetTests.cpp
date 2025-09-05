/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2023 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#include <gtest/gtest.h>
#include <gtest/gtest-param-test.h>
#include "TestUtils_Pointers.hpp"
#include "../collectionKeyTypes.h"

using BytesPtr = std::unique_ptr<byte[]>;
static constexpr size_t elementsCount = 131;

class CollectionOffsetTest: public testing::TestWithParam<FileOffsetUnsigned> {
public:
    BytesPtr bytes;
    byte *memoryStart = nullptr;
    byte *collectionStart = nullptr;
    CollectionPtr collection { nullptr, freeCollection };

    void rebuildCollection(FileOffsetUnsigned offset);
};

void CollectionOffsetTest::rebuildCollection(const FileOffsetUnsigned offset) {
    const size_t fullSize = (size_t)(offset + elementsCount);

    const CollectionHeader header = {
        offset, // startPosition
        elementsCount, // length
        elementsCount, // count
    };
    bytes = std::make_unique<byte[]>(fullSize);
    memoryStart = (byte *)bytes.get();
    collectionStart = memoryStart + offset;


    MemoryReader reader = {
        memoryStart, // startByte
        collectionStart, // current
        memoryStart + fullSize, // lastByte
        (FileOffset)fullSize, // length
    };
    fiftyoneDegreesCollection * const rawCollection = CollectionCreateFromMemory(
        &reader,
        header);
    this->collection = CollectionPtr(rawCollection, freeCollection);
}

TEST_P(CollectionOffsetTest, DirectWrite) {
    EXCEPTION_CREATE;
    const FileOffsetUnsigned offset = GetParam();
    rebuildCollection(offset);

    for (size_t i = 0; i < elementsCount; i++) {
        // prepare
        ItemBox item;
        const CollectionKey key {
            (uint32_t)i,
            CollectionKeyType_Byte,
        };
        auto const ptr = (const byte *)collection->get(
            collection.get(),
            &key,
            *item,
            exception);

        // set
        const byte x = (2 * i) % 255;
        collectionStart[i] = x;

        // check
        EXPECT_EQ(x, *ptr);
    }
}

TEST_P(CollectionOffsetTest, DirectRead) {
    EXCEPTION_CREATE;
    const FileOffsetUnsigned offset = GetParam();
    rebuildCollection(offset);

    for (size_t i = 0; i < elementsCount; i++) {
        // prepare
        ItemBox item;
        const CollectionKey key {
            (uint32_t)i,
            CollectionKeyType_Byte,
        };
        auto const ptr = (byte *)collection->get(
            collection.get(),
            &key,
            *item,
            exception);

        // set
        const byte x = (2 * i) % 255;
        *ptr = x;

        // check
        EXPECT_EQ(x, collectionStart[i]);
    }
}

static constexpr FileOffsetUnsigned testOffsets[] = {
    0,
    UINT16_MAX,
    UINT16_MAX + 5,
#ifdef FIFTYONE_DEGREES_LARGE_DATA_FILE_SUPPORT
    UINT32_MAX,
    UINT32_MAX + 7,
#endif
};


// VS doesn't see `INSTANTIATE_TEST_SUITE_P`
// -> https://developercommunity.visualstudio.com/t/INSTANTIATE_TEST_SUITE_P-function-defini/10409205?space=21&entry=problem&sort=newest
#if !defined(INSTANTIATE_TEST_SUITE_P) && defined(INSTANTIATE_TEST_CASE_P)
#   define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif


INSTANTIATE_TEST_SUITE_P(
    CollectionOffsetTests,
    CollectionOffsetTest,
    testing::ValuesIn(testOffsets));
