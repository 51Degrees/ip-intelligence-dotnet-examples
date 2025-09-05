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
#include <stdio.h>
#include <sys/stat.h>

#include "../exceptions.h"
#include "../pool.h"

// Note: fiftyone.h short names can't be used as conflicts with Pool class.

#ifndef _MSC_VER
#define _rmdir rmdir
#endif

typedef struct testResource_t {
	bool isSet;
} testResource;

typedef struct testResourceCounter_t {
	testResource* resources;
	int count;
} testResourceCounter;

 /**
  * File test class used to test the functionality of file.c.
  */
class Pool : public Base {
public:
	Pool() : Base() {}

protected:
	fiftyoneDegreesPool pool;
	static const uint16_t maxConcurrency = 4;
	testResource resources[Pool::maxConcurrency];

	/**
	 * Calls the base setup method to enable memory leak checking and memory
	 * allocation checking.
	 */
	void SetUp() {
		for (int i = 0; i < maxConcurrency; i++) {
			resources[i].isSet = false;
		}
		Base::SetUp();
	}

	/**
	 * Calls the base teardown method to check for memory leaks and compare
	 * expected and actual memory allocations.
	 */
	void TearDown() {
		fiftyoneDegreesPoolFree(&pool);
		Base::TearDown();
	}

	bool IsResource(void* resource) {
		for (int i = 0; i < maxConcurrency; i++) {
			if (&resources[i] == resource) {
				return true;
			}
		}
		return false;
	}
};

#ifdef _MSC_VER
#pragma warning (disable: 4100)
#endif

void* createResource(
	fiftyoneDegreesPool* pool,
	void* state,
	fiftyoneDegreesException* exception) {
	testResourceCounter* counter = (testResourceCounter*)state;
	counter->resources[counter->count].isSet = true;
	counter->count++;
	return &counter->resources[counter->count - 1];
}

void freeResource(
	fiftyoneDegreesPool* pool,
	void* state) {
	testResource* resource = (testResource*)state;
	resource->isSet = false;
}

#ifdef _MSC_VER
#pragma warning (default: 4100)
#endif


/**
 * Check that a pool can be initialized and released without error.
 */
TEST_F(Pool, PoolInit) {
	// Arrange
	void* result;
	testResourceCounter counter;
	counter.count = 0;
	counter.resources = resources;
	FIFTYONE_DEGREES_EXCEPTION_CREATE;

	// Act
	result = fiftyoneDegreesPoolInit(
		&pool,
		maxConcurrency,
		&counter,
		createResource,
		freeResource,
		exception);

	// Assert
	EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	ASSERT_EQ(result, &pool);
}

/**
 * Check that initializing the pool calls the createResource
 * method for all items.
 */
TEST_F(Pool, PoolResourcesCreated) {
	// Arrange
	testResourceCounter counter;
	counter.count = 0;
	counter.resources = resources;
	FIFTYONE_DEGREES_EXCEPTION_CREATE;

	// Act
	fiftyoneDegreesPoolInit(
		&pool,
		maxConcurrency,
		&counter,
		createResource,
		freeResource,
		exception);

	// Assert
	for (int i = 0; i < maxConcurrency; i++) {
		ASSERT_TRUE(resources[i].isSet);
	}
	EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
}

/**
 * Check that the freeResource method is called on
 * all resources when freeing the pool.
 */
TEST_F(Pool, PoolResourcesFreed) {
	// Arrange
	testResourceCounter counter;
	counter.count = 0;
	counter.resources = resources;
	FIFTYONE_DEGREES_EXCEPTION_CREATE;
    fiftyoneDegreesPoolInit(
		&pool,
		maxConcurrency,
		&counter,
		createResource,
		freeResource,
		exception);

	// Act
	fiftyoneDegreesPoolFree(&pool);

	// Assert
	for (int i = 0; i < maxConcurrency; i++) {
		ASSERT_FALSE(resources[i].isSet);
	}
	EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
}

/**
 * Check that the resource created by the createResource method
 * is returned by the PoolGet method.
 */
