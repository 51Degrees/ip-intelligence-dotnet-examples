/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
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

#include "../fiftyone.h"
#include "../string.h"
#include "Base.hpp"
#include "TestUtils_Pointers.hpp"
#include "../collectionKeyTypes.h"

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY
static void releaseFilePool(FilePool * const ptr) {
    if (ptr) {
        FilePoolRelease(ptr);
        delete ptr;
    }
}
using FilePoolPtr = std::unique_ptr<FilePool, decltype(&releaseFilePool)>;
using FileHandlePtr = std::unique_ptr<FileHandle, decltype(&FileHandleRelease)>;
#endif

typedef uint32_t Offset;
typedef std::vector<byte> ByteBuffer;

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY
static constexpr uint16_t requiredCollectionConcurrency = 11;

struct FileCollectionBox {
    fiftyoneDegreesCollectionConfig config = {
        false, // loaded
        0, // capacity
        requiredCollectionConcurrency, // concurrency
    };
    FileHandlePtr handle { nullptr, FileHandleRelease };
    CollectionPtr ptr { nullptr, freeCollection };
};
#endif

class StoredBinaryValues : public Base {
public:
    void SetUp() override;
    void TearDown() override;

    ByteBuffer rawStringsBuffer;
    byte *dataStart = nullptr;
    struct {
        Offset string1;
        Offset string2;
        Offset ipv4;
        Offset ipv6;
        Offset wkb;
        Offset shortValue;
        Offset floatValue;
        Offset intValue;
        Offset byteValue;
    } offsets = {};

#   ifndef FIFTYONE_DEGREES_MEMORY_ONLY
    struct FileProps {
        const uint16_t totalConcurrency = 2 * requiredCollectionConcurrency;
        FilePoolPtr pool { nullptr, releaseFilePool };
        CollectionHeader header = {};
    } file = {};
#   endif

    CollectionHeader header = {};
    struct {
        CollectionPtr memory { nullptr, freeCollection };
#       ifndef FIFTYONE_DEGREES_MEMORY_ONLY
        FileCollectionBox fileNoCache = {{ false, 0, requiredCollectionConcurrency }};
        FileCollectionBox fileCache = {
            {
                false,
                requiredCollectionConcurrency * requiredCollectionConcurrency,
                requiredCollectionConcurrency,
            }};
        FileCollectionBox fileLoadedNoCache = {{ true, 0, 0 }};
        FileCollectionBox fileLoadedCache = {{ true, requiredCollectionConcurrency, 0 }};
#       endif
    } collection;
};

static constexpr char string1_rawValueBytes[] = "\x12\0some-string-value";
static constexpr char string2_rawValueBytes[] = "\xE\0another-world";
static constexpr byte ipv4_rawValueBytes[] = {
    0x04, 0x00,
    0xD4, 0x0C, 0x00, 0x01,
};
static constexpr byte ipv6_rawValueBytes[] = {
    0x10, 0x00,
    0x20,0x01,0x0d,0xb8,
    0x85,0xa3,0x00,0x00,
    0x00,0x00,0x8a,0x2e,
    0x03,0x70,0x73,0x34,
};
static constexpr byte wkb_rawValueBytes[] = {
    0x15, 0x00,
    0x00,
    0x00, 0x00, 0x00, 0x01,
    0x40, 0x31, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x8b, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00,
};

static constexpr byte shortValue_rawValueBytes[] = {
    0x60, 0x60,
};
static constexpr int16_t shortValue_rawValue = 24672;
static constexpr double shortValue_azimuth = 135.53147984252449110385448774682;
static constexpr double shortValue_declination = 67.765739921262245551927243873409;

static constexpr byte floatValue_rawValueBytes[] = {
    0xCE, 0x2E, 0x88, 0x04,
};
static constexpr float floatValue_rawValue = 3.201642965935704e-36f;

static constexpr byte byteValue_rawValueBytes[] = {
    0x2f
};
static constexpr byte byteValue_rawValue = 0x2f;

static constexpr byte intValue_rawValueBytes[] = {
    0x65, 0x7A, 0x20, 0x52,
};
static constexpr int intValue_rawValue = 1377860197;

