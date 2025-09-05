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
@example IpIntelligence/OfflineProcessing.c
Offline processing example of using 51Degrees IP intelligence.

This example demonstrates one possible use of the 51Degrees on-premise IP intelligence
API and data for offline data processing. It also demonstrates that you can reuse the
retrieved results for multiple uses and only then release it.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/C/IpIntelligence/OfflineProcessing.c).

@include{doc} example-require-datafile-ipi.txt

@include{doc} example-how-to-run-ipi.txt

In detail, the example shows how to:

1. Specify the name of the data file and properties the data set should be
initialised with.
```
const char* fileName = argv[1];
fiftyoneDegreesPropertiesRequired properties =
	fiftyoneDegreesPropertiesDefault;
properties.string = "IpRangeStart,IpRangeEnd,"
		"RegisteredCountry,AccuracyRadius";
```

2. Instantiate the 51Degrees data set within a resource manager from the
specified data file with the required properties and the specified
configuration.
```
fiftyoneDegreesStatusCode status =
	fiftyoneDegreesIpiInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);
```

3. Create a results instance ready to be populated by the data set.
```
fiftyoneDegreesResultsIpi* results =
	fiftyoneDegreesResultsIpiCreate(
		&manager);
```

4. Open an output file to write the results to.
```
	FILE* fout = fopen(outputFile, "w");
```

5. Write a header to the output file with the property names in '|'	separated
CSV format ('|' separated because some properties contain commas)
```
fprintf(fout, "IP Address");
for (i = 0; i < dataSet->b.b.available->count; i++) {
	fprintf(fout, ",\"%s\"",
		&((fiftyoneDegreesString*)
			dataSet->b.b.available->items[i].name.data.ptr)->value);
}
fprintf(fout, "\n");
```

6. Iterate over the IP Addresses in an input file writing the processing results
to the output file.
```
fiftyoneDegreesTextFileIterate(
	ipAddressFilePath,
	ipAddress,
	sizeof(ipAddress),
	&state,
	executeTest);
```

7. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```

Expected output:
```
Output Written to [Full Path]/ip-intelligence-data/20000 IP Addresses.processed.csv

