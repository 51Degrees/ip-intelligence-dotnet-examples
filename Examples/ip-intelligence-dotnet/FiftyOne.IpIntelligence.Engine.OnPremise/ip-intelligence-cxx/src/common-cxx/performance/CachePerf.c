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

#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "../cache.h"
#include "../fiftyone.h"

#define PASSES 1000000

 // Number of marks to make when showing progress.
#define PROGRESS_MARKS 40

 // Number of threads to start for performance analysis.
#ifndef FIFTYONE_DEGREES_NO_THREADING
#define THREAD_COUNT 4
#else
#define THREAD_COUNT 1
#endif

static const char *_values[] = {
	"zero",
	"one",
	"two",
	"three",
	"four",
	"five",
	"six",
	"seven",
	"eight",
	"nine",
	"ten",
	"eleven",
	"twelve",
	"thirteen",
	"fourteen",
	"fifteen",
	"sixteen",
	"seventeen",
	"eighteen",
	"nineteen"};

static const int valuesCount = sizeof(_values) / sizeof(char*);

/**
 * Load the string value of an integer from zero to nineteen into the node data.
 * Frees old data if there is any.
 */
#pragma warning(push)
#pragma warning(disable: 4100)
static void load(
	const void *state, 
	Data *data, 
	const void *key, 
	fiftyoneDegreesException *exception) {
	const char *value = _values[*(int32_t*)key];
	size_t size = (strlen(value) + 1) * sizeof(char);
	DataMalloc(data, size);
	strcpy((char*)data->ptr, value);
	data->used = (uint32_t)size;
}
#pragma warning(pop)

// Used to control multi threaded performance.
typedef struct t_performance_state {
	volatile long count;
	int passes;
	int progress;
	int max;
	bool calibration;
	int numberOfThreads;
#ifndef FIFTYONE_DEGREES_NO_THREADING
	fiftyoneDegreesMutex mutex;
#endif
} performanceState;

typedef struct t_thread_performance_state {
	performanceState *state;
	int count;
} performanceThreadState;

fiftyoneDegreesCache *cache;

/**
 * Prints a progress bar
 */
void printLoadBar(performanceState *state) {
	long i;
	const long full = state->count / state->progress;
    const long empty = (state->max - state->count) / state->progress;

	printf("\r\t[");
	for (i = 0; i < full; i++) {
		printf("=");
	}

	for (i = 0; i < empty; i++) {
		printf(" ");
	}
	printf("]");
}

void reportProgress(
	performanceState *perfState,
	fiftyoneDegreesCacheNode *node) {

	// Update the user interface.
	printLoadBar(perfState);

	// If in real caching mode then print the value returned from the cache
	// to prove it's actually doing something!
	if (perfState->calibration == false && node != NULL) {
		const char *value = (const char*)node->data.ptr;
		assert(value != NULL);
		printf(" %s      ", value);
	}
}

static void executePerformanceTest(int key, void *state) {
	FIFTYONE_DEGREES_EXCEPTION_CREATE
	performanceThreadState *threadState = (performanceThreadState*)state;
	fiftyoneDegreesCacheNode *node = NULL;
	if (threadState->state->calibration == false) {
		node = fiftyoneDegreesCacheGet(cache, &key, exception);
		assert(FIFTYONE_DEGREES_EXCEPTION_OKAY);
		assert((int32_t)node->tree.key == key);
		assert(strcmp(
			(const char*)node->data.ptr, 
			(const char*)_values[key]) == 0);
	}
	threadState->count++;
	if (threadState->count == 1000) {
#ifndef FIFTYONE_DEGREES_NO_THREADING
		FIFTYONE_DEGREES_MUTEX_LOCK(&threadState->state->mutex);
#endif
		threadState->state->count += threadState->count;
		threadState->count = 0;
		if (threadState->state->count % threadState->state->progress == 0) {
			reportProgress(threadState->state, node);
		}
#ifndef FIFTYONE_DEGREES_NO_THREADING
		FIFTYONE_DEGREES_MUTEX_UNLOCK(&threadState->state->mutex);
#endif
	}
	if (node != NULL) {
		fiftyoneDegreesCacheRelease(node);
	}
}

void runPerformanceTest(void* state) {
	int i;
	performanceThreadState threadState;
	threadState.state = (performanceState*)state;
	threadState.count = 0;

	// Execute the performance test or calibration.
	for (i = 0; i < threadState.state->passes; i++) {
		executePerformanceTest(rand() % valuesCount, &threadState);
	}

#ifndef FIFTYONE_DEGREES_NO_THREADING
	FIFTYONE_DEGREES_THREAD_EXIT;
#endif
}

/**
 * Execute a performance test. If calibrate is true then no cache fetches are
 * performed.
 */
void performanceTest(performanceState *state) {
#ifndef FIFTYONE_DEGREES_NO_THREADING
	FIFTYONE_DEGREES_THREAD *threads =
		(FIFTYONE_DEGREES_THREAD*)malloc(
			sizeof(FIFTYONE_DEGREES_THREAD) * state->numberOfThreads);
	int thread;
#endif
	state->count = 0;

#ifndef FIFTYONE_DEGREES_NO_THREADING
	// Create the threads.
	for (thread = 0; thread < state->numberOfThreads; thread++) {
		FIFTYONE_DEGREES_THREAD_CREATE(
			threads[thread],
			(FIFTYONE_DEGREES_THREAD_ROUTINE)&runPerformanceTest,
			state);
	}

	// Wait for them to finish.
	for (thread = 0; thread < state->numberOfThreads; thread++) {
		FIFTYONE_DEGREES_THREAD_JOIN(threads[thread]);
	}
#else
	runPerformanceTest(state);
#endif

	// Finally report progress.
	reportProgress(state, NULL);
	printf("\n\n");

#ifndef FIFTYONE_DEGREES_NO_THREADING
	free(threads);
#endif
}

