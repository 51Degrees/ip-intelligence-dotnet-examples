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
#include "../resource.h"
#include "Base.hpp"

class ResourceManager : public Base
{
private:

	/**
	* Set the resource to true to indicate this method has been called by the
	* resource manager.
	*/
	static void freeResource(void *resourcePtr) {
		bool *resource = (bool*)resourcePtr;
		*resource = true;
	}

protected:

	/**
	 * Initialise the resource manager with a boolean resource, and set the
	 * resource to false to indicate the free method has not yet been called.
	 */
	void SetUp() {
		Base::SetUp();
		resource = false;
		fiftyoneDegreesResourceManagerInit(
			&manager,
			&resource,
			&resourceHandle,
			ResourceManager::freeResource);
	}

	/**
	 * Releases the resource manager structure if one was created. Then calls
	 * the base teardown method to check for memory leaks and compare expected
	 * and actual memory allocations.
	 */
	void TearDown() {
		disposeManager();
		ASSERT_TRUE(resource) <<
			"The resource was not closed.";
		Base::TearDown();
	}

	/**
	 * Dispose of the resource manager and check that the resource has been 
	 * freed.
	 */
	void disposeManager() {
		if (disposed == false) {
			disposed = true;
			fiftyoneDegreesResourceManagerFree(&manager);
		}
	}

	fiftyoneDegreesResourceManager manager;

	bool disposed = false;

	// The resource to use with the test.
	bool resource;

	// The handle assigned to the resource.
	fiftyoneDegreesResourceHandle *resourceHandle;
};

/**
 * Check that a resource manager contains the correct resource when it is
 * initialised.
 */
TEST_F(ResourceManager, Init) {
	ASSERT_EQ((void*)&resource, (void*)manager.active->resource) <<
		"The resource was not set correctly.";
}

/**
 * Check that the IncUse method returns a valid handle to the resource.
 */
TEST_F(ResourceManager, Handle_Inc) {
	fiftyoneDegreesResourceHandle *handle =
		fiftyoneDegreesResourceHandleIncUse(&manager);
	ASSERT_NE((void*)NULL, (void*)handle) <<
		"A null handle was returned.";
	ASSERT_EQ((void*)&resource, (void*)handle->resource) <<
		"The handle does not contain the correct resource.";
	ASSERT_EQ(1, fiftyoneDegreesResourceHandleGetUse(handle)) <<
		"The in use counter was not incremented correctly.";
	ASSERT_EQ(&manager, handle->manager) <<
		"The handle is not linked to the manager.";
	fiftyoneDegreesResourceHandleDecUse(handle);
}

/**
 * Check that the DecUse method decrements the in use counter.
 */
TEST_F(ResourceManager, Handle_IncDec) {
	fiftyoneDegreesResourceHandle *handle =
		fiftyoneDegreesResourceHandleIncUse(&manager);
	ASSERT_EQ(1, fiftyoneDegreesResourceHandleGetUse(handle)) <<
		"The in use counter was not incremented correctly.";
	fiftyoneDegreesResourceHandleDecUse(handle);
	ASSERT_EQ(0, fiftyoneDegreesResourceHandleGetUse(handle)) <<
		"The in use counter was not decremented correctly.";
}

/**
 * Check that the resource replace method correctly replaces the resource, and
 * that all resources are freed.
 */
TEST_F(ResourceManager, ResourceReplace) {
	fiftyoneDegreesResourceHandle *handle = NULL;
	bool newResource = false;
	fiftyoneDegreesResourceReplace(
		&manager,
		(void*)&newResource,
		&handle);
	ASSERT_NE((void*)NULL, (void*)handle) <<
		"The resource was not replaced in the manager.";
	ASSERT_EQ(&newResource, handle->resource) <<
		"The new resource is not correct.";
	disposeManager();
	ASSERT_TRUE(newResource) <<
		"The new resource was not closed.";
}

/**
 * Check that the manager free method leaves the resource open until it is
 * released, and that the resource is freed in the end.
 */