```
Expected content of output file:
```
"IP Address"|"AccuracyRadius"|"RegisteredCountry"|"IpRangeEnd"|"IpRangeStart"
"::f33e:25e3:c5bd:e182:e584"|"0.000000,0.000000":"1.000000"|"ZZ":"1.000000"|...
"0fa0:3a4::2626"|"0.000000,0.000000":"1.000000"|"ZZ":"1.000000"|"2001:023f:...
...
```

*/

#ifdef _DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include "../../Base/ExampleBase.h"
#include "../../../src/ipi.h"
#include "../../../src/common-cxx/textfile.h"
#include "../../../src/fiftyone.h"

static const char* dataDir = "ip-intelligence-data";

static const char* dataFileName = "51Degrees-LiteV41.ipi";

static const char* ipAddressFileName = "evidence.yml";

static char valueBuffer[1024] = "";

static char* getPropertyValueAsString(
	ResultsIpi* results,
	uint32_t requiredPropertyIndex) {
	EXCEPTION_CREATE;
	ResultsIpiGetValuesStringByRequiredPropertyIndex(
		results,
		requiredPropertyIndex,
		valueBuffer,
		sizeof(valueBuffer),
		",",
		exception);
	EXCEPTION_THROW;
	return valueBuffer;
}

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
  * State used for the offline processing operation.
  */
typedef struct t_offline_processing_state {
	FILE* output; /**< Output stream for the results */
	ResultsIpi* results; /**< The results used by the thread */
} offlineProcessState;

/**
 * Processes the IP Addresses provided, writing the results to the output file.
 * Called from the text file iterator.
 * @param ipAddress to be used for the test
 * @param state instance of offlineProcessState
 */
static void process(const char* ipAddress, void* state) {
	EXCEPTION_CREATE;
	uint32_t i;
	offlineProcessState* offline = (offlineProcessState*)state;
	ResultIpi* result;
	DataSetIpi* dataSet = (DataSetIpi*)offline->results->b.dataSet;
	ResultsIpiFromIpAddressString(
		offline->results,
		ipAddress,
		strlen(ipAddress),
		exception);
	result = (ResultIpi*)offline->results->items;

	// Write the IP Address.
	fprintf(offline->output, "\"%s\"", ipAddress);

	// Write all the available properties.
	for (i = 0; i < dataSet->b.b.available->count; i++) {
		if (ResultsIpiGetValues(
			offline->results,
			i,
			exception) == NULL ||
			EXCEPTION_FAILED ||
			offline->results->values.count == 0) {

			// Write an empty value if one isn't available.
			fprintf(offline->output, ",\"\"");
		}
		else {
			// Write value(s) with pipeline separator.
			fprintf(offline->output, "|");
			// Write weighted value with comma separator
			// between value and its weight.
			fprintf(offline->output, "%s", getPropertyValueAsString(
				offline->results,
				i));
		}
	}
	fprintf(offline->output, "\n");
}

void run(
	ResourceManager* manager,
	const char* ipAddressFilePath,
	const char* outputFilePath) {
	uint32_t i;
	char ipAddress[50];
	offlineProcessState state;
	DataSetIpi* dataSet;

	// Open a fresh data file for writing the output to.
	FileDelete(outputFilePath);
	state.output = fopen(outputFilePath, "w");
	if (state.output == NULL) {
		printf("Could not open file %s for write\n", outputFilePath);
		return;
	}

	// Get the results and data set from the manager.
	state.results = ResultsIpiCreate(manager);
	dataSet = (DataSetIpi*)state.results->b.dataSet;

	printf("Starting Offline Processing Example.\n");

	// Print CSV headers to output file.
	fprintf(state.output, "\"IP Address\"");
	for (i = 0; i < dataSet->b.b.available->count; i++) {
		fprintf(state.output, "|\"%s\"",
			&((String*)dataSet->b.b.available->items[i].name.data.ptr)->value);
	}
	fprintf(state.output, "\n");

	// Perform offline processing.
	fiftyoneDegreesEvidenceFileIterate(
		ipAddressFilePath,
		ipAddress,
		sizeof(ipAddress),
		&state,
		process);

	fclose(state.output);
	printf("Output Written to %s\n", outputFilePath);

	// Free the memory used by the results instance.
	ResultsIpiFree(state.results);
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

/**
 * Start the offline processing with the files and configuration provided.
 * @param dataFilePath full file path to the ip intelligence data file
 * @param ipAddressFilePath full file path to the IP Address test data
 * @param config configuration to use for the memory test
 */
void fiftyoneDegreesOfflineProcessingRun(
	const char* dataFilePath,
	const char* ipAddressFilePath,
	const char* outputFilePath,
	const char* requiredProperties,
	ConfigIpi config) {
	EXCEPTION_CREATE;

	// Set concurrency to ensure sufficient shared resources available.
	config.graph.concurrency =
		config.graphs.concurrency =
		config.profiles.concurrency =
		config.profileOffsets.concurrency =
		config.values.concurrency =
		config.strings.concurrency = 1;

	// Set the required properties for the output file.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = requiredProperties;

	ResourceManager manager;
	StatusCode status = IpiInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);

	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
	}
	else {

		// Process the IP Addresses writing the results to the output path.
		run(&manager, ipAddressFilePath, outputFilePath);

		// Free the memory used by the data set.
		ResourceManagerFree(&manager);
	}
}

#ifndef TEST

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 IP Addresses file path
 */
int main(int argc, char* argv[]) {
	int i = 0;
	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char ipAddressFilePath[FILE_MAX_PATH];
	char outputFilePath[FILE_MAX_PATH];
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
	if (argc > 3) {
		strcpy(outputFilePath, argv[3]);
	}
	else {
		while (ipAddressFilePath[i] != '.' && ipAddressFilePath[i] != '\0') {
			outputFilePath[i] = ipAddressFilePath[i];
			i++;
		}
		strcpy(&outputFilePath[i], ".processed.csv");
	}

#ifdef _DEBUG
#ifndef _MSC_VER
#else
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif

	// Start the offline processing.
	// TODO: Revert to "IpRangeStart,IpRangeEnd,RegisteredCountry,AccuracyRadius"
	fiftyoneDegreesOfflineProcessingRun(
		dataFilePath,
		ipAddressFilePath,
		outputFilePath,
		argc > 4 ? argv[4] : "name,areas",
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
