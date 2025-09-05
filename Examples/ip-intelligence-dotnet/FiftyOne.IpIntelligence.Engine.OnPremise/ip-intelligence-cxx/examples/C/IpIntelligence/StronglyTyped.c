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
@example IpIntelligence/StronglyTyped.c
Strongly Typed example of using 51Degrees IP intelligence.

The example shows how to extract the strongly typed value from the
returned results of the 51Degrees on-premise IP intelligence.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/C/IpIntelligence/StronglyTyped.c).

@include{doc} example-require-datafile-ipi.txt

@include{doc} example-how-to-run-ipi.txt

In detail, the example shows how to:

1. Specify the name of the data file and properties the data set should be
initialised with.
```
const char* fileName = argv[1];
fiftyoneDegreesPropertiesRequired properties =
	fiftyoneDegreesPropertiesDefault;
properties.string = "IpRangeStart,IpRangeEnd,RegisteredCountry,AccuracyRadius";
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
fiftyoneDegreesResultsIpi *results =
	fiftyoneDegreesResultsIpiCreate(
		&manager);
```

4. Process a single IP Address string to retrieve the values associated
with the IP Address for the selected properties.
```
fiftyoneDegreesResultsIpiFromIpAddressString(
	results,
	ipv4Address,
	strlen(ipv4Address),
	exception);
```

5. Check the type of values the property returns.
```
fiftyoneDegreesProperty *property = fiftyoneDegreesPropertyGetByName(
	dataSet->properties,
	dataSet->strings,
	propertyName,
	&item,
	exception);
fiftyoneDegreesPropertyValueType type = property->valueType;
```

6. Extract the value of a property as a coordinate from the results.
```
requiredPropertyIndex =
	PropertiesGetRequiredPropertyIndexFromName(
		dataSet->available,
		propertyName);
if (requiredPropertyIndex >= 0) {
	if (ResultsIpiGetValues(
		results,
		requiredPropertyIndex,
		exception) != NULL && EXCEPTION_OKAY) {
		value = IpiGetCoordinate(
			&results->values.items[0].item,
			exception);
	}
}
```

7. Release the memory used by the results.
```
fiftyoneDegreesResultsIpiFree(results);
```

8. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```

Expected output:
```
...
Ipv4 Address: 185.28.167.77
   AccuracyRadius: 53.576283,-2.328108
   RegisteredCountry: 0.000000,0.000000
   IpRangeEnd: 0.000000,0.000000
   IpRangeStart: 0.000000,0.000000

Ipv6 Address: 2001:4860:4860::8888
   AccuracyRadius: 0.000000,0.000000
   RegisteredCountry: 0.000000,0.000000
   IpRangeEnd: 0.000000,0.000000
   IpRangeStart: 0.000000,0.000000
...
```

*/

#ifdef _DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include <stdio.h>
#include "../../../src/ipi.h"
#include "../../../src/fiftyone.h"

static const char* dataDir = "ip-intelligence-data";

static const char* dataFileName = "51Degrees-LiteV41.ipi";

static void printCoordinateValues(ResultsIpi* results) {
	uint32_t i;
	const char* propertyName;
#	ifdef _MSC_VER
	UNREFERENCED_PARAMETER(results);
	UNREFERENCED_PARAMETER(i);
	UNREFERENCED_PARAMETER(propertyName);
#	endif
	// fiftyoneDegreesCoordinate coordinate;
	// DataSetBase* dataSet = (DataSetBase*)results->b.dataSet;
	// for (i = 0; i < dataSet->available->count; i++) {
	// 	propertyName = STRING(
	// 		dataSet->available->items[i].name.data.ptr);
	// 	coordinate = getPropertyValueAsCoordinate(results, propertyName);
	// 	printf("   %s: %f,%f\n",
	// 		propertyName,
	// 		coordinate.lat,
	// 		coordinate.lon);
	// }
}

/**
 * Reports the status of the data file initialization.
 */
static void reportStatus(StatusCode status,
	const char* fileName) {
	const char* message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

void fiftyoneDegreesIpiStronglyTyped(
	const char* dataFilePath,
	ConfigIpi* config) {
	EXCEPTION_CREATE;
	ResourceManager manager;

	// Set the properties to be returned for each IP Address.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "IpRangeStart,IpRangeEnd,RegisteredCountry,AccuracyRadius";

	// Initialise the manager for IP intelligence.
	StatusCode status = IpiInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
#ifndef TEST
		fgetc(stdin);
#endif
		return;
	}

	// Create a results instance to store and process IP Addresses.
	ResultsIpi* results = ResultsIpiCreate(&manager);

	// Ipv4 Address.
	const char* ipv4Address = "185.28.167.77";

	// Ipv6 Address.
	const char* ipv6Address = "2001:4860:4860::8888";


	printf("Starting Getting Started Example.\n");

	// Carries out a match for a Ipv4 Address.
	printf("\nIpv4 Address: %s\n", ipv4Address);
	ResultsIpiFromIpAddressString(
		results,
		ipv4Address,
		strlen(ipv4Address),
		exception);
	printCoordinateValues(results);

	// Carries out a match for a Ipv6 Address.
	printf("\nIpv6 Address: %s\n", ipv6Address);
	ResultsIpiFromIpAddressString(
		results,
		ipv6Address,
		strlen(ipv6Address),
		exception);
	printCoordinateValues(results);

	// Ensure the results are freed to avoid memory leaks.
	ResultsIpiFree(results);

	// Free the resources used by the manager.
	ResourceManagerFree(&manager);
}

#ifndef TEST

int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	ConfigIpi config = IpiInMemoryConfig;
	char dataFilePath[FILE_MAX_PATH];
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
		config = IpiInMemoryConfig;
	}

#ifdef _DEBUG
#ifndef _MSC_VER
#else
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif

	fiftyoneDegreesIpiStronglyTyped(
		dataFilePath,
		&config);

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
