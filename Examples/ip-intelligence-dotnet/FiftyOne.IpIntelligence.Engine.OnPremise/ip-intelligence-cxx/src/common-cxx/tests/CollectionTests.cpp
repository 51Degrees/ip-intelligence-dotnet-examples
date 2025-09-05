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

#include "pch.h"
#include <sstream>
#include "Base.hpp"
#include "FileHandle.hpp"
#include "TestStrings.hpp"
#include "../CollectionConfig.hpp"
#include "../Exceptions.hpp"
#include "../collection.h"
#include "../collectionKeyTypes.h"
#include "../list.h"

using namespace FiftyoneDegrees::Common;

class CollectionTestData {
public:
	const fiftyoneDegreesCollectionKeyType keyType;
	CollectionTestData(
		uint32_t count,
		const fiftyoneDegreesCollectionKeyType &keyType): keyType(keyType), count(count) {
		map = new uint32_t[count];
		data = nullptr;
	}
	virtual ~CollectionTestData() {
		if (map != nullptr) {
			delete [] map;
			map = nullptr;
		}
		if (data != nullptr) {
			delete [] data;
			data = nullptr;
		}
	}
#ifdef _MSC_VER
#pragma warning (disable: 4100) 
#endif
	virtual void verify(fiftyoneDegreesData *itemData, int index) {}
#ifdef _MSC_VER
#pragma warning (default: 4100) 
#endif
	virtual uint32_t outOfRange() { return 0; }
	fiftyoneDegreesCollectionItemComparer itemComparer;
	const uint32_t count;
	byte *data;
	size_t size;
	uint32_t elementSize;
	bool isCount;
	uint32_t *map;
};

class CollectionTestDataFixed : public CollectionTestData {
public:
	CollectionTestDataFixed(uint32_t count) : CollectionTestData(count, *CollectionKeyType_Integer) {
		elementSize = sizeof(int);
		size = (count * elementSize) + sizeof(uint32_t);
		data = new byte[size];
		uint32_t * const values = (uint32_t*)(data + sizeof(uint32_t));
		for (uint32_t i = 0; i < count; i++) {
			values[i] = i;
			map[i] = i;
		}
		itemComparer = itemComparerInt;
	}
	void verify(fiftyoneDegreesData *itemData, int index) {
		EXPECT_EQ(index, *(int*)itemData->ptr);
	}
	uint32_t outOfRange() {
		return map[count - 1] + 1;
	}

	static int itemComparerInt(
		void *state,
		fiftyoneDegreesCollectionItem *item,
		fiftyoneDegreesCollectionKey key,
		fiftyoneDegreesException *exception) {
#		ifdef _MSC_VER
		UNREFERENCED_PARAMETER(key);
		UNREFERENCED_PARAMETER(exception);
#		endif
		const uint32_t * const itemValue = (const uint32_t*)item->data.ptr;
		const uint32_t * const stateValue = (const uint32_t*)state;
		return *itemValue - *stateValue;
	}
};

class CollectionTestDataVariable : public CollectionTestData {
public:
	CollectionTestDataVariable(uint32_t count) : CollectionTestData(count, *CollectionKeyType_String) {
		size = sizeof(uint32_t);
		for (uint32_t i = 0; i < count; i++) {
			size += strlen(TEST_STRINGS[i]) + 1;
		}
		data = new byte[size];
		byte *current = data + sizeof(uint32_t);
		for (uint32_t i = 0; i < count; i++) {
			map[i] = (uint32_t)(current - data - sizeof(uint32_t));
			strcpy((char*)current, TEST_STRINGS[i]);
			current += strlen(TEST_STRINGS[i]);
			current++;
		}
		elementSize = 0;
		itemComparer = itemComparerString;
	}
	void verify(fiftyoneDegreesData *itemData, int index) {
		EXPECT_EQ(strcmp(TEST_STRINGS[index], (const char*)itemData->ptr), 0);
	}
	uint32_t outOfRange() {
		return (uint32_t)(size + 1);
	}

	static int itemComparerString(
		void *state,
		fiftyoneDegreesCollectionItem *item,
		fiftyoneDegreesCollectionKey key,
		fiftyoneDegreesException *exception) {
#		ifdef _MSC_VER
		UNREFERENCED_PARAMETER(key);
		UNREFERENCED_PARAMETER(exception);
#		endif
		const char *itemString = FIFTYONE_DEGREES_STRING(item->data.ptr);
		const char *targetString = (const char *)state;
		return strcmp(itemString, targetString);
	}
};

