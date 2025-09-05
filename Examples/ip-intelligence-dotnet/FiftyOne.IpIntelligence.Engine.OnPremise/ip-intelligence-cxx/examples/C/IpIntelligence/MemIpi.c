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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include "../../Base/ExampleBase.h"
#include "../../../src/ipi.h"
#include "../../../src/fiftyone.h"
#include "../../../src/common-cxx/textfile.h"

// Number of marks to make when showing process.
#define PROGRESS_MARKS 40

// Number of threads to start for performance analysis.
#define THREAD_COUNT 4

static const char* dataDir = "ip-intelligence-data";

static const char* dataFileName = "51Degrees-LiteV41.ipi";

static const char* ipAddressFileName = "evidence.yml";

static char valueBuffer[1024] = "";

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

#define CONFIG fiftyoneDegreesIpiInMemoryConfig
// #define CONFIG fiftyoneDegreesIpiHighPerformanceConfig
// #define CONFIG fiftyoneDegreesIpiLowMemoryConfig
// #define CONFIG fiftyoneDegreesIpiBalancedConfig
// #define CONFIG fiftyoneDegreesIpiBalancedTempConfig

/**
 * Shared test state. All members are immutable except runningThreads
 * which is updated via an interlocked compare and exchange.
 */
typedef struct t_memory_state {
	int ipAddressesCount; // Total number of IP Addresses
	int progress; // Number of IP Addresses to process for each = marker
	const char* evidenceFilePath; // Filename for the IP Address file
	int numberOfThreads; // Number of parallel threads
	fiftyoneDegreesResourceManager* manager; // Manager resource for detection
	volatile long runningThreads; // Number of active running threads
	FIFTYONE_DEGREES_THREAD threads[THREAD_COUNT]; // Running threads
} memoryState;

/**
 * Individual thread state where all members are exclusively accessed by a
 * single test thread.
 */
typedef struct t_thread_memory_state {
	memoryState* main; // Reference to the main threads shared state
	long count; // Number of IP Addresses the thread has processed
	bool reportProgress; // True if this thread should report progress
	fiftyoneDegreesResultsIpi* results; // The results used by the thread
} memoryThreadState;

/**
 * Prints the progress bar based on the state provided.
 * @param state information about the overall performance test
 */
void printLoadBar(memoryThreadState* state) {
	int i;
	long full = state->count / state->main->progress;
	long empty = (state->main->ipAddressesCount - state->count) /
		state->main->progress;
	printf("\r\t[");
	for (i = 0; i < full; i++) {
		printf("=");
	}

	for (i = 0; i < empty; i++) {
		printf(" ");
	}
	printf("]");
}

/**
 * The NetworkId can get very long and not suitable
 * to be displayed as full in a console interface.
 * Thus, shorten with '...' at the end if it gets
 * too long.
 * @param networkName to be printed
*/
static void printShortenNetworkName(const char * const networkName) {
	// Buffer to hold the printed network ID. Additional 4
	// bytes to hold the '...' and the null terminator.
	char buffer[54] = "";
	// Triple dots to be attached
	const char tripleDots[] = "...";
	// Max length to display the network ID string
	const size_t maxLength = 50;
	if (strlen(networkName) > maxLength) {
		memcpy(buffer, networkName, maxLength);
		memcpy(buffer + maxLength, tripleDots, sizeof(tripleDots));
		buffer[53] = '\0';
	}
	printf("%s", buffer);
}

/**
 * Reports progress using the required property index specified.
 * @param state of the performance test
 */
void reportProgress(memoryThreadState* state) {
	EXCEPTION_CREATE;
	char networkName[1024] = "";

	// Update the user interface.
	printLoadBar(state);

	// If in real detection mode then print the id of the network profile found
	// to prove it's actually doing something!
	if (state->results != NULL) {
		printf(" ");
		ResultsIpiGetValuesString(
			state->results,
			"Name",
			networkName,
			sizeof(networkName),
			", ",
			exception);
		EXCEPTION_THROW;
		printShortenNetworkName(networkName);
	}
}

/**
 * Runs the test for the IP Address provided. Called from the text file
 * iterator.
 * @param ipAddress to be used for the test
 * @param state instance of performanceThreadState
 */
