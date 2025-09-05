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

#include "ExampleBase.h"
#include "../../src/fiftyone.h"
#include <string.h>

#define CONFIG_EQUALS(h) memcmp(&config, &h, sizeof(ConfigIpi)) == 0

const char* fiftyoneDegreesExampleGetConfigName(ConfigIpi config) {
	if (CONFIG_EQUALS(IpiInMemoryConfig)) {
		return "InMemory";
	}
	if (CONFIG_EQUALS(IpiHighPerformanceConfig)) {
		return "HighPerformance";
	}
	if (CONFIG_EQUALS(IpiLowMemoryConfig)) {
		return "LowMemory";
	}
	if (CONFIG_EQUALS(IpiBalancedConfig)) {
		return "Balanced";
	}
	if (CONFIG_EQUALS(IpiBalancedTempConfig)) {
		return "BalancedTemp";
	}
	return "Unknown";
}

void fiftyoneDegreesExampleMemCheck(
    fiftyoneDegreesExampleParameters *parameters,
    fiftyoneDegreesExampleRunPtr run) {
// Windows specific memory checking
#if defined(_DEBUG) && defined(_MSC_VER)
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
	
// Use memory tracking for non Windows platforms. can be enabled on Windows
// platform via FORCE_MEMORY_TRACKING
#if (defined(_DEBUG) && !defined(_MSC_VER)) || defined(FORCE_MEMORY_TRACKING)
	fiftyoneDegreesSetUpMemoryTracking();
#endif

	// Call the actual function.
	run(parameters);

// Use memory tracking for non Windows platforms. can be enabled on Windows
// platform via FORCE_MEMORY_TRACKING
#if (defined(_DEBUG) && !defined(_MSC_VER)) || defined(FORCE_MEMORY_TRACKING)
	if (fiftyoneDegreesUnsetMemoryTracking() != 0) {
		printf("ERROR: There is memory leak. All allocated memory should "
			"be freed at the end of this test.\n");
		exit(1);
	}
#endif

// Windows specific memory checking
#if defined(_DEBUG) && defined(_MSC_VER)
	_CrtDumpMemoryLeaks();
#endif
}