class CollectionTestDataVariableSize : public CollectionTestDataVariable {
public:
	CollectionTestDataVariableSize(uint32_t count)
		: CollectionTestDataVariable(count) {
		*(uint32_t*)data = (uint32_t)(size - sizeof(uint32_t));
		isCount = false;
	}
};

class CollectionTestDataFixedCount : public CollectionTestDataFixed {
public:
	CollectionTestDataFixedCount(uint32_t count)
		: CollectionTestDataFixed(count) {
		*(uint32_t*)data = (uint32_t)count;
		isCount = true;
	}
};

class CollectionTestDataFixedSize : public CollectionTestDataFixed {
public:
	CollectionTestDataFixedSize(uint32_t count)
		: CollectionTestDataFixed(count) {
		*(uint32_t*)data = (uint32_t)(size - sizeof(uint32_t));
		isCount = false;
	}
};

class CollectionTest : public Base {
public:
	CollectionTest(
		CollectionConfig *config, 
		CollectionTestData *data) { 
		this->config = config;
		this->data = data;
		collection = NULL;
	}
	~CollectionTest() {
        if (data != NULL) {
            delete data;
        }
        if (config != NULL) {
            delete config;
        }
	}
	virtual void SetUp() { Base::SetUp(); }
	void TearDown() {
		if (collection != NULL) {
			collection->freeCollection(collection);
			collection = NULL;
		}
		Base::TearDown();
	}

