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
#include "Base.hpp"
#include "TestStrings.hpp"
#include "../Exceptions.hpp"
#include "../cache.h"
#include "../memory.h"

#define TEST_CACHE(c,a,o) \
class CacheTest##c : public CacheTest { \
public: \
	void SetUp() { CacheTest::SetUp(); createCache(a,o); }; \
};

/**
* Unit tests for the cache implementation. These ensure that the cache behaves
* as intended.
*/
class CacheTest : public Base {
public:
	CacheTest() { cache = NULL; }
	virtual void SetUp() { Base::SetUp(); }
	void TearDown() {
		if (cache != NULL) {
			fiftyoneDegreesCacheFree(cache);
			cache = NULL;
		}
		Base::TearDown();
	}
	fiftyoneDegreesCache *cache;
	
#ifdef _MSC_VER
	// This is a mock function, so not all parameters can be used.
#pragma warning (disable: 4100)
#endif
	/**
	 * Load the string value of an integer from zero to nine into the node data.
	 * Frees old data if there is any.
	 */
	static void load(
		const void *state,
		fiftyoneDegreesData *data,
		const void *key, 
		fiftyoneDegreesException *exception) {
		const char **values = (const char**)state;
		ASSERT_EQ(values, TEST_STRINGS) <<
			L"State passed for loading is not the expected array of strings";
		const char *value = *(int*)key < TEST_STRINGS_COUNT ? values[*(int*)key] : "";
		size_t size = (strlen(value) + 1) * sizeof(char);
		if (fiftyoneDegreesDataMalloc(data, size) != nullptr) {
			strcpy((char*)data->ptr, value);
			data->used = (uint32_t)size;
		}
	}
#ifdef _MSC_VER
#pragma warning (default: 4100)
#endif

	static void checkValue(long key, fiftyoneDegreesCacheNode *node) {
		ASSERT_EQ(key, node->tree.key) <<
			"The key in the returned node was incorrect.";
		ASSERT_STREQ(TEST_STRINGS[key], (char*)node->data.ptr) <<
			"The value returned by the cache was incorrect.";
	}

	/**
	 * Create a single threaded cache with the requested size ready to load the
	 * string representations of integers from zero to nine.
	 */
	void createCache(uint32_t capacity, uint16_t concurrency) {
		cache = fiftyoneDegreesCacheCreate(
			capacity,
			concurrency,
			load,
			fiftyoneDegreesCacheHash32,
			TEST_STRINGS);
	}

	/**
	* Check that the cache has be created with the expected parameters.
	*/ \
	void verify(int minCapacity, int concurrency) {
		ASSERT_LE(minCapacity, cache->capacity) <<
			"The cache capacity was not set correctly.";
		ASSERT_EQ(concurrency, cache->concurrency) <<
			"The cache concurrency was not set correctly.";
		ASSERT_EQ((void*)TEST_STRINGS, (void*)cache->loaderState) <<
			"The cache parameters were not set correctly.";
		ASSERT_EQ(&load, (void*)cache->load) <<
			"The data load method was not set correctly.";
		ASSERT_EQ(0, cache->hits) <<
			"The cache hits were not initialised to zero.";
		ASSERT_EQ(0, cache->misses) <<
			"The cache misses were not initialised to zero.";
		EXPECT_LE(cache->concurrency, minCapacity / cache->concurrency) <<
			"Concurrency is too low for a successful test.";
	}
	
	/**
	* Get each value from the cache within a range, check it is correct, and 
	* release the node for possible eviction.
	* @param start the index in the source data to start at
	* @param end the index in the source data to end at
	*/
	void getAndCheck(int start, int end) {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		for (int i = start; i <= end; i++) {
			fiftyoneDegreesCacheNode *node = fiftyoneDegreesCacheGet(
				cache, 
				&i,
				exception);
			FIFTYONE_DEGREES_EXCEPTION_THROW
			const char *value = (const char*)node->data.ptr;
			ASSERT_STREQ(TEST_STRINGS[i], value) <<
				"The incorrect value was returned.";
			ASSERT_EQ(1, node->activeCount) <<
				"The number of references to the node is not correct.";
			ASSERT_EQ(i, node->tree.key) <<
				"The node's key does not match the key used to return it.";
			fiftyoneDegreesCacheRelease(node);
		}
	}