/**
 * Perform the test and return the average time.
 */
double performTest(performanceState *state, char *test) {
#ifdef _MSC_VER
	double start, end;
#else
	struct timespec start, end;
#endif
	fflush(stdout);

	// Set the progress indicator.
	state->progress = (state->max > 0 ? state->max : INT_MAX) / PROGRESS_MARKS;

	// Perform a number of passes of the test.
#ifdef _MSC_VER
	start = GetTickCount();
#else
	clock_gettime(CLOCK_MONOTONIC, &start);
#endif

	printf("%s\n\n", test);
	performanceTest(state);

#ifdef _MSC_VER
	end = GetTickCount();
	return (end - start) / 1000;
#else
	clock_gettime(CLOCK_MONOTONIC, &end);
	return ((end.tv_sec - start.tv_sec) +
		(end.tv_nsec - start.tv_nsec) / 1.0e9);
#endif
}

void printTime(performanceState *state, double totalSec) {
#	define INDENT "    "
#	define TIME_UNITS "ns"

	const double TIME_FACTOR = 1e9;

	printf(INDENT "%ld cache fetches made in %.3fs using %d threads.\n",
		state->count,
		totalSec,
		state->numberOfThreads);

	const double timeUnitsPerFetch = TIME_FACTOR * totalSec / (double)state->count;
	printf(INDENT "Average time per cache fetch: %.2f" TIME_UNITS " (%.2f" TIME_UNITS " per thread)\n",
		timeUnitsPerFetch,
		timeUnitsPerFetch / state->numberOfThreads);

	const double cps = (double)state->count / totalSec;
	printf(INDENT "Average cache fetches per second: %.2f (%.2f per thread)\n\n\n",
		cps,
		cps / (double)state->numberOfThreads);

#	undef INDENT
#	undef TIME_UNITS
}

void outputTime(performanceState *state, double totalSec, const char *outFile) {
	double cps = (double)state->count / totalSec;

	FILE *file = fopen(outFile, "w");
	fprintf(file, "{\n");
	fprintf(file, "  \"CacheFetchesPerSecond\": %.2f,\n", cps);
	fprintf(file, "  \"CacheFetchesPerSecondPerThread\": %.2f\n", cps / (double)state->numberOfThreads);
	fprintf(file, "}");
	fclose(file);
}

/**
 * Performance test.
 */
void performance(int passes, const char* outFile) {
	performanceState state;

#ifndef FIFTYONE_DEGREES_NO_THREADING
	FIFTYONE_DEGREES_MUTEX_CREATE(state.mutex);
#endif

	// Set the state for the calibration.
	state.numberOfThreads = THREAD_COUNT;
	state.passes = passes;
	state.max = passes * state.numberOfThreads;
	state.calibration = true;

	{
		// Run the process without doing any cache fetches to get a
		// calibration time.
		const double calibration = performTest(&state, "Calibration");
		printTime(&state, calibration);
	};

	// Fetch random items from a cache which does not have enough capacity to
	// hold all items at once.
	state.calibration = false;
	{
		cache = fiftyoneDegreesCacheCreate(
			valuesCount - THREAD_COUNT,
			THREAD_COUNT,
			load,
			fiftyoneDegreesCacheHash32,
			_values);
		const double test = performTest(&state, "Cache Too Small");
		fiftyoneDegreesCacheFree(cache);
		printTime(&state, test);
	};
	
	// Fetch random items from a cache which has enough capacity to hold all
	// all the items at once.
	{
		cache = fiftyoneDegreesCacheCreate(
			valuesCount,
			THREAD_COUNT,
			load,
			fiftyoneDegreesCacheHash32,
			_values);
		const double test = performTest(&state, "Cache Just Right");
		fiftyoneDegreesCacheFree(cache);
		printTime(&state, test);
		if (outFile != NULL) {
			outputTime(&state, test, outFile);
		}
	};

	// Fetch random items from a cache which has more than enough capacity to
	// hold all the items at once.
	{
		cache = fiftyoneDegreesCacheCreate(
			valuesCount * 20,
			THREAD_COUNT,
			load,
			fiftyoneDegreesCacheHash32,
			_values);
		const double test = performTest(&state, "Cache Too Big");
		fiftyoneDegreesCacheFree(cache);
		printTime(&state, test);
	};
}

/**
 * The main method used by the command line test routine.
 */
#pragma warning(push)
#pragma warning(disable: 4100)
int main(int argc, char* argv[]) {
	printf("\n");
	printf("\t#############################################################\n");
	printf("\t#                                                           #\n");
	printf("\t#  This program can be used to test the performance of the  #\n");
	printf("\t#            51Degrees cache implementation.                #\n");
	printf("\t#                                                           #\n");
	printf("\t#   The test will fetch random items from a loading cache   #\n");
	printf("\t#  and calculate the number of fetch operations per second. #\n");
	printf("\t#                                                           #\n");
	printf("\t#############################################################\n");

	Malloc = MemoryStandardMalloc;
	MallocAligned = MemoryStandardMallocAligned;
	Free = MemoryStandardFree;
	FreeAligned = MemoryStandardFreeAligned;

	// Run the performance tests.
	char *outFile = NULL;
	if (argc > 1) {
		outFile = argv[1];
	}
	performance(PASSES, outFile);
	
	return 0;
}
#pragma warning(pop)