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
#include <string.h>
#include "../../../src/ipi.h"
#include "../../../src/fiftyone.h"

static const char* dataDir = "ip-intelligence-data";

static const char* dataFileName = "51Degrees-LiteV41.ipi";

static void buildString(
	fiftyoneDegreesResultsIpi* const results,
	StringBuilder* const builder,
	Exception *exception) {
	int i;
	const char* property;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;
	for (i = 0; i < (int)dataSet->b.b.available->count; i++) {
		property = STRING( // name is string
			PropertiesGetNameFromRequiredIndex(
				dataSet->b.b.available,
				i));
		if (i) {
			StringBuilderAddChar(builder, ';');
		}
		StringBuilderAddChars(builder, property, strlen(property));
		StringBuilderAddChar(builder, '=');
		StringBuilderAddChar(builder, '[');
		ResultsIpiAddValuesString(results, property, builder, ",", exception);
		EXCEPTION_THROW;
		StringBuilderAddChar(builder, ']');
	}
	StringBuilderComplete(builder);
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any progress
 */
static void reportStatus(
	fiftyoneDegreesStatusCode status,
	const char* fileName) {
	const char* message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

static int run(fiftyoneDegreesResourceManager *manager) {
	EXCEPTION_CREATE;
	char ipAddress[50], output[50000];
	int count = 0;
	ResultsIpi *results = ResultsIpiCreate(manager);
	while (fgets(ipAddress, sizeof(ipAddress), stdin) != 0) {
		if (!strlen(ipAddress)) {
			break;
		}
		StringBuilder builder = { output, sizeof(output) };
		StringBuilderInit(&builder);

		// Set the results from the IP address provided from standard in.
		ResultsIpiFromIpAddressString(
			results,
			ipAddress,
			strlen(ipAddress),
			exception);
		EXCEPTION_THROW;

		// Print the values for all the required properties.
		buildString(results, &builder, exception);
		EXCEPTION_THROW;

		printf("%s\n", output);

		count++;
	}
	ResultsIpiFree(results);
	return count;
}

int fiftyoneDegreesProcIpiRun(
	const char *dataFilePath,
	const char *requiredProperties,
	fiftyoneDegreesConfigIpi *config) {
	EXCEPTION_CREATE;
	int count = 0;
	ResourceManager manager;
	PropertiesRequired properties = PropertiesDefault;
	properties.string = requiredProperties;
	StatusCode status = IpiInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
	EXCEPTION_THROW;
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
	}
	else {
		count = run(&manager);
		ResourceManagerFree(&manager);
	}
	return count;
}

#ifndef TEST

/**
 * Only included if the example is being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 required properties
 */
int main(int argc, char* argv[]) {
	char dataFilePath[FILE_MAX_PATH];
	StatusCode status = SUCCESS;
	ConfigIpi config = fiftyoneDegreesIpiInMemoryConfig;
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
	if (CollectionGetIsMemoryOnly()) {
		config = fiftyoneDegreesIpiInMemoryConfig;
	}

	// Capture input from standard in and display property value.
	fiftyoneDegreesProcIpiRun(
		dataFilePath,
		argc > 2 ? argv[2] : "IpRangeStart,IpRangeEnd,"
			"RegisteredCountry,AccuracyRadius",
		& config);

		return 0;
}

#endif
