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

/**
@example IpIntelligence/PerfIpi.c
Getting started example of using 51Degrees IP intelligence.

The example shows how to run a performance test on 51Degrees on-premise
IP intelligence APIs.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/C/IpIntelligence/PerfIpi.c).

@include{doc} example-require-datafile-ipi.txt

@include{doc} example-how-to-run-ipi.txt

In detail, the example performs two separate measurements. One
does not do any actual detections and one does. The difference
is then taken to calculate the actual performance of the detection
work.

Expected output:
```
...
Caching Data pass 1 of 1:

        [========================================]

Calibration pass 1 of 1:

        [========================================]

Detection test pass 1 of 1:

        [========================================] 6416:1.000000|6417:1.000000|0:1.000000|0:1.000000|...

Total seconds for 80000 IP Addresses over 4 thread(s): 0.** s
Average matching per second: ***
...
```

*/

#ifdef _DEBUG
#define PASSES 1
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#else
#define PASSES 1
#endif

#include <time.h>

#include "../../Base/ExampleBase.h"
#include "../../../src/ipi.h"
#include "../../../src/fiftyone.h"
#include "../../../src/common-cxx/textfile.h"

 // Size of the character buffers
#define BUFFER 1000

// Number of marks to make when showing progress.
#define PROGRESS_MARKS 40

// Number of threads to start for performance analysis.
#define THREAD_COUNT 4

static const char* dataDir = "ip-intelligence-data";

static const char* dataFileName = "51Degrees-LiteV41.ipi";

static const char* ipAddressFileName = "evidence.yml";

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
  * Shared performance state. All members are immutable except runningThreads
  * which is updated via an interlocked compare and exchange.
  */
typedef struct t_performance_state {
	int ipAddressesCount; // Total number of IP Addresses
	int progress; // Number of IP Addresses to process for each = marker
	bool calibration; // True if calibrating, otherwise false
	const char* ipAddressFilePath; // Filename for the IP Address file
	int numberOfThreads; // Number of parallel threads
	fiftyoneDegreesResourceManager* manager; // Manager resource for detection
	volatile long runningThreads; // Number of active running threads
	FIFTYONE_DEGREES_THREAD threads[THREAD_COUNT]; // Running threads
} performanceState;

/**
 * Individual thread performance state where all members are exclusively
 * accessed by a single test thread.
 */
typedef struct t_thread_performance_state {
	performanceState* main; // Reference to the main threads shared state
	long count; // Number of IP Addresses the thread has processed
	bool reportProgress; // True if this thread should report progress
	fiftyoneDegreesResultsIpi* results; // The results used by the thread
} performanceThreadState;

/**
 * Prints the progress bar based on the state provided.
 * @param state information about the overall performance test
 */
static void printLoadBar(performanceThreadState* state) {
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
static void reportProgress(performanceThreadState* state) {
	EXCEPTION_CREATE;
	char networkName[1024] = "";

	// Update the user interface.
	printLoadBar(state);

	// If in real detection mode then print the name of the network profile found
	// to prove it's actually doing something!
	if (state->results != NULL) {
		printf(" ");
		ResultsIpiGetValuesString(
			state->results,
			"RegisteredName",
			networkName,
			sizeof(networkName),
			", ",
			exception);
		EXCEPTION_THROW;
		printShortenNetworkName(networkName);
	}
}

/**
 * Runs the performance test for the IP Address provided. Called from the text
 * file iterator.
 * @param ipAddress to be used for the test
 * @param state instance of performanceThreadState
 */
static void executeTest(const char* ipAddress, void* state) {
	performanceThreadState* threadState = (performanceThreadState*)state;
	IpAddress eIpAddress;
	EXCEPTION_CREATE;

	// Parse the IP Address string to a byte array
	const bool parsed = fiftyoneDegreesIpAddressParse(
			ipAddress, 
			ipAddress + strlen(ipAddress),
			&eIpAddress);

	if (parsed) {
		// If not calibrating the test environment perform IP intelligence.
		if (threadState->main->calibration == false) {
			ResultsIpiFromIpAddress(
				threadState->results,
				eIpAddress.value,
				eIpAddress.type == IP_TYPE_IPV4 ?
					FIFTYONE_DEGREES_IPV4_LENGTH : 
					FIFTYONE_DEGREES_IPV6_LENGTH,
				(IpType)eIpAddress.type,
				exception);
			EXCEPTION_THROW;
		}
	}
	else {
		// Terminates as failed to allocate memory for IP address
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
		EXCEPTION_THROW;
	}

	// Increase the count for this performance thread and update the user
	// interface if a progress mark should be written.
	threadState->count++;
	if (threadState->reportProgress == true &&
		threadState->count % threadState->main->progress == 0) {
		reportProgress(threadState);
	}
}

/**
 * A single threaded performance test. Many of these will run in parallel to
 * ensure the single managed resource is being used.
 * @param mainState state information about the main test
 */
static void runPerformanceThread(void* mainState) {
	char ipAddress[BUFFER] = "";
	performanceThreadState threadState;
	threadState.main = (performanceState*)mainState;

	// Ensure that only one thread reports progress. Avoids keeping a running
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

	if (threadState.main->calibration == 0) {
		// Create an instance of results to access the returned values.
		threadState.results = ResultsIpiCreate(
			threadState.main->manager);
	}
	else {
		threadState.results = NULL;
	}

	// Execute the performance test or calibration.
	fiftyoneDegreesEvidenceFileIterate(
		threadState.main->ipAddressFilePath,
		ipAddress,
		sizeof(ipAddress),
		&threadState,
		executeTest);

	if (threadState.main->calibration == 0) {
		// Free the memory used by the results instance.
		ResultsIpiFree(threadState.results);
	}

	if (ThreadingGetIsThreadSafe()) {
		THREAD_EXIT;
	}
}

/**
 * Execute performance tests in parallel using a file of null terminated
 * IP Address strings as input. If calibrate is true then the file is read but
 * no detections are performed.
 */
static void runPerformanceTests(performanceState* state) {
	int thread;
	state->runningThreads = 0;
	if (ThreadingGetIsThreadSafe()) {

		// Create and start the threads.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_CREATE(
				state->threads[thread],
				(THREAD_ROUTINE)&runPerformanceThread,
				state);
		}

		// Wait for them to finish.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_JOIN(state->threads[thread]);
			THREAD_CLOSE(state->threads[thread]);
		}
	}
	else {
		runPerformanceThread(state);
	}

	printf("\n\n");
}