TEST_F(ResourceManager, Free_HandleInUse) {
	fiftyoneDegreesResourceHandle *handle;
	handle = fiftyoneDegreesResourceHandleIncUse(&manager);
	disposeManager();
	ASSERT_EQ(&resource, handle->resource) <<
		"The new resource is not correct.";
	ASSERT_FALSE(resource) <<
		"The old resource was closed prematurely.";
	fiftyoneDegreesResourceHandleDecUse(handle);
	ASSERT_TRUE(resource) <<
		"The new resource was not closed.";
}

/**
 * Check that the resource replace method leaves the old resource open until it
 * is released.
 */
TEST_F(ResourceManager, ResourceReplace_HandleInUse) {
	fiftyoneDegreesResourceHandle *oldHandle;
	fiftyoneDegreesResourceHandle *newHandle;
	bool newResource = false;
	oldHandle = fiftyoneDegreesResourceHandleIncUse(&manager);
	fiftyoneDegreesResourceReplace(&manager, (void*)&newResource, &newHandle);
	ASSERT_NE((void*)NULL, (void*)newHandle) <<
		"The resource was not replaced in the manager.";
	ASSERT_EQ(&newResource, newHandle->resource) <<
		"The new resource is not correct.";
	ASSERT_FALSE(resource) <<
		"The old resource was closed prematurely.";
	fiftyoneDegreesResourceHandleDecUse(oldHandle);
	disposeManager();
	ASSERT_TRUE(newResource) <<
		"The new resource was not closed.";
}

// Number of threads
#define THREAD_COUNT 8
// Number of Inc/Dec per thread
#define NUMBER_OF_UPDATES 100000
// Number of reloads
#define NUMBER_OF_RELOADS 4

/*
 * Run by each thread to increase resource 
 * usage counter
 */
static void runResourceInc(void *state) {
	fiftyoneDegreesResourceManager *manager =
		(fiftyoneDegreesResourceManager *)state;
	for (int i = 0; i < NUMBER_OF_UPDATES; i++) {
		fiftyoneDegreesResourceHandleIncUse(manager);
	}
}

/*
 * Run by each thread to decrease resource 
 * usage counter
 */
static void runResourceDec(void *state) {
	fiftyoneDegreesResourceHandle *handle =
		(fiftyoneDegreesResourceHandle *)state;
	for (int i = 0; i < NUMBER_OF_UPDATES; i++) {
		fiftyoneDegreesResourceHandleDecUse(handle);
	}
}

/*
 * Start threads
 */
static void startThreads(
	FIFTYONE_DEGREES_THREAD *threads, 
	FIFTYONE_DEGREES_THREAD_ROUTINE threadRoutine,
	void *state) {
	for (int i = 0; i < THREAD_COUNT; i++) {
		FIFTYONE_DEGREES_THREAD_CREATE(
			threads[i],
			threadRoutine,
			state);
	}
}

/*
 * Wait and join threads
 */
static void joinThreads(FIFTYONE_DEGREES_THREAD *threads) {
	for (int i = 0; i < THREAD_COUNT; i++) {
		FIFTYONE_DEGREES_THREAD_JOIN(threads[i]);
		FIFTYONE_DEGREES_THREAD_CLOSE(threads[i]);
	}
}

/*
 * Check that the resource usage counter is as expected
 * after being modified in a multi-threading environment
 */
TEST_F(ResourceManager, MultiThreading_HandleIncDec) {
	if (fiftyoneDegreesThreadingGetIsThreadSafe()) {
		FIFTYONE_DEGREES_THREAD threads[THREAD_COUNT];

		// Run an initial Inc/Dec
		fiftyoneDegreesResourceHandle *handle =
			fiftyoneDegreesResourceHandleIncUse(&manager);
		ASSERT_EQ(1, fiftyoneDegreesResourceHandleGetUse(handle)) <<
			"The in use counter was not incremented correctly.";
		fiftyoneDegreesResourceHandleDecUse(handle);
		ASSERT_EQ(0, fiftyoneDegreesResourceHandleGetUse(handle)) <<
			"The in use counter was not decremented correctly.";

		// Start threads which increase the usage counter
		startThreads(
			threads,
			(FIFTYONE_DEGREES_THREAD_ROUTINE)&runResourceInc,
			&manager);
		joinThreads(threads);
		ASSERT_EQ(
			THREAD_COUNT * NUMBER_OF_UPDATES, 
			fiftyoneDegreesResourceHandleGetUse(handle)) <<
			"The in use counter was not increased correctly.";

		// Start threads which decrease the usage counter
		startThreads(
			threads,
			(FIFTYONE_DEGREES_THREAD_ROUTINE)&runResourceDec,
			handle);
		joinThreads(threads);
		ASSERT_EQ(
			0, fiftyoneDegreesResourceHandleGetUse(handle)) <<
			"The in use counter was not decreased correctly.";
	}
}