static void executeTest(const char* ipAddress, void* state) {
	if (!ipAddress || !strlen(ipAddress)) {
		return;
	}

	memoryThreadState * const threadState = (memoryThreadState*)state;
	EXCEPTION_CREATE;

	// If not calibrating the test environment perform IP range search.
	ResultsIpiFromIpAddressString(
		threadState->results,
		ipAddress,
		strlen(ipAddress),
		exception);
	EXCEPTION_THROW;

	// Increase the count for this performance thread and update the user
	// interface if a progress mark should be written.
	threadState->count++;
	if (threadState->reportProgress == true &&
		threadState->count % threadState->main->progress == 0) {
		reportProgress(threadState);
	}
}

/**
 * A single threaded memory test. Many of these will run in parallel to
 * ensure the single managed resource is being used.
 * @param mainState state information about the main test
 */
static void runMemoryThread(void* mainState) {
	char ipAddress[500] = "";
	memoryThreadState threadState;
	threadState.main = (memoryState*)mainState;

	// Ensure that only on thread reports progress. Avoids keeping a running
	// total and synchronising performance test threads.
	long initialRunning = threadState.main->runningThreads;
	if (ThreadingGetIsThreadSafe()) {
		threadState.reportProgress = INTERLOCK_EXCHANGE(
			threadState.main->runningThreads,
			initialRunning + 1,
			initialRunning) == threadState.main->numberOfThreads - 1;
	}
	else {
		threadState.reportProgress = 1;
	}
	threadState.count = 0;

	// Create an instance of results to access the returned values.
	threadState.results = ResultsIpiCreate(
		threadState.main->manager);

	// Execute the IP intelligence test.
	fiftyoneDegreesEvidenceFileIterate(
		threadState.main->evidenceFilePath,
		ipAddress,
		sizeof(ipAddress),
		&threadState,
		executeTest);

	// Free the memory used by the results instance.
	ResultsIpiFree(threadState.results);

	if (ThreadingGetIsThreadSafe()) {
		THREAD_EXIT;
	}
}

/**
 * Execute memory tests in parallel using a file of null terminated
 * IP Address strings as input. If calibrate is true then the file is read but
 * no detections are performed.
 */
static void runMemoryTests(memoryState* state) {
	int thread;
	state->runningThreads = 0;
	if (ThreadingGetIsThreadSafe()) {
		// Create and start the threads.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_CREATE(
				state->threads[thread],
				(THREAD_ROUTINE)&runMemoryThread,
				state);
		}

		// Wait for them to finish.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_JOIN(state->threads[thread]);
			THREAD_CLOSE(state->threads[thread]);
		}
	}
	else {
		runMemoryThread(state);
	}

	printf("\n\n");
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100)
#endif
/**
 * Increments the state counter to work out the number of IP Addresses.
 */
static void ipAddressCount(const char* ipAddress, void* state) {
	(*(int*)state)++;
}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

/**
 * Iterate the IP Address source file and count the number of lines it
 * contains. Used to determine progress during the test.
 * @param ipAddressFilePath
 * @param number of lines the file contains
 */
static int getIpAddressesCount(const char* ipAddressFilePath) {
	int count = 0;
	char ipAddress[50];
	TextFileIterate(
		ipAddressFilePath,
		ipAddress,
		sizeof(ipAddress),
		&count,
		ipAddressCount);
	return count;
}

/**
 * Sets the test state and then runs calibration, and IP intelligence.
 * @param manager initialised manager to use for the tests
 * @param ipAddressFilePath path to the IP Addresses file to use for the testing.
 */