/**
 * Runs calibration and real test passes working out the total average time
 * to complete IP intelligence on the IP Address provided in multiple
 * threads.
 * @param state main state information for the tests.
 * @param passes the number of calibration and test passes to perform to work
 * out the averages.
 * @param test name of the test being performed.
 */
static double runTests(performanceState* state, int passes, const char* test) {
	int pass;
#ifdef _MSC_VER
	double start, end;
#else
	struct timespec start, end;
#endif
	fflush(stdout);

	// Set the progress indicator.
	state->progress = state->ipAddressesCount / PROGRESS_MARKS;
	if (!state->progress) {
		state->progress = 1;
	}

	// Perform a number of passes of the test.
#ifdef _MSC_VER
	start = GetTickCount();
#else
	clock_gettime(CLOCK_MONOTONIC, &start);
#endif
	for (pass = 1; pass <= passes; pass++) {
		printf("%s pass %i of %i: \n\n", test, pass, passes);
		runPerformanceTests(state);
	}

#ifdef _MSC_VER
	end = GetTickCount();
	return (end - start) / (double)1000 / (double)passes;
#else
	clock_gettime(CLOCK_MONOTONIC, &end);
	return ((double)(end.tv_sec - start.tv_sec) +
		(end.tv_nsec - start.tv_nsec) / 1.0e9) / (double)passes;
#endif
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
 * @return number of lines the file contains
 */
static int getIpAddressesCount(const char* ipAddressFilePath) {
	int count = 0;
	char ipAddress[BUFFER];
	fiftyoneDegreesEvidenceFileIterate(
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
 * @param ipAddressFilePath path to the IP Addresses file to use for testing.
 */
static void run(
	fiftyoneDegreesResourceManager* manager,
	const char* ipAddressFilePath) {
	performanceState state;
	double total, test, calibration;

	// Set the file name and manager.
	state.ipAddressFilePath = ipAddressFilePath;
	state.manager = manager;

	// Count the number of IP Addresses in the source file.
	state.ipAddressesCount = getIpAddressesCount(ipAddressFilePath);

	// Get the number of records so the progress bar prints nicely.
	state.numberOfThreads = 1;
	state.calibration = 1;
	runTests(&state, 1, "Caching Data");

	// Set the state for the calibration.
	state.numberOfThreads = THREAD_COUNT;

	// Run the process without doing any detections to get a
	// calibration time.
	calibration = runTests(&state, PASSES, "Calibration");

	// Process the data file doing the IP intelligence.
	state.calibration = 0;
	test = runTests(&state, PASSES, "Detection test");

	// Work out the time to complete the IP intelligenc ignoring the time
	// taken to read the data from the file system.
	total = test - calibration;
	if (total < 0) total = test;

	// Report the performance times.
	printf("Total seconds for %i IP Addresses over %i thread(s): %.2fs\n",
		state.ipAddressesCount * state.numberOfThreads,
		state.numberOfThreads,
		total);
	printf("Average matching per second: %.0f\n",
		(double)(state.ipAddressesCount * state.numberOfThreads) / total);
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
 * Run the performance test from either the tests or the main method.
 * @param dataFilePath full file path to the IP intelligence data file
 * @param ipAddressFilePath full file path to the IP Address test data
 * @param config configuration to use for the performance test
 */
void fiftyoneDegreesPerfIpiRun(
	const char* dataFilePath,
	const char* ipAddressFilePath,
	fiftyoneDegreesConfigIpi config) {

	// Set concurrency to ensure sufficient shared resources available.
	config.graph.concurrency =
		config.graphs.concurrency =
		config.profiles.concurrency =
		config.profileOffsets.concurrency =
		config.values.concurrency =
		config.strings.concurrency = THREAD_COUNT;
	config.strings.capacity = 100;

	// Configure to return the Country property.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "RegisteredName,areas";

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
}

#ifndef TEST

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 IP Address file path
 */
int main(int argc, char* argv[]) {

	// Memory leak IP intelligence.
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
	printf("\t#  This program can be used to test the performance of the  #\n");
	printf("\t#           51Degrees 'IP Intelligence' C API.              #\n");
	printf("\t#                                                           #\n");
	printf("\t#  The test will read a list of IP Addresses and calculate  #\n");
	printf("\t#            the number of matchings per second.            #\n");
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
	printf("\nPress enter to start performance tests.\n");
	fgetc(stdin);

	// Run the performance test.
	fiftyoneDegreesPerfIpiRun(dataFilePath, ipAddressFilePath, CONFIG);

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