/*
 * Check that the total usage count of all resouce handles
 * is equal to the total number of increases done by all
 * threads while resource under management is being reloaded.
 * 
 * NOTE: There is no tests for reloads during decrease of usage
 * counts under multi-threads environment. The reason for that 
 * is because the decrease is done directly to the handle and 
 * reload does not impact the handle once it has been replaced.
 */
TEST_F(ResourceManager, MultiThreading_HandleReplace_Inc) {
	if (fiftyoneDegreesThreadingGetIsThreadSafe()) {
		FIFTYONE_DEGREES_THREAD threads[THREAD_COUNT];
		uint32_t i;

		// Run an initual Inc/Dec
		fiftyoneDegreesResourceHandle *handle =
			fiftyoneDegreesResourceHandleIncUse(&manager);
		ASSERT_EQ(1, fiftyoneDegreesResourceHandleGetUse(handle)) <<
			"The in use counter was not incremented correctly.";
		fiftyoneDegreesResourceHandleDecUse(handle);
		ASSERT_EQ(0, fiftyoneDegreesResourceHandleGetUse(handle)) <<
			"The in use counter was not decremented correctly.";

		// Start threads which increase the usage counter
		startThreads(
			threads,
			(FIFTYONE_DEGREES_THREAD_ROUTINE)&runResourceInc,
			&manager);

		// Perform reloads
		fiftyoneDegreesResourceHandle *newHandles[NUMBER_OF_RELOADS];
		bool newResources[NUMBER_OF_RELOADS];
		for (i = 0; i < NUMBER_OF_RELOADS; i++) {
			// Pause between each load for threads
			// to have time updating the new handle
#ifdef _MSC_VER
			Sleep(20); // milliseconds
#else
			usleep(20000); // microseconds
#endif
			newHandles[i] = NULL;
			newResources[i] = false;
			fiftyoneDegreesResourceReplace(
				&manager,
				(void*)&newResources[i],
				&newHandles[i]);
		}

		// Wait for threads to finish
		joinThreads(threads);

		// Check that total usage count of all handles is the same
		// as number of increases performed by all threads
		uint32_t total = 0;
		if (resource == false) {
			total += fiftyoneDegreesResourceHandleGetUse(handle);
		}

		for (i = 0; i < NUMBER_OF_RELOADS; i++) {
			if (newResources[i] == false) {
				total += fiftyoneDegreesResourceHandleGetUse(newHandles[i]);
			}
		}
		ASSERT_EQ(THREAD_COUNT * NUMBER_OF_UPDATES, total) <<
			"The in use counter was not increased correctly.";

		// Decrease usage counter of all handles to 0 to mark
		// as no longer used
		uint32_t usageCount = 0;
		if (resource == false) {
			usageCount = fiftyoneDegreesResourceHandleGetUse(handle);
			for (i = 0; i < usageCount; i++) {
				fiftyoneDegreesResourceHandleDecUse(handle);
			}
		}

		for (i = 0; i < NUMBER_OF_RELOADS; i++) {
			if (newResources[i] == false) {
				usageCount = fiftyoneDegreesResourceHandleGetUse(newHandles[i]);
				for (uint32_t j = 0; j < usageCount; j++) {
					fiftyoneDegreesResourceHandleDecUse(newHandles[i]);
				}
			}
		}
		// Manager has to be disposed here because resources
		// are defined locally.
		disposeManager();
	}
}