static CollectionPtr buildMemoryCollection(
    ByteBuffer &rawStringsBuffer,
    const uint16_t fileHeaderSize,
    const CollectionHeader &header) {
    byte * const ptr = rawStringsBuffer.data();
    fiftyoneDegreesMemoryReader reader = {
        ptr, // startByte
        ptr + fileHeaderSize, // current
        ptr + fileHeaderSize + header.length, // lastByte
        (FileOffset)header.length, // length
    };
    fiftyoneDegreesCollection * const collection = CollectionCreateFromMemory(
        &reader,
        header);
    CollectionPtr result(collection, freeCollection);
    return result;
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY
static constexpr char fileName[] = "StoredBinaryValueTests_Data.hex";

static void prepareFileProps(StoredBinaryValues::FileProps &fileProps) {
    EXCEPTION_CREATE;

    auto const pool = new FilePool();
    FilePoolInit(
        pool,
        fileName,
        fileProps.totalConcurrency,
        exception);
    EXCEPTION_THROW;
    fileProps.pool = FilePoolPtr(pool, releaseFilePool);
}

static void buildFileCollection(
    StoredBinaryValues::FileProps &fileProps,
    const CollectionHeader &header,
    FileCollectionBox &outBox) {
    EXCEPTION_CREATE;

    outBox.handle = FileHandlePtr(
        FileHandleGet(fileProps.pool.get(), exception),
        FileHandleRelease);
    EXCEPTION_THROW;

    fileProps.header = header;
    // fileProps.header = CollectionHeaderFromFile(
    //     outBox.handle->file,
    //     1,
    //     true);

    fiftyoneDegreesCollection * const collection = CollectionCreateFromFile(
        outBox.handle->file,
        fileProps.pool.get(),
        &outBox.config,
        fileProps.header,
        StoredBinaryValueRead);
    outBox.ptr = {collection, freeCollection};
}
#endif

void StoredBinaryValues::SetUp() {
    // add some junk into start to emulate file header
    const size_t fileHeaderSize = sizeof(uint32_t);
    rawStringsBuffer.assign(fileHeaderSize, 0);
    rawStringsBuffer.push_back(42);
    rawStringsBuffer.push_back(29);
    rawStringsBuffer.push_back(13);
    // add contents
#   define add_value_to_buffer(x) \
    offsets.x = (Offset)rawStringsBuffer.size() - fileHeaderSize; \
    for (size_t i = 0; i < sizeof(x##_rawValueBytes); i++) { \
        rawStringsBuffer.push_back(x##_rawValueBytes[i]); \
    }
    add_value_to_buffer(string1)
    add_value_to_buffer(string2)
    add_value_to_buffer(ipv4)
    add_value_to_buffer(ipv6)
    add_value_to_buffer(wkb)
    add_value_to_buffer(shortValue)
    add_value_to_buffer(floatValue)
    add_value_to_buffer(intValue)
    add_value_to_buffer(byteValue)
#   undef add_value_to_buffer

    header = {
        fileHeaderSize, // startPosition
        (uint32_t)(rawStringsBuffer.size() - fileHeaderSize), // length
        (uint32_t)(rawStringsBuffer.size() - fileHeaderSize), // count
    };
    for (size_t i = 0; i < fileHeaderSize; i++) {
        rawStringsBuffer[i] = ((const byte*)&header.length)[i];
    }

    dataStart = rawStringsBuffer.data() + fileHeaderSize;
#   ifndef FIFTYONE_DEGREES_MEMORY_ONLY
    FileWrite(fileName, rawStringsBuffer.data(), rawStringsBuffer.size());
#   endif

    collection.memory = buildMemoryCollection(rawStringsBuffer, fileHeaderSize, header);
#   ifndef FIFTYONE_DEGREES_MEMORY_ONLY
    prepareFileProps(file);
    buildFileCollection(file, header, collection.fileNoCache);
    buildFileCollection(file, header, collection.fileCache);
    buildFileCollection(file, header, collection.fileLoadedNoCache);
    buildFileCollection(file, header, collection.fileLoadedCache);
#   endif
}

void StoredBinaryValues::TearDown() {
}


// ============== StoredBinaryValueGet (from memory) ==============


TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_Direct_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const fiftyoneDegreesCollectionKey key {
        offsets.string1,
        CollectionKeyType_String,
    };
    auto * const value = (String *)collection.memory->get(
        collection.memory.get(),
        &key,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.string1, (byte *)value);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.string1,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.string1, (byte *)value);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String2_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.string2,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.string2, (byte *)value);
    ASSERT_EQ(sizeof(string2_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string2_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string2_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv4_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.ipv4,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.ipv4, (byte *)value);
    ASSERT_EQ(sizeof(ipv4_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv4_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv4_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv6_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.ipv6,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.ipv6, (byte *)value);
    ASSERT_EQ(sizeof(ipv6_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv6_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv6_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_WKB_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.wkb,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.wkb, (byte *)value);
    ASSERT_EQ(sizeof(wkb_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(wkb_rawValueBytes) - 2; i++) {
        ASSERT_EQ(wkb_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Azimuth_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.shortValue, (byte *)value);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_azimuth, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Declination_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.shortValue, (byte *)value);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_declination, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Float_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.floatValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.floatValue, (byte *)value);
    for (size_t i = 0; i < sizeof(floatValue_rawValueBytes); i++) {
        ASSERT_EQ(floatValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(floatValue_rawValue, FLOAT_TO_NATIVE(value->floatValue));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Integer_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.intValue, (byte *)value);
    for (size_t i = 0; i < sizeof(intValue_rawValueBytes); i++) {
        ASSERT_EQ(intValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(intValue_rawValue, value->intValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Byte_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.byteValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        *item,
        exception);
    ASSERT_EQ(dataStart + offsets.byteValue, (byte*)value);
    for (size_t i = 0; i < sizeof(byteValue_rawValueBytes); i++) {
        ASSERT_EQ(byteValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(byteValue_rawValue, value->byteValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Object_FromMemory) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.memory.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        *item,
        exception);
    // In-Memory collection does NOT care about property type
    // => no exception
    ASSERT_TRUE(EXCEPTION_OKAY);
    ASSERT_EQ(dataStart + offsets.intValue, (byte *)value);
    for (size_t i = 0; i < sizeof(intValue_rawValueBytes); i++) {
        ASSERT_EQ(intValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(intValue_rawValue, value->intValue);
}


#ifndef FIFTYONE_DEGREES_MEMORY_ONLY


// ============== StoredBinaryValueGet (from file loaded no cache) ==============


TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_Direct_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const fiftyoneDegreesCollectionKey key {
        offsets.string1,
        CollectionKeyType_String,
    };
    auto * const value = (String *)collection.fileLoadedNoCache.ptr->get(
        collection.fileLoadedNoCache.ptr.get(),
        &key,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.string1,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String2_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.string2,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(string2_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string2_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string2_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv4_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.ipv4,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(ipv4_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv4_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv4_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv6_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.ipv6,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(ipv6_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv6_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv6_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_WKB_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.wkb,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(wkb_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(wkb_rawValueBytes) - 2; i++) {
        ASSERT_EQ(wkb_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Azimuth_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_azimuth, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Declination_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_declination, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Float_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.floatValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(floatValue_rawValueBytes); i++) {
        ASSERT_EQ(floatValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(floatValue_rawValue, FLOAT_TO_NATIVE(value->floatValue));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Integer_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(intValue_rawValueBytes); i++) {
        ASSERT_EQ(intValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(intValue_rawValue, value->intValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Byte_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.byteValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(byteValue_rawValueBytes); i++) {
        ASSERT_EQ(byteValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(byteValue_rawValue, value->byteValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Object_FromFileLoadedNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedNoCache.ptr.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        *item,
        exception);
    // In-Memory collection does NOT care about property type
    // => no exception
    ASSERT_TRUE(EXCEPTION_OKAY);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(intValue_rawValueBytes); i++) {
        ASSERT_EQ(intValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(intValue_rawValue, value->intValue);
}


// ============== StoredBinaryValueGet (from file loaded cache) ==============


TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_Direct_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const fiftyoneDegreesCollectionKey key {
        offsets.string1,
        CollectionKeyType_String,
    };
    auto * const value = (String *)collection.fileLoadedCache.ptr->get(
        collection.fileLoadedCache.ptr.get(),
        &key,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.string1,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String2_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.string2,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(string2_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string2_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string2_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv4_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.ipv4,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(ipv4_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv4_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv4_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv6_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.ipv6,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(ipv6_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv6_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv6_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_WKB_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.wkb,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    ASSERT_EQ(sizeof(wkb_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(wkb_rawValueBytes) - 2; i++) {
        ASSERT_EQ(wkb_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Azimuth_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_azimuth, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Declination_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_declination, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Float_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.floatValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(floatValue_rawValueBytes); i++) {
        ASSERT_EQ(floatValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(floatValue_rawValue, FLOAT_TO_NATIVE(value->floatValue));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Integer_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(intValue_rawValueBytes); i++) {
        ASSERT_EQ(intValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(intValue_rawValue, value->intValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Byte_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.byteValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(byteValue_rawValueBytes); i++) {
        ASSERT_EQ(byteValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(byteValue_rawValue, value->byteValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Object_FromFileLoadedCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileLoadedCache.ptr.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        *item,
        exception);
    // In-Memory collection does NOT care about property type
    // => no exception
    ASSERT_TRUE(EXCEPTION_OKAY);
    ASSERT_EQ(0, item->data.allocated);
    for (size_t i = 0; i < sizeof(intValue_rawValueBytes); i++) {
        ASSERT_EQ(intValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(intValue_rawValue, value->intValue);
}


// ============== StoredBinaryValueGet (from file cache) ==============


TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_Direct_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const fiftyoneDegreesCollectionKey key {
        offsets.string1,
        CollectionKeyType_String,
    };
    auto * const value = (String *)collection.fileCache.ptr->get(
        collection.fileCache.ptr.get(),
        &key,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(string1_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.string1,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(string1_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String2_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.string2,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(string2_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(string2_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string2_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string2_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv4_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.ipv4,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(ipv4_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(ipv4_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv4_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv4_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv6_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.ipv6,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(ipv6_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(ipv6_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv6_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv6_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_WKB_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.wkb,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(wkb_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(wkb_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(wkb_rawValueBytes) - 2; i++) {
        ASSERT_EQ(wkb_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Azimuth_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(shortValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_azimuth, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Declination_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(shortValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_declination, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Float_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.floatValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(floatValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(floatValue_rawValueBytes); i++) {
        ASSERT_EQ(floatValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(floatValue_rawValue, FLOAT_TO_NATIVE(value->floatValue));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Integer_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(intValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(intValue_rawValueBytes); i++) {
        ASSERT_EQ(intValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(intValue_rawValue, value->intValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Byte_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.byteValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(byteValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(byteValue_rawValueBytes); i++) {
        ASSERT_EQ(byteValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(byteValue_rawValue, value->byteValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Object_FromFileCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileCache.ptr.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        *item,
        exception);
    ASSERT_FALSE(EXCEPTION_OKAY);
    ASSERT_EQ(UNSUPPORTED_STORED_VALUE_TYPE, exception->status);
    ASSERT_FALSE(value);
}


// ============== StoredBinaryValueGet (from file no cache) ==============


TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_Direct_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const fiftyoneDegreesCollectionKey key {
        offsets.string1,
        CollectionKeyType_String,
    };
    auto * const value = (String *)collection.fileNoCache.ptr->get(
        collection.fileNoCache.ptr.get(),
        &key,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(string1_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String1_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.string1,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(string1_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(string1_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string1_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string1_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_String2_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.string2,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(string2_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(string2_rawValueBytes) - 2, value->stringValue.size);
    for (size_t i = 0; i < sizeof(string2_rawValueBytes) - 2; i++) {
        ASSERT_EQ(string2_rawValueBytes[i + 2], (&value->stringValue.value)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv4_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.ipv4,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(ipv4_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(ipv4_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv4_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv4_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_IPv6_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.ipv6,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(ipv6_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(ipv6_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(ipv6_rawValueBytes) - 2; i++) {
        ASSERT_EQ(ipv6_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_WKB_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.wkb,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(wkb_rawValueBytes), item->data.allocated);
    ASSERT_EQ(sizeof(wkb_rawValueBytes) - 2, value->byteArrayValue.size);
    for (size_t i = 0; i < sizeof(wkb_rawValueBytes) - 2; i++) {
        ASSERT_EQ(wkb_rawValueBytes[i + 2], (&value->byteArrayValue.firstByte)[i]);
    }
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Azimuth_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(shortValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_azimuth, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Declination_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.shortValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(shortValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(shortValue_rawValueBytes); i++) {
        ASSERT_EQ(shortValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(shortValue_rawValue, value->shortValue);
    ASSERT_EQ(shortValue_declination, StoredBinaryValueToDoubleOrDefault(
        value,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        0));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Float_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.floatValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(floatValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(floatValue_rawValueBytes); i++) {
        ASSERT_EQ(floatValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(floatValue_rawValue, FLOAT_TO_NATIVE(value->floatValue));
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Integer_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(intValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(intValue_rawValueBytes); i++) {
        ASSERT_EQ(intValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(intValue_rawValue, value->intValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Byte_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.byteValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        *item,
        exception);
    EXCEPTION_THROW;
    ASSERT_NE(nullptr, value);
    ASSERT_EQ(sizeof(byteValue_rawValueBytes), item->data.allocated);
    for (size_t i = 0; i < sizeof(byteValue_rawValueBytes); i++) {
        ASSERT_EQ(byteValue_rawValueBytes[i], ((const byte *)value)[i]);
    }
    ASSERT_EQ(byteValue_rawValue, value->byteValue);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_Get_Object_FromFileNoCache) {
    EXCEPTION_CREATE;
    ItemBox item;
    const StoredBinaryValue * const value = StoredBinaryValueGet(
        collection.fileNoCache.ptr.get(),
        offsets.intValue,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        *item,
        exception);
    ASSERT_FALSE(EXCEPTION_OKAY);
    ASSERT_EQ(UNSUPPORTED_STORED_VALUE_TYPE, exception->status);
    ASSERT_FALSE(value);
}


#endif


// ============== StoredBinaryValueToIntOrDefault ==============


TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_String_DoubleZero) {
    const char rawBytes[] = { 3, 0, '0', '0', 0 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_String_x7) {
    const char rawBytes[] = { 3, 0, 'x', '7', 0 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_String_7x4) {
    const char rawBytes[] = { 4, 0, '7', 'x', '4', 0 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(7, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_String_15) {
    const char rawBytes[] = { 3, 0, '1', '5', 0 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(15, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_String_minus827) {
    const char rawBytes[] = { 5, 0, '-', '8', '2', '7', 0 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(-827, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_String_3_95) {
    const char rawBytes[] = { 5, 0, '3', '.', '9', '5', 0 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(3, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Byte_0) {
    const byte rawByte = 0;
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)&rawByte,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Byte_7) {
    const byte rawByte = 7;
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)&rawByte,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        -1);
    EXPECT_EQ(7, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Integer_0) {
    const char rawBytes[] = { 0, 0, 0, 0 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Integer_Positive) {
    const unsigned char rawBytes[] = { 0x50, 0xBE, 0x09, 0x1B };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        -1);
    EXPECT_EQ(453623376, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Integer_Negative) {
    const unsigned char rawBytes[] = { 0xB0, 0x41, 0xF6, 0xE4 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        -1);
    EXPECT_EQ(-453623376, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Float_Zero) {
    const unsigned char rawBytes[] = { 0, 0, 0, 0 }; // 0.0
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Float_Positive) {
    const unsigned char rawBytes[] = { 0xCD, 0x5A, 0x34, 0x43 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(180, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Float_LargePositive) {
    const unsigned char rawBytes[] = { 0x5F, 0x43, 0xC2, 0x47 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(99462, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Float_Negative) {
    const unsigned char rawBytes[] = { 0x9E, 0x86, 0x90, 0xC2 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(-72, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Float_LargeNegative) {
    const unsigned char rawBytes[] = { 0xA6, 0x0B, 0xA0, 0xC7 };
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(-81943, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Azimuth) {
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        -1);
    EXPECT_EQ((int)shortValue_azimuth, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Declination) {
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        -1);
    EXPECT_EQ((int)shortValue_declination, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Default_minus1) {
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        -1);
    EXPECT_EQ(-1, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToInt_Default_542) {
    const int result = StoredBinaryValueToIntOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        542);
    EXPECT_EQ(542, result);
}


// ============== StoredBinaryValueToDoubleOrDefault ==============


TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_String_DoubleZero) {
    const char rawBytes[] = { 3, 0, '0', '0', 0 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_String_x7) {
    const char rawBytes[] = { 3, 0, 'x', '7', 0 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_String_7x4) {
    const char rawBytes[] = { 4, 0, '7', 'x', '4', 0 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(7, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_String_15) {
    const char rawBytes[] = { 3, 0, '1', '5', 0 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(15, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_String_minus827) {
    const char rawBytes[] = { 5, 0, '-', '8', '2', '7', 0 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(-827, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_String_3_95) {
    const char rawBytes[] = { 5, 0, '3', '.', '9', '5', 0 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        -1);
    EXPECT_EQ(3.95, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Byte_0) {
    const byte rawByte = 0;
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)&rawByte,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Byte_7) {
    const byte rawByte = 7;
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)&rawByte,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        -1);
    EXPECT_EQ(7, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Integer_0) {
    const char rawBytes[] = { 0, 0, 0, 0 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Integer_Positive) {
    const unsigned char rawBytes[] = { 0x50, 0xBE, 0x09, 0x1B };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        -1);
    EXPECT_EQ(453623376, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Integer_Negative) {
    const unsigned char rawBytes[] = { 0xB0, 0x41, 0xF6, 0xE4 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        -1);
    EXPECT_EQ(-453623376, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Float_Zero) {
    const unsigned char rawBytes[] = { 0, 0, 0, 0 }; // 0.0
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(0, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Float_Positive) {
    const unsigned char rawBytes[] = { 0xCD, 0x5A, 0x34, 0x43 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(180.3546905517578125, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Float_LargePositive) {
    const unsigned char rawBytes[] = { 0x5F, 0x43, 0xC2, 0x47 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(99462.7421875, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Float_Negative) {
    const unsigned char rawBytes[] = { 0x9E, 0x86, 0x90, 0xC2 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(-72.2629241943359375, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Float_LargeNegative) {
    const unsigned char rawBytes[] = { 0xA6, 0x0B, 0xA0, 0xC7 };
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        -1);
    EXPECT_EQ(-81943.296875, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Azimuth) {
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        -1);
    EXPECT_EQ(shortValue_azimuth, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Declination) {
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        -1);
    EXPECT_EQ(shortValue_declination, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Default_minus1) {
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        -1);
    EXPECT_EQ(-1, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToDouble_Default_54_2) {
    const double result = StoredBinaryValueToDoubleOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        54.2);
    EXPECT_EQ(54.2, result);
}


// ============== StoredBinaryValueToBoolOrDefault ==============


TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_True) {
    const char rawBytes[] = { 5, 0, 't', 'r', 'u', 'e', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_TrueCapitalized) {
    const char rawBytes[] = { 5, 0, 'T', 'r', 'u', 'e', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_TrueAllCaps) {
    const char rawBytes[] = { 5, 0, 'T', 'R', 'U', 'E', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_TrueMixed) {
    const char rawBytes[] = { 5, 0, 't', 'R', 'U', 'e', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_Trueish) {
    const char rawBytes[] = { 8, 0, 't', 'r', 'u', 'e', 'i', 's', 'h', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_DoubleZero) {
    const char rawBytes[] = { 3, 0, '0', '0', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_x7) {
    const char rawBytes[] = { 3, 0, 'x', '7', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_7x4) {
    const char rawBytes[] = { 4, 0, '7', 'x', '4', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_15) {
    const char rawBytes[] = { 3, 0, '1', '5', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_minus827) {
    const char rawBytes[] = { 5, 0, '-', '8', '2', '7', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_String_3_95) {
    const char rawBytes[] = { 5, 0, '3', '.', '9', '5', 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Integer_0) {
    const char rawBytes[] = { 0, 0, 0, 0 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Byte_0) {
    const byte rawByte = 0;
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)&rawByte,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Byte_7) {
    const byte rawByte = 7;
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)&rawByte,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Integer_Positive) {
    const unsigned char rawBytes[] = { 0x50, 0xBE, 0x09, 0x1B };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Integer_Negative) {
    const unsigned char rawBytes[] = { 0xB0, 0x41, 0xF6, 0xE4 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Float_Zero) {
    const unsigned char rawBytes[] = { 0, 0, 0, 0 }; // 0.0
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Float_Positive) {
    const unsigned char rawBytes[] = { 0xCD, 0x5A, 0x34, 0x43 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Float_LargePositive) {
    const unsigned char rawBytes[] = { 0x5F, 0x43, 0xC2, 0x47 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Float_Negative) {
    const unsigned char rawBytes[] = { 0x9E, 0x86, 0x90, 0xC2 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Float_LargeNegative) {
    const unsigned char rawBytes[] = { 0xA6, 0x0B, 0xA0, 0xC7 };
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)rawBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Azimuth) {
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_AZIMUTH,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Declination) {
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DECLINATION,
        false);
    EXPECT_EQ(true, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Default_minus1) {
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        false);
    EXPECT_EQ(false, result);
}

TEST_F(StoredBinaryValues, StoredBinaryValue_ToBool_Default_54_2) {
    const byte result = StoredBinaryValueToBoolOrDefault(
        (const StoredBinaryValue *)shortValue_rawValueBytes,
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_OBJECT,
        true);
    EXPECT_EQ(true, result);
}
