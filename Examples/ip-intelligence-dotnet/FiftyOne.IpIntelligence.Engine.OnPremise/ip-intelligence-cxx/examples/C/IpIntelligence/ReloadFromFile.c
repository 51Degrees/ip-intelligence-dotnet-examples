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
@example IpIntelligence/ReloadFromFile.c
Reload from file example of using 51Degrees IP intelligence.

This example illustrates how to use a single reference to the resource manager
to use 51Degrees on-premise IP intelligence and invoke the reload functionality
instead of maintaining a reference to the dataset directly.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/C/IpIntelligence/ReloadFromFile.c).

@include{doc} example-require-datafile-ipi.txt

@include{doc} example-how-to-run-ipi.txt

In detail, this example shows how to:

1. Only maintain a reference to the fiftyoneDegreesResourceManager and use the
reference to access dataset.

2. Use the #fiftyoneDegreesIpiReloadManagerFromOriginalFile function to
reload the dataset from the same location and with the same	set of properties.

3. Retrieve a results instance from the data set and release it when done with
detecting current IP Address.

4. Use the reload functionality in a single threaded environment.

5. Use the reload functionality in a multi threaded environment.

The #fiftyoneDegreesIpiReloadManagerFromOriginalFile function requires an
existing resource with the initialized dataset. Function reloads the dataset
from the same location and with the same parameters as the original dataset.

Please keep in mind that even if the current dataset was constructed with
all available properties this does not guarantee that the new dataset will
be initialized with the same set of properties. If the new data file
contains properties that were not part of the original data file, the new
extra property(ies) will not be initialized. If the new data file does not
contain one or more property that were previously available, then these
property(ies) will not be initialized.

Each successful data file reload should be accompanied by the integrity
check to verify that the properties you want have indeed been loaded. This
can be achieved by simply comparing the number of properties before and
after the reload as the number can not go up but it can go down.

The reload functionality works both with the single threaded as well as the
multi threaded modes. To try the reload functionality in single threaded
mode build with FIFTYONE_DEGREES_NO_THREADING defined. Or build without
FIFTYONE_DEGREES_NO_THREADING for multi threaded example.

In a single threaded environment the reload function is executed as part of
the normal flow of the program execution and will prevent any other actions
until the reload is complete.

Expected output:
```
...
Finished with hash code '548397166'
Finished with hash code '548397166'
Finished with hash code '548397166'
Finished with hash code '548397166'
Reloaded '17' times.
Failed to reload '0' times.
...
```

*/

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

#include "../../Base/ExampleBase.h"
#include "../../../src/common-cxx/textfile.h"
#include "../../../src/ipi.h"
#include "../../../src/fiftyone.h"

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
 * State containing the states for all threads running in the example.
 */
typedef struct shared_state_t {
	ResourceManager* manager; /**< Pointer to the manager containing the data
							  set */
	const char* ipAddressFilePath; /**< Path to the IP Addresses to process */
	volatile long threadsFinished; /**< Number of threads that have finished
								   their processing */
	THREAD threads[THREAD_COUNT]; /**< Pointers to the running threads */
} sharedState;

/**
 * State for a single thread carrying out processing.
 */
typedef struct thread_state_t {
	ResourceManager* manager; /**< Pointer to the manager containing the data
							  set */
	unsigned long hashCode; /**< Running hash code for the processing being carried out.
				  This is used to verify the work carried out */
} threadState;

/**
 * Returns a basic hashcode for the string value provided.
 * @param value string whose hashcode is required.
 * @return the hashcode for the string provided.
 */
static unsigned long generateHash(unsigned char* value) {
	unsigned long hashCode = 5381;
	int i;
	while ((i = *value++)) {
		hashCode = ((hashCode << 5) + hashCode) + i;
	}
	return hashCode;
}

/**
 * Returns the hash code for the values of properties contained in the results.
 * @param results containing the results of processing
 */
static unsigned long getHashCode(ResultsIpi* results) {
	EXCEPTION_CREATE;
	const ProfilePercentage* valueItem;
	unsigned long hashCode = 0;
	uint32_t requiredPropertyIndex;
	const char* valueName;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;
	for (requiredPropertyIndex = 0;
		requiredPropertyIndex < dataSet->b.b.available->count;
		requiredPropertyIndex++) {
		EXCEPTION_CLEAR;
		if (ResultsIpiGetHasValues(
			results,
			requiredPropertyIndex,
			exception) == true) {
			EXCEPTION_THROW;
			valueItem = ResultsIpiGetValues(
				results,
				requiredPropertyIndex,
				exception);
			EXCEPTION_THROW;
			valueName = STRING(valueItem->item.data.ptr); // FIXME: value may not be a string
			hashCode ^= generateHash((unsigned char*)(valueName));
		}
	}
	return hashCode;
}