TEST_F(Pool, PoolGet) {
	// Arrange
	testResourceCounter counter;
	counter.count = 0;
	counter.resources = resources;
	FIFTYONE_DEGREES_EXCEPTION_CREATE;
    fiftyoneDegreesPoolInit(
		&pool,
		1,
		&counter,
		createResource,
		freeResource,
		exception);

	// Act
	fiftyoneDegreesPoolItem *item = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);

	// Assert
	EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	ASSERT_EQ((void*)&resources[0], item->resource);

	// Cleanup
	fiftyoneDegreesPoolItemRelease(item);
}

/**
 * Check that the resources created by the createResource method
 * are returned by the PoolGet method, and that the same one is
 * not returned twice.
 */
TEST_F(Pool, PoolGetMultiple) {
	// Arrange
	testResourceCounter counter;
	counter.count = 0;
	counter.resources = resources;
	FIFTYONE_DEGREES_EXCEPTION_CREATE;
	fiftyoneDegreesPoolInit(
		&pool,
		2,
		&counter,
		createResource,
		freeResource,
		exception);

	// Act
	fiftyoneDegreesPoolItem* item = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);
	fiftyoneDegreesPoolItem* item2 = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);

	// Assert
	EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	ASSERT_TRUE(IsResource(item->resource));
	ASSERT_TRUE(IsResource(item2->resource));
	ASSERT_NE(item->resource, item2->resource);

	// Cleanup
	fiftyoneDegreesPoolItemRelease(item);
	fiftyoneDegreesPoolItemRelease(item2);
}

/**
 * Check that when a pool of one resource has been exhausted,
 * null is returned and the correct exception is set.
 */
TEST_F(Pool, PoolGetInsufficientHandles) {
	// Arrange
	testResourceCounter counter;
	counter.count = 0;
	counter.resources = resources;
	FIFTYONE_DEGREES_EXCEPTION_CREATE;
	fiftyoneDegreesPoolInit(
		&pool,
		1,
		&counter,
		createResource,
		freeResource,
		exception);

	// Act
	fiftyoneDegreesPoolItem* item = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);
	ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	fiftyoneDegreesPoolItem* item2 = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);

	// Assert
	ASSERT_EQ(NULL, item2);
	ASSERT_FALSE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_HANDLES));

	// Cleanup
	fiftyoneDegreesPoolItemRelease(item);
}

/**
 * Check that when a pool of multiple resources has been exhausted,
 * null is returned and the correct exception is set.
 */
TEST_F(Pool, PoolGetInsufficientHandlesMultiple) {
	// Arrange
	testResourceCounter counter;
	counter.count = 0;
	counter.resources = resources;
	FIFTYONE_DEGREES_EXCEPTION_CREATE;
	fiftyoneDegreesPoolInit(
		&pool,
		2,
		&counter,
		createResource,
		freeResource,
		exception);

	// Act
	fiftyoneDegreesPoolItem* item = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);
	ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	fiftyoneDegreesPoolItem* item2 = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);
	ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	fiftyoneDegreesPoolItem* item3 = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);

	// Assert
	ASSERT_EQ(NULL, item3);
	ASSERT_FALSE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_HANDLES));

	// Cleanup
	fiftyoneDegreesPoolItemRelease(item);
	fiftyoneDegreesPoolItemRelease(item2);
}

/**
 * Check that when a pool of resources has been exhausted,
 * then handles returned to it, resources are handed out again.
 */
TEST_F(Pool, PoolGetHandlesReturned) {
	// Arrange
	testResourceCounter counter;
	counter.count = 0;
	counter.resources = resources;
	FIFTYONE_DEGREES_EXCEPTION_CREATE;
	fiftyoneDegreesPoolInit(
		&pool,
		1,
		&counter,
		createResource,
		freeResource,
		exception);

	// Act
	fiftyoneDegreesPoolItem* item = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);
	ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	fiftyoneDegreesPoolItem* item2 = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);
	ASSERT_EQ(NULL, item2);
	ASSERT_FALSE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_HANDLES));
	FIFTYONE_DEGREES_EXCEPTION_CLEAR;
	fiftyoneDegreesPoolItemRelease(item);
	item2 = fiftyoneDegreesPoolItemGet(
		&pool,
		exception);

	// Assert
	ASSERT_NE((void*)NULL, (void*)item2);
	ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
	ASSERT_EQ(item->resource, item2->resource);

	// Cleanup
	fiftyoneDegreesPoolItemRelease(item);
	fiftyoneDegreesPoolItemRelease(item2);
}
