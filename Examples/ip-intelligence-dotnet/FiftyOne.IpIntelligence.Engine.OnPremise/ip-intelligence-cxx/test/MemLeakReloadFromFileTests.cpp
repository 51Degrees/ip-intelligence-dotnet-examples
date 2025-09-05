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

#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif

#ifdef _DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include "../examples/Base/ExampleBase.h"
#include "../src/common-cxx/textfile.h"
#include "../src/ipi.h"
#include "../src/fiftyone.h"
#include "ExampleIpIntelligenceTests.hpp"

#define THREAD_COUNT 4

/*
 * State containing the states for all threads running in the example.
 */
typedef struct shared_state_t {
	ResourceManager *manager; /**< Pointer to the manager containing the data
							  set */
	const char *ipAddressFilePath; /**< Path to the IP addresses to process */
	volatile long threadsFinished; /**< Number of threads that have finished
								   their processing */
	THREAD threads[THREAD_COUNT]; /**< Pointers to the running threads */
} sharedState;

/*
 * State for a single thread carrying out processing.
 */
typedef struct thread_state_t {
	ResourceManager *manager; /**< Pointer to the manager containing the data
							  set */
} threadState;

/*
 * Runs the performance test for the IP address provided. Called from the text
 * file iterator.
 * @param ipAddress to be used for the test
 * @param state instance of performanceThreadState
 */
static void executeTest(const char *ipAddress, void *state) {
	threadState *thread = (threadState*)state;
	ResultsIpi *results = ResultsIpiCreate(
		thread->manager);
	EvidenceKeyValuePairArray *evidence = EvidenceCreate(1);
	EvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_QUERY,
		"client-ip-51d",
		ipAddress);
	EXCEPTION_CREATE;
	ResultsIpiFromEvidence(results, evidence, exception);
	EXCEPTION_THROW;
	EvidenceFree(evidence);
	ResultsIpiFree(results);
}

static void runRequestsSingle(sharedState *state) {
	char ipAddress[500] = "";
	sharedState *shared = (sharedState*)state;
	threadState thread;
	thread.manager = shared->manager;
	TextFileIterateWithLimit(
		shared->ipAddressFilePath,
		ipAddress,
		sizeof(ipAddress),
		300,
		&thread,
		executeTest);
}

static void runRequestsMulti(void *state) {
	sharedState *shared = (sharedState*)state;
	runRequestsSingle(shared);
	INTERLOCK_INC(&shared->threadsFinished);
}

/*
 * Starts threads that run IP intelligence. Must be done after the dataset
 * has been initialized.
 */
static void startThreads(sharedState *state) {
	int thread;
	for (thread = 0; thread < THREAD_COUNT; thread++) {
		THREAD_CREATE(
			state->threads[thread],
			(THREAD_ROUTINE)&runRequestsMulti,
			state);
	}
}

/*
 * Joins the threads and frees the memory occupied by the threads.
 */
static void joinThreads(sharedState *state) {
	int thread;
	for (thread = 0; thread < THREAD_COUNT; thread++) {
		THREAD_JOIN(state->threads[thread]);
		THREAD_CLOSE(state->threads[thread]);
	}
}

/*
 * Reload resource manager
 * @param manager
 * @reader memory data to read from. NULL if read from file.
 * @fromFile indicates if reload from original file
 * otherwise reload from memory
 * @exception object to be used when an exception occurs
 */
static StatusCode reload(
	ResourceManager *manager,
	MemoryReader *reader,
	bool fromFile,
	Exception *exception) {
	StatusCode status;
	if (fromFile) {
		status = IpiReloadManagerFromOriginalFile(
			manager,
			exception);
	}
	else {
		status = IpiReloadManagerFromMemory(
			manager,
			reader->startByte,
			reader->length,
			exception);
	}
	EXCEPTION_THROW;
	return status;
}