	void verify() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		fiftyoneDegreesCollectionItem item;
		fiftyoneDegreesDataReset(&item.data);
		for (uint32_t i = 0; i < data->count; i++) {
			const fiftyoneDegreesCollectionKey key {
				data->map[i],
				&data->keyType,
			};
			collection->get(
				collection,
				&key,
				&item,
				exception);
			FIFTYONE_DEGREES_EXCEPTION_THROW
			data->verify(&item.data, i);
			if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {
				FIFTYONE_DEGREES_COLLECTION_RELEASE(collection, &item);
			}
		}
	}

	void binarySearch() {
		if (this->data->isCount == false) {
			cout << "Skipping binary search test for as the collection "
				<< "contains variable size elements.\n";
		}
		else {
			FIFTYONE_DEGREES_EXCEPTION_CREATE;
			fiftyoneDegreesCollectionItem resultItem, targetItem;
			fiftyoneDegreesDataReset(&resultItem.data);
			fiftyoneDegreesDataReset(&targetItem.data);

			for (uint32_t i = 0; i < data->count; i++) {
				const fiftyoneDegreesCollectionKey key {
					data->map[i],
					&data->keyType,
				};
				auto const theValue = (const uint32_t *)collection->get(
					collection,
					&key,
					&targetItem,
					exception);
#				ifdef _MSC_VER
				UNREFERENCED_PARAMETER(theValue);
#				endif
				FIFTYONE_DEGREES_EXCEPTION_THROW;
				const fiftyoneDegreesCollectionIndexOrOffset end = {data->count - 1};
				const long index = fiftyoneDegreesCollectionBinarySearch(
					collection,
					&resultItem,
					fiftyoneDegreesCollectionIndexOrOffset_Zero,
					end,
					&data->keyType,
					targetItem.data.ptr,
					data->itemComparer,
					exception);
				EXPECT_GE(index, 0);
				EXPECT_FALSE(FIFTYONE_DEGREES_EXCEPTION_FAILED);
				data->verify(&resultItem.data, i);
				if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {
					FIFTYONE_DEGREES_COLLECTION_RELEASE(collection, &resultItem);
					FIFTYONE_DEGREES_COLLECTION_RELEASE(collection, &targetItem);
				}
			}
		}
	}

	void binarySearch_notFound() {
		if (this->data->isCount == false) {
			cout << "Skipping binary search test for as the collection "
				<< "contains variable size elements.\n";
		}
		else {
			FIFTYONE_DEGREES_EXCEPTION_CREATE;
			fiftyoneDegreesCollectionItem resultItem, targetItem;
			fiftyoneDegreesDataReset(&resultItem.data);
			fiftyoneDegreesDataReset(&targetItem.data);

			int DummyIDs[] = {
				-5,
				(int)data->count + 3,
			};
			for (uint32_t i = 0; i < sizeof(DummyIDs) / sizeof(DummyIDs[0]); i++) {
				const fiftyoneDegreesCollectionIndexOrOffset end = {data->count - 1};
				EXPECT_EQ(-1, fiftyoneDegreesCollectionBinarySearch(
					collection,
					&resultItem,
					fiftyoneDegreesCollectionIndexOrOffset_Zero,
					end,
					&data->keyType,
					&(DummyIDs[i]),
					data->itemComparer,
					exception));
				EXPECT_FALSE(FIFTYONE_DEGREES_EXCEPTION_FAILED);
			}
		}
	}

	void outOfRange() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		fiftyoneDegreesCollectionItem item;
		fiftyoneDegreesDataReset(&item.data);
		const fiftyoneDegreesCollectionKey key {
			data->outOfRange(),
			&data->keyType,
		};
		EXPECT_EQ(collection->get(
			collection,
			&key,
			&item,
			exception), nullptr) << "Returned pointer should always be "
			"null if out of range";
		EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_FAILED);
		EXPECT_EQ(item.data.ptr, nullptr) << "Item data pointer should be "
			"null when an invalid index or offset is provided";
		EXPECT_EQ(item.data.allocated, 0) << "Item data allocation should be "
			"0 when an invalid index or offset is provided";
		EXPECT_EQ(item.data.used, 0) << "Item data usage should be 0 when an "
			"invalid index or offset is provided";
		if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {
			FIFTYONE_DEGREES_COLLECTION_RELEASE(collection, &item);
		}
	}

	void random() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		fiftyoneDegreesCollectionItem item;
		fiftyoneDegreesDataReset(&item.data);
		for (uint32_t i = 0; i < data->count; i++) {
			uint32_t index = (uint32_t)(rand() % data->count);
			ASSERT_LT(index, data->count) << "Random index must be less than "
			"the count of available test data items";
			const fiftyoneDegreesCollectionKey key {
					data->map[index],
				&data->keyType,
			};
			EXPECT_NE(collection->get(
				collection,
				&key,
				&item, 
				exception), nullptr);
			FIFTYONE_DEGREES_EXCEPTION_THROW
			data->verify(&item.data, index);
			if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {
				FIFTYONE_DEGREES_COLLECTION_RELEASE(collection, &item);
			}
		}
	}

	void list(double percentage) {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		fiftyoneDegreesList list;
		uint32_t capacity = (uint32_t)(data->count * percentage);
		int inc = data->count / capacity;
		fiftyoneDegreesListInit(&list, capacity);

		// Add every X items from the collection to the list.
		uint32_t i = 0;
		while (list.count < list.capacity && i < data->count) {
			fiftyoneDegreesCollectionItem item;
			fiftyoneDegreesDataReset(&item.data);
			const fiftyoneDegreesCollectionKey key {
				data->map[i],
				&data->keyType,
			};
			EXPECT_NE(collection->get(
				collection,
				&key,
				&item,
				exception), nullptr);
			FIFTYONE_DEGREES_EXCEPTION_THROW
			fiftyoneDegreesListAdd(&list, &item);
			i += inc;
		}

		// Do something else now with the collection whilst the items that are
		// in the list are assigned to it and not available to be released.
		random();

		// Retrieve the list items that were added before the random test.
		i = 0;
		while (list.count < list.capacity && i < data->count) {
			data->verify(&list.items[i].data, i);
			i += inc;
		}

		// Release the items from the list and free the memory.
		fiftyoneDegreesListFree(&list);
	}

	static void randomMultiThreadedRunThread(void* state) {
		((CollectionTest*)state)->random();
		FIFTYONE_DEGREES_THREAD_EXIT;
	}


	void randomMultiThreaded() {
		if (fiftyoneDegreesThreadingGetIsThreadSafe() == false) {
			return;
		}
		runThreads(
			config->getConcurrency(),
			(FIFTYONE_DEGREES_THREAD_ROUTINE)
			CollectionTest::randomMultiThreadedRunThread);
	}

protected:

	/* Config used to create the collection */
	CollectionConfig *config;

	/* Test data to be used */
	CollectionTestData *data;

	/* Pointer to the collection under test */
	fiftyoneDegreesCollection *collection;
};

class CollectionTestMemory : public CollectionTest {
public:
	CollectionTestMemory(
		CollectionConfig *config,
		CollectionTestData *data)
		: CollectionTest(config, data) {
		reader.startByte = reader.current = (byte*)data->data;
		reader.length = (fiftyoneDegreesFileOffset)data->size;
		reader.lastByte = reader.current + reader.length;
	}
	virtual ~CollectionTestMemory() {}
	virtual void SetUp() {
		CollectionTest::SetUp();
		collection = fiftyoneDegreesCollectionCreateFromMemory(
			&reader,
			fiftyoneDegreesCollectionHeaderFromMemory(
				&reader,
				data->elementSize,
				data->isCount));
		EXPECT_NE(collection, nullptr);
	}
	fiftyoneDegreesMemoryReader reader;
};