/**
 * Runs the performance test for the IP Address provided. Called from the text
 * file iterator.
 * @param ipAddress to be used for the test
 * @param state instance of performanceThreadState
 */
static void executeTest(const char* ipAddress, void* state) {
	threadState* thread = (threadState*)state;
	ResultsIpi* results = ResultsIpiCreate(
		thread->manager);
	EXCEPTION_CREATE;
	ResultsIpiFromIpAddressString(
		results,
		ipAddress,
		strlen(ipAddress),
		exception);
	EXCEPTION_THROW;
	thread->hashCode ^= getHashCode(results);
	ResultsIpiFree(results);
}

static void runRequestsSingle(sharedState* state) {
	sharedState* const shared = (sharedState*)state;
	threadState thread;
	thread.hashCode = 0;
	thread.manager = shared->manager;

	const uint32_t ipsCount = fiftyoneDegreesIterateFakeIPv4s(
		0x00000000U,
		0xFFFFE381U,
		0x00068DB8U,
		executeTest,
		&thread);

	printf("Finished '%lu' addresses with hash code '%lu'\r\n",
		(unsigned long)ipsCount, (unsigned long)thread.hashCode);
}

static void runRequestsMulti(void* state) {
	sharedState* shared = (sharedState*)state;
	runRequestsSingle(shared);
	INTERLOCK_INC(&shared->threadsFinished);
}

/**
 * Starts threads that run IP intelligence. Must be done after the dataset
 * has been initialized.
 */
static void startThreads(sharedState* state) {
	int thread;
	for (thread = 0; thread < THREAD_COUNT; thread++) {
		THREAD_CREATE(
			state->threads[thread],
			(THREAD_ROUTINE)&runRequestsMulti,
			state);
	}
}

/**
 * Joins the threads and frees the memory occupied by the threads.
 */
static void joinThreads(sharedState* state) {
	int thread;
	for (thread = 0; thread < THREAD_COUNT; thread++) {
		THREAD_JOIN(state->threads[thread]);
		THREAD_CLOSE(state->threads[thread]);
	}
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any messages
 */
static void reportStatus(
	StatusCode status,
	const char* fileName) {
	const char* message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

static void run(
	ResourceManager* manager,
	const char* ipAddressFilePath) {
	StatusCode status;
	int numberOfReloads = 0;
	int numberOfReloadFails = 0;
	sharedState state;
	state.manager = manager;
	state.ipAddressFilePath = ipAddressFilePath;
	state.threadsFinished = 0;
	EXCEPTION_CREATE;

	if (ThreadingGetIsThreadSafe()) {
		printf("** Multi Threaded Reload Example **\r\n");
		startThreads(&state);
		while (state.threadsFinished < THREAD_COUNT) {
			status = IpiReloadManagerFromOriginalFile(
				manager,
				exception);
			EXCEPTION_THROW;
			if (status == SUCCESS) {
				numberOfReloads++;
			}
			else {
				numberOfReloadFails++;
			}
#ifdef _MSC_VER
			Sleep(1000); // milliseconds
#else
			usleep(1000000); // microseconds
#endif
		}
		joinThreads(&state);
	}
	else {
		printf("** Single Threaded Reload Example **\r\n");
		runRequestsSingle(&state);
		status = IpiReloadManagerFromOriginalFile(
			manager,
			exception);
		EXCEPTION_THROW;
		if (status == SUCCESS) {
			numberOfReloads++;
		}
		else {
			numberOfReloadFails++;
		}
		runRequestsSingle(&state);
	}

	// Report the number of reloads.
	printf("Reloaded '%i' times.\r\n", numberOfReloads);
	printf("Failed to reload '%i' times.\r\n", numberOfReloadFails);
	printf("Program execution complete. Press Return to exit.");
}

void fiftyoneDegreesIpiReloadFromFileRun(
	const char* dataFilePath,
	const char* ipAddressFilePath,
	const char* requiredProperties,
	ConfigIpi config) {

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
	StatusCode status = IpiInitManagerFromFile(
		&manager,
		&config,
		&reqProps,
		dataFilePath,
		exception);
	EXCEPTION_THROW;

	// Free the memory used for the required properties if allocated.
	if (requiredProperties != reqProps.string) {
		free((void*)reqProps.string);
	}

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

int main(int argc, char* argv[]) {

	// Memory leak detection code.
#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif

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

	// Run the performance test.
	fiftyoneDegreesIpiReloadFromFileRun(
		dataFilePath,
		ipAddressFilePath,
		argc > 3 ? argv[3] : "RegisteredName",
		CONFIG);

#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif
#endif

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