static void run(
	ResourceManager *manager,
	const char *ipAddressFilePath,
	MemoryReader *reader,
	bool fromFile) {
	StatusCode status;
	int numberOfReloads = 0;
	int numberOfReloadFails = 0;
	sharedState state;
	state.manager = manager;
	state.ipAddressFilePath = ipAddressFilePath;
	state.threadsFinished = 0;
	EXCEPTION_CREATE;

	if (ThreadingGetIsThreadSafe()) {
		startThreads(&state);
		while (state.threadsFinished < THREAD_COUNT) {
			status = reload(manager, reader, fromFile, exception);
			if (status == SUCCESS) {
				numberOfReloads++;
			}
			else {
				numberOfReloadFails++;
			}
#ifdef _MSC_VER
			Sleep(50); // milliseconds
#else
			usleep(500000); // microseconds
#endif
		}
		joinThreads(&state);
	}
	else {
		runRequestsSingle(&state);
		status = reload(manager, reader, fromFile, exception);
		if (status == SUCCESS) {
			numberOfReloads++;
		}
		else {
			numberOfReloadFails++;
		}
		runRequestsSingle(&state);
	}
}

void memReloadRun(
	const char *dataFilePath,
	const char *ipAddressFilePath,
	const char *requiredProperties,
	ConfigIpi config,
	bool fromFile) {

	// Set the required properties to the string provided in the arguments.
	PropertiesRequired reqProps = PropertiesDefault;

	// Set the required properties for hashing each test thread.
	reqProps.string = requiredProperties;

	// Set concurrency to ensure sufficient shared resources available.
	config.graph.concurrency =
		config.graphs.concurrency =
		config.components.concurrency =
		config.properties.concurrency =
		config.profiles.concurrency =
		config.profileOffsets.concurrency =
		config.values.concurrency =
		config.strings.concurrency = THREAD_COUNT;

	ResourceManager manager;
	EXCEPTION_CREATE;
	MemoryReader reader;
	reader.startByte = nullptr;
	StatusCode status;
	if (fromFile) {
		// Initialize manager from file
		status = IpiInitManagerFromFile(
			&manager,
			&config,
			&reqProps,
			dataFilePath,
			exception);
	}
	else {
		// Read the data file into memory for the initialise and reload 
		// operations.
		status = FileReadToByteArray(dataFilePath, &reader);
		if (status != SUCCESS) {
			return;
		}
		// Initialize manager from memory
		status = IpiInitManagerFromMemory(
			&manager,
			&config,
			&reqProps,
			reader.startByte,
			reader.length,
			exception);
	}
	EXCEPTION_THROW;

	// Free the memory used for the required properties if allocated.
	if (requiredProperties != reqProps.string) {
		free((void*)reqProps.string);
	}

	if (status == SUCCESS) {

		// Run the performance tests.
		run(&manager, ipAddressFilePath, fromFile ? NULL : &reader, fromFile);
		
		// Free the memory used by the data set.
		ResourceManagerFree(&manager);
	}

	// Free the memory for the test.
	if (!fromFile && reader.startByte != nullptr) {
		Free(reader.startByte);
	}
}

/*
 * Reload template class test
 */
#define MEM_LEAK_TEST_CLASS(t,f) \
class MemLeakTestReloadFrom##t : public ExampleIpIntelligenceTest {	\
public:	\
	void run(fiftyoneDegreesConfigIpi config) {	\
		MemoryTrackingReset();	\
		Malloc = MemoryTrackingMalloc;	\
		MallocAligned = MemoryTrackingMallocAligned;	\
		Free = MemoryTrackingFree;	\
		FreeAligned = MemoryTrackingFreeAligned;	\
\
		memReloadRun(	\
			dataFilePath.c_str(),	\
			ipAddressFilePath.c_str(),	\
			requiredProperties,	\
			config,	\
			f);	\
\
		EXPECT_EQ(0, MemoryTrackingGetAllocated()) <<	\
			"There is memory leak. All allocated memory should "	\
			"be freed at the end of this test.";	\
\
		Malloc = MemoryStandardMalloc;	\
		MallocAligned = MemoryStandardMallocAligned;	\
		Free = MemoryStandardFree;	\
		FreeAligned = MemoryStandardFreeAligned;	\
		MemoryTrackingReset();	\
	}	\
};

/*
 * Tests for memory leak with different configurations
 */
#define MEM_TEST(c,t) \
TEST_F(c, LowMemory) { \
	if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) { \
		run(fiftyoneDegrees##t##LowMemoryConfig); \
	} \
} \
TEST_F(c, InMemory) { \
	run(fiftyoneDegrees##t##InMemoryConfig); \
}

/* Create test classes */
MEM_LEAK_TEST_CLASS(File,true)
MEM_LEAK_TEST_CLASS(Memory,false)
/* Create tests */
MEM_TEST(MemLeakTestReloadFromFile,Ipi)
MEM_TEST(MemLeakTestReloadFromMemory,Ipi)