void fiftyoneDegreesExampleCheckDataFile(
	fiftyoneDegreesDataSetIpi *dataset) {
	Item item;
	DataReset(&item.data);

	EXCEPTION_CREATE
	StoredBinaryValueGet(
		dataset->strings,
		dataset->header.nameOffset,
		FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING, // result is unused anyway
		&item,
		exception);
	EXCEPTION_THROW

	const char* dataTier = STRING(item.data.ptr); // product is string

	fiftyoneDegreesDate date = dataset->header.published;
	struct tm gt;
	gt.tm_year = (date.year >= 1900) ? date.year - 1900 : date.year;
	gt.tm_mday = date.day;
	gt.tm_mon = (int)date.month - 1;
	gt.tm_hour = 0;
	gt.tm_min = 0;
	gt.tm_sec = 0;
	gt.tm_isdst = 0;
	gt.tm_wday = 0;
	gt.tm_yday = 0;
	time_t published = mktime(&gt);
	time_t now = time(NULL);

	char timeStr[500] = "";
#if defined( _MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
	if (asctime_s(timeStr, 500, (const struct tm*)&gt) != 0) {
#else
	if (asctime_r((const struct tm*)&gt, timeStr) != timeStr) {
#endif
		printf("\033[0;33m");
		printf(("Failed to obtain textual representation of the data file "
			"published date\n"));
		printf("\033[0m");
	}

	printf("Using a %s data file created %s from location %s\n",
		dataTier, timeStr, dataset->b.b.fileName);

#define DATA_FILE_AGE_WARNING 30
	if ((now - published) / (24 * 60 * 60) > DATA_FILE_AGE_WARNING) {
		printf("\033[0;33m");
		printf(("This example is using a data file "
			"that is more than %d days old. "
			"A more recent data file may be needed to "
			"correctly detect the IP ranges."
			"The latest lite data file is available from "
			"the ip-intelligence-data repository on GitHub "
			"https://github.com/51Degrees/ip-intelligence-data. "
			"Find out about the Enterprise data file, which "
			"includes automatic daily updates, on our pricing "
			"page: https://51degrees.com/pricing\n"), DATA_FILE_AGE_WARNING);
		printf("\033[0m");
	}

	if (strncmp(dataTier, "Lite", strlen("Lite")) == 0) {
		printf(("This example is using the \"Lite\" "
			"data file. This is used for illustration, and "
			"has limited accuracy and capabilities. Find "
			"out about the Enterprise data file on our "
			"pricing page: https://51degrees.com/pricing\n"));
	}
	if (!CollectionGetIsMemoryOnly()) {
		COLLECTION_RELEASE(dataset->strings, &item);
	}
}

uint32_t fiftyoneDegreesIterateFakeIPv4s(
	uint32_t const rangeStart,
	uint32_t const rangeEnd,
	uint32_t const increment,
	fiftyoneDegreesIpAddressHandlerPtr const ipAddressHandler,
	void * const state) {

	char buffer[16];
	uint32_t count = 0;
	for (uint64_t i = rangeStart; i <= rangeEnd; i += increment, ++count) {
		{
			StringBuilder builder = { buffer, sizeof(buffer) };
			StringBuilderInit(&builder);
			StringBuilderAddInteger(&builder, (uint8_t)((i >> 24) & 0xFF));
			StringBuilderAddChar(&builder, '.');
			StringBuilderAddInteger(&builder, (uint8_t)((i >> 16) & 0xFF));
			StringBuilderAddChar(&builder, '.');
			StringBuilderAddInteger(&builder, (uint8_t)((i >> 8) & 0xFF));
			StringBuilderAddChar(&builder, '.');
			StringBuilderAddInteger(&builder, (uint8_t)(i & 0xFF));
			StringBuilderComplete(&builder);
		}
		ipAddressHandler(buffer, state);
	}
	return count;
}

// Parameters used for allocating memory when reading evidence.
#define SIZE_OF_KEY 500
#define SIZE_OF_VALUE 1000
#define MAX_EVIDENCE 20

/**
 * Data for TextFileIterate-to-YamlFileIterate callback adapter.
 */
typedef struct {
	char * const ipAddressBuffer;
	const int bufferLength;
	void * const state;
	void(* const callback)(const char*, void *);
} YamlCallbackState;

/**
 * Adapter of TextFileIterate-compatible callback
 * to YamlFileIterate-compatible one.
 *
 * @param pairs Pairs of YAML document
 * @param size Number of pairs in the YAML document
 * @param callbackState Wrapped callback and parameters
 */
static void yamlCallback(
	fiftyoneDegreesKeyValuePair * const pairs,
	const uint16_t size,
	void * const callbackState) {

	const YamlCallbackState * const state =
		(const YamlCallbackState *)callbackState;
	for (uint16_t i = 0; i < size; i++) {
		const size_t writeableBufferLength = (state->bufferLength > 0
			? (size_t)(state->bufferLength - 1) : 0);
		size_t const length = (writeableBufferLength < pairs[i].valueLength
			? writeableBufferLength : pairs[i].valueLength);
		memcpy(state->ipAddressBuffer, pairs[i].value, length);
		state->ipAddressBuffer[length] = '\0';
		state->callback(state->ipAddressBuffer, state->state);
	}
}

void fiftyoneDegreesEvidenceFileIterate(
	const char * const fileName,
	char * const buffer,
	const int length,
	void * const state,
	void(* const callback)(const char*, void *)) {

	// Allocate working memory for iterating over the YAML evidence source.
	char evidenceBuffer[MAX_EVIDENCE * (SIZE_OF_KEY + SIZE_OF_VALUE)];
	KeyValuePair pair[MAX_EVIDENCE];
	char key[MAX_EVIDENCE][SIZE_OF_KEY];
	char value[MAX_EVIDENCE][SIZE_OF_VALUE];
	for (int i = 0; i < MAX_EVIDENCE; i++) {
		pair[i].key = key[i];
		pair[i].keyLength = SIZE_OF_KEY;
		pair[i].value = value[i];
		pair[i].valueLength = SIZE_OF_VALUE;
	}

	YamlCallbackState callbackState = {
		buffer,
		length,
		state,
		callback,
	};

	YamlFileIterate(
		fileName,
		evidenceBuffer,
		sizeof(evidenceBuffer),
		pair,
		MAX_EVIDENCE,
		&callbackState,
		yamlCallback);
}

void fiftyoneDegreesEvidenceFileIterateWithLimit(
	const char * const fileName,
	char * const buffer,
	const int length,
	int limit,
	void * const state,
	void(* const callback)(const char*, void *)) {

	// Allocate working memory for iterating over the YAML evidence source.
	char evidenceBuffer[MAX_EVIDENCE * (SIZE_OF_KEY + SIZE_OF_VALUE)];
	KeyValuePair pair[MAX_EVIDENCE];
	char key[MAX_EVIDENCE][SIZE_OF_KEY];
	char value[MAX_EVIDENCE][SIZE_OF_VALUE];
	for (int i = 0; i < MAX_EVIDENCE; i++) {
		pair[i].key = key[i];
		pair[i].keyLength = SIZE_OF_KEY;
		pair[i].value = value[i];
		pair[i].valueLength = SIZE_OF_VALUE;
	}

	YamlCallbackState callbackState = {
		buffer,
		length,
		state,
		callback,
	};

	YamlFileIterateWithLimit(
		fileName,
		evidenceBuffer,
		sizeof(evidenceBuffer),
		pair,
		MAX_EVIDENCE,
		limit,
		&callbackState,
		yamlCallback);
}