void run(
	fiftyoneDegreesResourceManager* manager,
	const char* ipAddressFilePath) {
	memoryState state;

	// Set the file name and manager and number of threads.
	state.evidenceFilePath = ipAddressFilePath;
	state.manager = manager;
	state.numberOfThreads = THREAD_COUNT;

	// Count the number of IP Addresses in the source file.
	state.ipAddressesCount = getIpAddressesCount(ipAddressFilePath);

	// Run the test once as the amount of memory used won't vary.
	// Set the process indicator.
	state.progress = state.ipAddressesCount / PROGRESS_MARKS;
	if (!state.progress) {
		state.progress = 1;
	}
	runMemoryTests(&state);

	// Report the maximum memory usage.
	printf("Maximum allocated memory %.2fMBs",
		(double)MemoryTrackingGetMax() / (double)(1024 * 1024));
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any messages
 */
static void reportStatus(
	fiftyoneDegreesStatusCode status,
	const char* fileName) {
	const char* message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

/**
 * Run the memory test from either the tests or the main method.
 * @param dataFilePath full file path to the IP intelligence data file
 * @param ipAddressFilePath full file path to the IP Address test data
 * @param config configuration to use for the memory test
 */
void fiftyoneDegreesMemIpiRun(
	const char* dataFilePath,
	const char* ipAddressFilePath,
	fiftyoneDegreesConfigIpi config) {

	// Ensure the tracking malloc and free methods are used and the counters
	// reset.
	MemoryTrackingReset();
	Malloc = MemoryTrackingMalloc;
	MallocAligned = MemoryTrackingMallocAligned;
	Free = MemoryTrackingFree;
	FreeAligned = MemoryTrackingFreeAligned;

	// Set concurrency to ensure sufficient shared resouces available.
	config.graph.concurrency =
		config.graphs.concurrency =
		config.profiles.concurrency =
		config.profileOffsets.concurrency =
		config.values.concurrency =
		config.strings.concurrency = THREAD_COUNT;

	// Configure to return the IP range properties.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "IpRangeStart,IpRangeEnd,RegisteredCountry";

	ResourceManager manager;
	EXCEPTION_CREATE;
	StatusCode status = IpiInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);
	EXCEPTION_THROW;

	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
	}
	else {
		// Run the performance tests.
		run(&manager, ipAddressFilePath);

		// Free the memory used by the data set.
		ResourceManagerFree(&manager);
	}

	// Ensure the standard malloc and free methods are reinstated now when the
	// tracking has finished.
	Malloc = MemoryStandardMalloc;
	MallocAligned = MemoryStandardMallocAligned;
	Free = MemoryStandardFree;
	FreeAligned = MemoryStandardFreeAligned;
	MemoryTrackingReset();
}


#ifndef TEST

/**
 * Only included if the example is being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 IP Address file path
 */
int main(int argc, char* argv[]) {
	// Memory leak detection code
#ifdef _DEBUG
#ifndef _MSC_VER
#else
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif

	printf("\n");
	printf("\t#############################################################\n");
	printf("\t#                                                           #\n");
	printf("\t#   This program can be used to test the memory usage of    #\n");
	printf("\t#         the 51Degrees 'IP Intelligence' C API.            #\n");
	printf("\t#                                                           #\n");
	printf("\t#  The test will process a list of IP Addresses and output  #\n");
	printf("\t#                  the peak memory usage.                   #\n");
	printf("\t#                                                           #\n");
	printf("\t# Command line arguments should be a IP Intelligence format #\n");
	printf("\t#data file and a CSV file containing a list of IP Addresses.#\n");
	printf("\t#      A test file of 1 million can be downloaded from      #\n");
	printf("\t#            http://51degrees.com/million.zip               #\n");
	printf("\t#                                                           #\n");
	printf("\t#############################################################\n");

	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char ipAddressFilePath[FILE_MAX_PATH];
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = FileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, dataFileName);
		fgetc(stdin);
		return 1;
	}
	if (argc > 2) {
		strcpy(ipAddressFilePath, argv[2]);
	}
	else {
		status = FileGetPath(
			dataDir,
			ipAddressFileName,
			ipAddressFilePath,
			sizeof(ipAddressFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, ipAddressFilePath);
		fgetc(stdin);
		return 1;
	}

	// Report the files that are being used with the performance test.
	printf("\n\nIP Address file is: %s\n\nData file is: %s\n\n",
		FileGetFileName(ipAddressFilePath),
		FileGetFileName(dataFilePath));

	// Wait for a character to be pressed.
	printf("\nPress enter to start memory test.\n");
	fgetc(stdin);

	// Run the performance test.
	fiftyoneDegreesMemIpiRun(
		dataFilePath,
		ipAddressFilePath,
		CONFIG);

#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#else
#endif
#endif

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