class CollectionTestMemoryFixed : public CollectionTestMemory {
public:
	CollectionTestMemoryFixed(
		CollectionConfig *config,
		CollectionTestData *data)
		: CollectionTestMemory(config, data) {}
};

class CollectionTestMemoryVariable : public CollectionTestMemory {
public:
	CollectionTestMemoryVariable(
		CollectionConfig *config,
		CollectionTestData *data)
		: CollectionTestMemory(config, data) {}
};

class CollectionTestFile : public CollectionTest {
public:
	CollectionTestFile(
		CollectionConfig *config, 
		CollectionTestData *data)
		: CollectionTest(config, data) {
		fileHandle = new FileHandle(
			"collection-test",
			data->data,
			data->size,
			config->getConcurrency());
	}
	~CollectionTestFile() {
		delete fileHandle;
	}
	virtual void SetUp() {
		CollectionTest::SetUp();

		if (fiftyoneDegreesThreadingGetIsThreadSafe()) {
			// Check that the concurrency level is okay for the count of items in 
			// the data being tested.
			ASSERT_GT(config->getConcurrency(), 0) << "Concurrency must be "
				"greater than 0";
			ASSERT_LE(
				config->getConcurrency(),
				data->count / config->getConcurrency()) <<
				"Number of threads are too high for a successful test.";
		}
		fseek(fileHandle->getFile(), 0, SEEK_SET);
		collection = fiftyoneDegreesCollectionCreateFromFile(
			fileHandle->getFile(),
			fileHandle->getFilePool(),
			config->getConfig(),
			fiftyoneDegreesCollectionHeaderFromFile(
				fileHandle->getFile(),
				data->elementSize, 
				data->isCount),
			readMethod);
		ASSERT_NE(collection, nullptr);
		if (fiftyoneDegreesCollectionGetIsMemoryOnly()) {
			ASSERT_EQ(nullptr, collection->release) <<
				L"Collections were compiled for memory only operation, so "
				"the release method should always be null.";
		}
	}

	fiftyoneDegreesCollectionFileRead readMethod;

	FileHandle *fileHandle;
};

class CollectionTestFileFixed : public CollectionTestFile {
public:
	CollectionTestFileFixed(CollectionConfig *config, CollectionTestData *data)
		: CollectionTestFile(config, data) {
		readMethod = CollectionTestFileFixed::Read;
	}

	static void* Read(
		const fiftyoneDegreesCollectionFile * const file,
		const fiftyoneDegreesCollectionKey * const key,
		fiftyoneDegreesData * const data,
		fiftyoneDegreesException * const exception) {
		return fiftyoneDegreesCollectionReadFileFixed(file, key, data, exception);
	}
};

class CollectionTestFileVariable : public CollectionTestFile {
public:
	CollectionTestFileVariable(CollectionConfig *config, CollectionTestData *data)
		: CollectionTestFile(config, data) {
		readMethod = CollectionTestFileVariable::Read;
	}

	static void* Read(
		const fiftyoneDegreesCollectionFile *file,
		const fiftyoneDegreesCollectionKey * const key,
		fiftyoneDegreesData * const data,
		fiftyoneDegreesException * const exception) {
		stringstream stream;
		char c;
		uint32_t length = 1;

		// Get the handle positioned at the start of the item to be read.
		fiftyoneDegreesFileHandle *handle =
			fiftyoneDegreesCollectionReadFilePosition(
				file,
				key->indexOrOffset.offset,
				exception);
		if (handle == NULL || FIFTYONE_DEGREES_EXCEPTION_FAILED) {
			return NULL;
		}

		// Read characters one at a time until the null terminator for the 
		// string is read.
		while (fread(&c, sizeof(char), 1, handle->file) == 1 && c != '\0') {
			stream << c;
			length++;
		}

		// Ensure sufficient memory is allocated for the item being read.
		if (fiftyoneDegreesDataMalloc(data, length) == NULL) {
			fiftyoneDegreesFileHandleRelease(handle);
			return NULL;
		}

		// Copy from the stream to the data pointer.
		strcpy((char*)data->ptr, stream.str().c_str());
		data->used = length;

		// Release the handle to the pool.
		fiftyoneDegreesFileHandleRelease(handle);

		return data->ptr;
	}
};