	/**
	* Get each value from the cache, check it is correct, and release the node
	* for possible eviction. Check the cache misses and hits.
	* @param hits the expected hits after the 2nd pass
	* @param misses the expected misses after the 2nd pass
	*/
	void getAndCheckAll(int hits, int misses) {
		// First pass from an empty cache
		getAndCheck(0, TEST_STRINGS_COUNT - 1);
		ASSERT_EQ(0, cache->hits) <<
			"There should not have been any cache hits as no values were repeated.";
		ASSERT_EQ(TEST_STRINGS_COUNT, cache->misses) <<
			"Every fetch should have been a miss as no values were repeated.";

		// Second pass from a cache populated with the last cache->capacity values
		getAndCheck(0, TEST_STRINGS_COUNT - 1);
		ASSERT_EQ(hits, cache->hits) <<
			"All values should have existed in the cache.";
		ASSERT_EQ(misses, cache->misses) <<
			"All values should have existed in the cache.";
	}

	/**
	* Check that a null is returned when there are no nodes available for 
	* eviction when the new value requires a free node to be stored in.
	*/
	void exhaust() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		int count = TEST_STRINGS_COUNT;
		fiftyoneDegreesCacheNode *node;

		// Get all the values from the cache without releasing them. 
		for (int i = 0; i < TEST_STRINGS_COUNT; i++) {
			node = fiftyoneDegreesCacheGet(cache, &i, exception);
			FIFTYONE_DEGREES_EXCEPTION_THROW
			// Don't release the node, we want to check that null is returned
		}

		// Check that null is returned when the cache cannot evict anything to
		// make space for the new value
		node = fiftyoneDegreesCacheGet(cache, &count, exception);
		FIFTYONE_DEGREES_EXCEPTION_THROW
		ASSERT_EQ(NULL, node) <<
			"Get should have returned null, there are no free nodes to load the "
			"new value into.";
	}

	/**
	* Check that the cache correctly evicts items without becoming corrupted or
	* leaking memory.
	*/
	void evict(int count, int secondStart) {
		getAndCheck(0, count - 1);
		ASSERT_EQ(0, cache->hits) <<
			"There should not have been any cache hits as no values were repeated.";
		ASSERT_EQ(count, cache->misses) <<
			"Every fetch should have been a miss as no values were repeated.";

		// Check values are loaded again rather than retrieved
		getAndCheck(secondStart, secondStart + count - 1);
		ASSERT_EQ(0, cache->hits) <<
			"The values being requested should have been evicted due to the size "
			"of the cache.";
		ASSERT_EQ(count * 2, cache->misses) <<
			"The values being requested should have been evicted due to the size "
			"of the cache.";
	}

	/**
	* Performs a set number of cache fetches using a random key, checking that
	* the value returned is correct.
	*/
	void random() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		long key;
		fiftyoneDegreesCacheNode *node;
		for (int i = 0; i < TEST_STRINGS_COUNT; i++) {
			key = rand() % TEST_STRINGS_COUNT;
			node = fiftyoneDegreesCacheGet(cache, &key, exception);
			FIFTYONE_DEGREES_EXCEPTION_THROW
			checkValue(key, node);
			fiftyoneDegreesCacheRelease(node);
		}
	}

	/**
	* Check that a node's reference is safe until it is released. By fetching more
	* unique values than the cache has capacity to store, the oldest values will
	* all be evicted unless they are being referenced.
	*/
	void respectReference() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		long key = 0;

		// Hold a reference to a node
		fiftyoneDegreesCacheNode *storedNode = fiftyoneDegreesCacheGet(
			cache, 
			&key,
			exception);
		FIFTYONE_DEGREES_EXCEPTION_THROW

		// Flush the cache out by fetching more values than the cache has capacity
		for (int i = 0; i < TEST_STRINGS_COUNT; i++) {
			fiftyoneDegreesCacheNode *node = fiftyoneDegreesCacheGet(
				cache, 
				&i,
				exception);
			FIFTYONE_DEGREES_EXCEPTION_THROW
			if (i == key) {
				ASSERT_EQ(2, storedNode->activeCount) <<
					"There are two references to this node.";
			}
			fiftyoneDegreesCacheRelease(node);
		}

		// Check that the node has not been evicted
		ASSERT_EQ(1, storedNode->activeCount) <<
			"Node references have not been tracked correctly.";
		ASSERT_STREQ(TEST_STRINGS[key], (const char*)storedNode->data.ptr) <<
			"The node reference has not been reserved. It was either moved or "
			"evicted from the cache.";
	}
	
	static void* multiThreadRandomRunThread(void* state) {
		((CacheTest*)state)->random();
		FIFTYONE_DEGREES_THREAD_EXIT;
#if defined(__MINGW32__) || defined(__MINGW64__)
		// When compiled with MinGW, this results in a warning due to
		// no return statement presents when it is expected.
		// However, the FIFTYONE_DEGREES_THREAD_EXIT should terminate
		// the thread before this point, so this should never be reached.
		return NULL;
#endif
	}
	
	/**
	* Check that multiple threads can fetch items from the cache safely.
	* NOTE: it is important that 'number of threads' <=
	* 'number of values' / 'number of threads'. This prevents null from being
	* returned by the cache.
	* @param concurrency number of threads to run the test with
	*/

	void multiThreadRandom(uint16_t concurrency) {
		if (fiftyoneDegreesThreadingGetIsThreadSafe() == false) {
			return;
		}
		ASSERT_NE(nullptr, cache);
		EXPECT_LE(concurrency, TEST_STRINGS_COUNT / concurrency) <<
			"Number of threads are too high for a successful test.";
		runThreads(
			concurrency,
			(FIFTYONE_DEGREES_THREAD_ROUTINE)multiThreadRandomRunThread);
	}
};

