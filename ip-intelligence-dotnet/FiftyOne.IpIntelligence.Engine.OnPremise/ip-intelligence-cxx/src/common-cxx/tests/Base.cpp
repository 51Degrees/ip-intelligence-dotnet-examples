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

#include "Base.hpp"

/**
 * Create a memory checkpoint before the test method is called. Also resets
 * 51Degrees memory tracking counters.
 */
void Base::SetUpMemoryCheck() {
	// Reset the memory tracking counters in case the test will use memory
	// allocation tracking.
	fiftyoneDegreesMemoryTrackingReset();

#ifdef _DEBUG
#ifdef _MSC_FULL_VER
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);

	// Take a sample of the memory before allocating anything.
	_CrtMemCheckpoint(&_states.s1);
#else
	// Setup the memory track.
	fiftyoneDegreesSetUpMemoryTracking();
#endif
#endif
}

/**
 * Begins the memory leak check.
 */
void Base::SetUp() {
	SetUpMemoryCheck();
}

/**
 * Checks that the status code is success. If not then a message is output for
 * the status code and the test will stop.
 * @param status code to be checked.
 * @parma fileName of the data file used for the associated init, create or 
 * size method. NULL if no data file was used.
 */
void Base::AssertStatus(
	fiftyoneDegreesStatusCode status, 
	const char *fileName) {
	const char *message = fiftyoneDegreesStatusGetMessage(status, fileName);
	ASSERT_EQ(status, FIFTYONE_DEGREES_STATUS_SUCCESS) << message;
	free((void*)message);
}

/**
 * Iterates up the folders from the current working directory until a file
 * in the sub folder dataFolderName with the name fileName is found which
 * can be opened. This is assumed to be the data file required by the test.
 * @param dataFolderName the name of the sub folder which is expected to 
 * contain the data file.
 * @param fileName the name of the data file.
 * @return the full path to the data file requested if found, otherwise NULL.
 */
string Base::GetFilePath(string dataFolderName, string fileName) {
	char buffer[FIFTYONE_DEGREES_FILE_MAX_PATH];
	string filePath = "";
	if (fiftyoneDegreesFileGetPath(
		dataFolderName.c_str(),
		fileName.c_str(),
		buffer,
		sizeof(buffer)) == FIFTYONE_DEGREES_STATUS_SUCCESS) {
		filePath.assign(buffer);
	}
	return filePath;
}

/**
 * Concludes the memory leak check by creating a memory checkpoint after the
 * test method is called and comparing to the first.
 */
size_t Base::PerformMemoryCheck() {
	size_t leakedMemory = 0;
#ifdef _DEBUG
#ifdef _MSC_FULL_VER
	_CrtMemState s3;
	// Take a sample of the memory now that everything is done.
	_CrtMemCheckpoint(&_states.s2);

	// Check there were no memory leaks.
	leakedMemory = _CrtMemDifference(&s3, &_states.s1, &_states.s2);
	if (leakedMemory) {
		// There were, so dump the info and fail the test.
		_CrtMemDumpAllObjectsSince(&_states.s1);
		_CrtMemDumpStatistics(&s3);
	}
#else
	// Check memory leaks.
	leakedMemory = fiftyoneDegreesUnsetMemoryTracking();
#endif
#endif
	return leakedMemory;
}

/**
 * Check if there is memory which has not been freed. If yes then fails the 
 * test.
 */
void Base::TearDown() {
	if (PerformMemoryCheck()) {
		FAIL() << "There was a memory leak, see debug log for details.";
	}
}

/**
 * Check that multiple threads can run a method safely.
 * @param concurrency number of threads to run the method in
 * @param state pointer to pass to the method
 * @param runThread method to run in each thread
 */
void Base::runThreads(
	int concurrency,
	FIFTYONE_DEGREES_THREAD_ROUTINE runThread) {
	int thread;
	FIFTYONE_DEGREES_THREAD *threads =
		(FIFTYONE_DEGREES_THREAD*)malloc(
			sizeof(FIFTYONE_DEGREES_THREAD) * concurrency);

	// Create the threads.
	for (thread = 0; thread < concurrency; thread++) {
		FIFTYONE_DEGREES_THREAD_CREATE(
			threads[thread],
			runThread,
			this);
	}
	// Wait for them to finish.
	for (thread = 0; thread < concurrency; thread++) {
		FIFTYONE_DEGREES_THREAD_JOIN(threads[thread]);
		FIFTYONE_DEGREES_THREAD_CLOSE(threads[thread]);
	}
	free((void*)threads);
}