class CollectionTestFileVariableLimits : public Base {
public:
	CollectionTestFileVariableLimits() {}
	void SetUp() {
		Base::SetUp();
	}
	void TearDown() {
		Base::TearDown();
	}
};

static uint32_t getIntArraySize(
	const void *initial,
    fiftyoneDegreesException * const exception) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(exception);
#	endif
	return (uint32_t)(sizeof(int32_t) * ((*(int32_t*)initial) + 1));
}

/**
* Reads a string from the source file at the offset within the string
* structure.
*/
void* intArrayRead(
	const fiftyoneDegreesCollectionFile * const file,
	const fiftyoneDegreesCollectionKey * const key,
	fiftyoneDegreesData * const data,
	fiftyoneDegreesException * const exception) {
	uint32_t length;
	return fiftyoneDegreesCollectionReadFileVariable(
		file,
		data,
		key,
		&length,
		exception);
}


/**
 * Check that a variable size collection item which is no bigger than its
 * header (i.e. the extra allocation is zero) is successfully loaded in the
 * collection. This checks that the collection can be created successfully and
 * that items returned correctly.
 *
 * The structure used is an integer array which has a count (the header),
 * then the integers (the additional data). By loading an element which has
 * zero as the count, the additional data to allocate is nothing, so we are
 * checking that this is handled and the correct pointer returned.
 */
TEST_F(CollectionTestFileVariableLimits, OnlyElementHeader) {
	FIFTYONE_DEGREES_EXCEPTION_CREATE
	uint32_t size, offsetIndex;
	byte *data, *current;
	uint32_t *offsets;
	FileHandle *fileHandle;
	fiftyoneDegreesCollection *collection;
	// Data to test.
	uint32_t count = 9;
	const uint32_t values[]{
		1, 10, /* 1 value */
		4, 2, 3, 2, 1, /* 4 values */
		2, 3, 2, /* 2 values */
		0, /* 0 values (just the element header, this is what we are testing) */
		1, 321, /* 1 value */
		5, 3, 4, 5, 43, 2, /* etc. */
		3, 34, 3, 654,
		5, 5, 2435, 432, 43, 45,
		2, 32, 54 };
	// This configuration ensures that not everything is loaded into memory.
	fiftyoneDegreesCollectionConfig config = { true, 5, 1 };

	// Now set up the binary file containing the data structure.
	size = sizeof(values);
	data = new byte[size + sizeof(uint32_t)];
	offsets = new uint32_t[count];
	// Write the total size of the collection.
	*(uint32_t*)data = size;
	current = data + sizeof(uint32_t);
	offsetIndex = 0;
	for (uint32_t i = 0; i < size / sizeof(uint32_t); i++) {
		uint32_t valCount = values[i];
		// Write the header for this element.
		*(uint32_t*)current = valCount;
		// Store the offset so we can fetch it from the collection later.
		offsets[offsetIndex++] = (uint32_t)(current - data - sizeof(uint32_t));
		// Move over the data we just wrote.
		current += sizeof(uint32_t);
		// Write the rest of the element.
		for (uint32_t j = 0; j < valCount; j++) {
			i++;
			*(uint32_t*)current = values[i];
			current += sizeof(uint32_t);
		}
	}
	// Write the data to a file.
	fileHandle = new FileHandle(
		"collection-test",
		data,
		size + sizeof(uint32_t),
		1);
	fseek(fileHandle->getFile(), 0, SEEK_SET);

	// Create the collection.
	collection =
		fiftyoneDegreesCollectionCreateFromFile(
		fileHandle->getFile(),
		fileHandle->getFilePool(),
		&config,
		fiftyoneDegreesCollectionHeaderFromFile(
			fileHandle->getFile(),
			0,
			false),
			intArrayRead);
	ASSERT_NE(collection, nullptr) <<
		L"The collection was not created correctly";

	// Fetch values from the collection.
	fiftyoneDegreesCollectionItem item;
	fiftyoneDegreesDataReset(&item.data);
	uint32_t *value;
	const fiftyoneDegreesCollectionKeyType keyType {
		FIFTYONE_DEGREES_COLLECTION_ENTRY_TYPE_CUSTOM,
		sizeof(int32_t),
		getIntArraySize,
	};
	for (uint32_t i = 0; i < count; i++) {
		const fiftyoneDegreesCollectionKey key {
			offsets[i],
			&keyType,
		};
		value = (uint32_t*)collection->get(
			collection,
			&key,
			&item,
			exception);
		FIFTYONE_DEGREES_EXCEPTION_THROW
		for (uint32_t j = 0; j < value[0]; j++) {
			ASSERT_EQ(
				values[(offsets[i] / sizeof(uint32_t)) + 1 + j],
				value[1 + j]) <<
				L"The value returned was not correct.";
		}
		if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {
			FIFTYONE_DEGREES_COLLECTION_RELEASE(collection, &item);
		}
	}

	// Free everything.
	collection->freeCollection(collection);
	delete fileHandle;
	delete[] offsets;
	delete[] data;
}