#define TEST_CACHE_METHODS_BASIC(n,a,o,h,m) \
TEST_CACHE(n, a, o) \
TEST_F(CacheTest##n, Verify) { verify(a, o); } \
TEST_F(CacheTest##n, Random) { random(); } \
TEST_F(CacheTest##n, GetAndCheckAll) { getAndCheckAll(h,m); }

#define TEST_CACHE_METHODS(n,a,o,h,m) \
TEST_CACHE_METHODS_BASIC(n,a,o,h,m) \
TEST_F(CacheTest##n, RespectReference) { respectReference(); } \
TEST_F(CacheTest##n, Exhaust) { exhaust(); }

/**
 * All single threaded tests with concurrency one.
 */
TEST_CACHE_METHODS(OneAll, TEST_STRINGS_COUNT, 1, TEST_STRINGS_COUNT, TEST_STRINGS_COUNT)
TEST_CACHE_METHODS(OneHalf, TEST_STRINGS_COUNT / 2, 1, 0, TEST_STRINGS_COUNT * 2)
TEST_CACHE_METHODS(OneTwo, 2, 1, 0, TEST_STRINGS_COUNT * 2)
TEST_CACHE_METHODS_BASIC(OneOne, 1, 1, 0, TEST_STRINGS_COUNT * 2)

/**
 * Check that a cache is not returned when an attempt is made to create a cache
 * with a concurrency setting which is too high. When the concurrency is high
 * enough that the number of nodes in a single shard is greater than the
 * concurrency, then null should be returned as the cache would not be safe for
 * that many threads.
 */
TEST_CACHE(HighConcurrency, TEST_STRINGS_COUNT, 100);
TEST_F(CacheTestHighConcurrency, FailHighConcurrency) {
ASSERT_EQ((void*)cache, (void*)NULL) <<
	"An unsafe cache was returned. Construction should have failed.";
}
TEST_F(CacheTestOneHalf, Evict) {
	evict(TEST_STRINGS_COUNT / 2, TEST_STRINGS_COUNT / 2);
}

/**
* Multi threaded tests with higher concurrency.
*/
TEST_CACHE_METHODS_BASIC(FourtyEight, TEST_STRINGS_COUNT, 48, TEST_STRINGS_COUNT, TEST_STRINGS_COUNT)
TEST_CACHE_METHODS_BASIC(HalfFourtyEight, TEST_STRINGS_COUNT / 2, 48, 0, TEST_STRINGS_COUNT * 2)
TEST_CACHE_METHODS_BASIC(TwentyFour, TEST_STRINGS_COUNT, 24, TEST_STRINGS_COUNT, TEST_STRINGS_COUNT)
TEST_CACHE_METHODS_BASIC(HalfTwentyFour, TEST_STRINGS_COUNT / 2, 24, 0, TEST_STRINGS_COUNT * 2)
TEST_CACHE_METHODS_BASIC(Twelve, TEST_STRINGS_COUNT, 12, TEST_STRINGS_COUNT, TEST_STRINGS_COUNT)
TEST_CACHE_METHODS_BASIC(HalfTwelve, TEST_STRINGS_COUNT / 2, 12, 0, TEST_STRINGS_COUNT * 2)
TEST_CACHE_METHODS_BASIC(Six, TEST_STRINGS_COUNT, 6, TEST_STRINGS_COUNT, TEST_STRINGS_COUNT)
TEST_CACHE_METHODS_BASIC(HalfSix, TEST_STRINGS_COUNT / 2, 6, 0, TEST_STRINGS_COUNT * 2)
TEST_CACHE_METHODS_BASIC(Four, TEST_STRINGS_COUNT, 4, TEST_STRINGS_COUNT, TEST_STRINGS_COUNT)
TEST_CACHE_METHODS_BASIC(HalfFour, TEST_STRINGS_COUNT / 2, 4, 0, TEST_STRINGS_COUNT * 2)
TEST_CACHE_METHODS_BASIC(Three, TEST_STRINGS_COUNT, 3, TEST_STRINGS_COUNT, TEST_STRINGS_COUNT)
TEST_CACHE_METHODS_BASIC(HalfThree, TEST_STRINGS_COUNT / 2, 3, 0, TEST_STRINGS_COUNT * 2)
TEST_CACHE_METHODS_BASIC(Two, TEST_STRINGS_COUNT, 2, TEST_STRINGS_COUNT, TEST_STRINGS_COUNT)
TEST_CACHE_METHODS_BASIC(HalfTwo, TEST_STRINGS_COUNT / 2, 2, 0, TEST_STRINGS_COUNT * 2)

TEST_F(CacheTestFourtyEight, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestFourtyEight, ThreadSafety4) { multiThreadRandom(4); }
TEST_F(CacheTestFourtyEight, ThreadSafety6) { multiThreadRandom(6); }
TEST_F(CacheTestFourtyEight, ThreadSafety12) { multiThreadRandom(12); }
TEST_F(CacheTestFourtyEight, ThreadSafety24) { multiThreadRandom(24); }
TEST_F(CacheTestFourtyEight, ThreadSafety48) { multiThreadRandom(48); }

TEST_F(CacheTestTwentyFour, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestTwentyFour, ThreadSafety4) { multiThreadRandom(4); }
TEST_F(CacheTestTwentyFour, ThreadSafety6) { multiThreadRandom(6); }
TEST_F(CacheTestTwentyFour, ThreadSafety12) { multiThreadRandom(12); }
TEST_F(CacheTestTwentyFour, ThreadSafety24) { multiThreadRandom(24); }

TEST_F(CacheTestTwelve, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestTwelve, ThreadSafety4) { multiThreadRandom(4); }
TEST_F(CacheTestTwelve, ThreadSafety6) { multiThreadRandom(6); }
TEST_F(CacheTestTwelve, ThreadSafety12) { multiThreadRandom(12); }

TEST_F(CacheTestSix, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestSix, ThreadSafety4) { multiThreadRandom(4); }
TEST_F(CacheTestSix, ThreadSafety6) { multiThreadRandom(6); }

TEST_F(CacheTestFour, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestFour, ThreadSafety3) { multiThreadRandom(3); }
TEST_F(CacheTestFour, ThreadSafety4) { multiThreadRandom(4); }

TEST_F(CacheTestThree, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestThree, ThreadSafety3) { multiThreadRandom(3); }

TEST_F(CacheTestTwo, ThreadSafety2) { multiThreadRandom(2); }

TEST_F(CacheTestHalfFourtyEight, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestHalfFourtyEight, ThreadSafety4) { multiThreadRandom(4); }
TEST_F(CacheTestHalfFourtyEight, ThreadSafety6) { multiThreadRandom(6); }
TEST_F(CacheTestHalfFourtyEight, ThreadSafety12) { multiThreadRandom(12); }
TEST_F(CacheTestHalfFourtyEight, ThreadSafety24) { multiThreadRandom(24); }
TEST_F(CacheTestHalfFourtyEight, ThreadSafety48) { multiThreadRandom(48); }

TEST_F(CacheTestHalfTwentyFour, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestHalfTwentyFour, ThreadSafety4) { multiThreadRandom(4); }
TEST_F(CacheTestHalfTwentyFour, ThreadSafety6) { multiThreadRandom(6); }
TEST_F(CacheTestHalfTwentyFour, ThreadSafety12) { multiThreadRandom(12); }
TEST_F(CacheTestHalfTwentyFour, ThreadSafety24) { multiThreadRandom(24); }

TEST_F(CacheTestHalfTwelve, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestHalfTwelve, ThreadSafety4) { multiThreadRandom(4); }
TEST_F(CacheTestHalfTwelve, ThreadSafety6) { multiThreadRandom(6); }
TEST_F(CacheTestHalfTwelve, ThreadSafety12) { multiThreadRandom(12); }

TEST_F(CacheTestHalfSix, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestHalfSix, ThreadSafety4) { multiThreadRandom(4); }
TEST_F(CacheTestHalfSix, ThreadSafety6) { multiThreadRandom(6); }

TEST_F(CacheTestHalfFour, ThreadSafety2) { multiThreadRandom(2); }
TEST_F(CacheTestHalfFour, ThreadSafety4) { multiThreadRandom(4); }

TEST_F(CacheTestHalfTwo, ThreadSafety2) { multiThreadRandom(2); }