#define COLLECTION_TEST(s, w, e, o, c) \
class CollectionTest##s##w##e##o : public CollectionTest##s##w { \
public: \
	CollectionTest##s##w##e##o() : CollectionTest##s##w( \
		new CollectionConfig(&o), \
		new CollectionTestData##w##e(c)) {} \
}; \
TEST_F(CollectionTest##s##w##e##o, Verify) { verify(); } \
TEST_F(CollectionTest##s##w##e##o, OutOfRange) { outOfRange(); } \
TEST_F(CollectionTest##s##w##e##o, Random) { random(); } \
TEST_F(CollectionTest##s##w##e##o, RandomOutOfRange) { random(); outOfRange(); } \
TEST_F(CollectionTest##s##w##e##o, RandomMultiThreaded) { randomMultiThreaded(); } \
TEST_F(CollectionTest##s##w##e##o, List) { list(0.1); } \
TEST_F(CollectionTest##s##w##e##o, BinarySearch) { binarySearch(); } \
TEST_F(CollectionTest##s##w##e##o, BinarySearchNotFound) { binarySearch_notFound(); }

/* Configs to test. */
#define COLLECTION_TEST_THREADS 4
fiftyoneDegreesCollectionConfig MaxMemConf = {
	true, 0, COLLECTION_TEST_THREADS
};
fiftyoneDegreesCollectionConfig ExactMemConf = {
	true, 0, COLLECTION_TEST_THREADS
};
fiftyoneDegreesCollectionConfig CacheConf = {
	false, (uint32_t)TEST_STRINGS_COUNT, COLLECTION_TEST_THREADS
};
fiftyoneDegreesCollectionConfig StreamConf = {
	false, 0, COLLECTION_TEST_THREADS
};
fiftyoneDegreesCollectionConfig MixedCacheConf = {
	true,
	((uint32_t)TEST_STRINGS_COUNT / 2) / 2,
	COLLECTION_TEST_THREADS
};
fiftyoneDegreesCollectionConfig MixedStreamConf = {
	true, 0, COLLECTION_TEST_THREADS
};
fiftyoneDegreesCollectionConfig MixedStreamCacheConf = {
	true,
	((uint32_t)TEST_STRINGS_COUNT / 3),
	COLLECTION_TEST_THREADS
};

COLLECTION_TEST(Memory, Fixed, Count, MaxMemConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(Memory, Fixed, Size, MaxMemConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(Memory, Variable, Size, MaxMemConf, TEST_STRINGS_COUNT)

COLLECTION_TEST(Memory, Fixed, Count, ExactMemConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(Memory, Fixed, Size, ExactMemConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(Memory, Variable, Size, ExactMemConf, TEST_STRINGS_COUNT)

COLLECTION_TEST(File, Fixed, Count, StreamConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Fixed, Size, StreamConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Variable, Size, StreamConf, TEST_STRINGS_COUNT)

COLLECTION_TEST(File, Fixed, Count, CacheConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Fixed, Size, CacheConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Variable, Size, CacheConf, TEST_STRINGS_COUNT)

COLLECTION_TEST(File, Fixed, Count, MixedCacheConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Fixed, Size, MixedCacheConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Variable, Size, MixedCacheConf, TEST_STRINGS_COUNT)

COLLECTION_TEST(File, Fixed, Count, MixedStreamConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Fixed, Size, MixedStreamConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Variable, Size, MixedStreamConf, TEST_STRINGS_COUNT)

COLLECTION_TEST(File, Fixed, Count, MixedStreamCacheConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Fixed, Size, MixedStreamCacheConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Variable, Size, MixedStreamCacheConf, TEST_STRINGS_COUNT)

COLLECTION_TEST(File, Fixed, Count, MaxMemConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Fixed, Size, MaxMemConf, TEST_STRINGS_COUNT)
COLLECTION_TEST(File, Variable, Size, MaxMemConf, TEST_STRINGS_COUNT)
