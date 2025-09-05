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
@example IpIntelligence/FindProfiles.c
Getting started example of using 51Degrees IP intelligence.

The example shows how to use 51Degrees on-premise IP intelligence to iterate
over all profiles in the data set which match a specified property value pair.

This feature is supported on normal profiles where the property is not dynamic.
The dynamic properties where this feature is not supported are 'IpRangeStart',
'IpRangeEnd'.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/C/IpIntelligence/FindProfiles.c).

@include{doc} example-require-datafile-ipi.txt

@include{doc} example-how-to-run-ipi.txt

In detail, the example shows how to:

1. Specify the name of the data file and properties the data set should be
initialised with.
```
const char* fileName = argv[1];
fiftyoneDegreesPropertiesRequired properties =
	fiftyoneDegreesPropertiesDefault;
properties.string = "RegisteredCountry";
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

3. Iterate over all the profiles in the data set which match a specified
property value pair.
```
fiftyoneDegreesIpiIterateProfilesForPropertyAndValue(
	manager,
	"RegisteredCountry",
	"ITA",
	&isItaly,
	count,
	exception);
```

4. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```

Expected output:
```
...
There are '1' countries in the data set with code 'it'.
There are '1' countries in the data set with code 'gb'.
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

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

#define CONFIG fiftyoneDegreesIpiInMemoryConfig
// #define CONFIG fiftyoneDegreesIpiHighPerformanceConfig
// #define CONFIG fiftyoneDegreesIpiLowMemoryConfig
// #define CONFIG fiftyoneDegreesIpiBalancedConfig
// #define CONFIG fiftyoneDegreesIpiBalancedTempConfig

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif
static bool count(void* state, Item* item) {
	(*(uint32_t*)state) += 1;
	return true;
}
#ifdef _MSC_VER
#pragma warning (default:4100) 
#endif

void run(ResourceManager* manager) {
	EXCEPTION_CREATE;
	uint32_t isGreatBritian = 0;
	uint32_t isItaly = 0;

	printf("Starting Find Profiles Example.\n\n");

	IpiIterateProfilesForPropertyAndValue(
		manager,
		"RegisteredCountry",
		"it",
		&isItaly,
		count,
		exception);
	EXCEPTION_THROW;
	printf("There are '%d' entries in the data set with code 'it'.\n", isItaly);

	IpiIterateProfilesForPropertyAndValue(
		manager,
		"RegisteredCountry",
		"gb",
		&isGreatBritian,
		count,
		exception);
	EXCEPTION_THROW;
	printf("There are '%d' entries in the data set with code 'gb'.\n",
		isGreatBritian);
}

/**
 * Reports the status of the data file initialization.
 */
static void reportStatus(
	StatusCode status,
	const char* fileName) {
	const char* message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

void fiftyoneDegreesIpiFindProfiles(
	const char* dataFilePath,
	ConfigIpi config) {
	EXCEPTION_CREATE;
	ResourceManager manager;

	// Set the properties to be returned for each IP Address.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "RegisteredCountry";

	// Initialise the manager for IP intelligence.
	StatusCode status = IpiInitManagerFromFile(
		&manager,
		&config,
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

	run(&manager);

	// Free the manager and related data structures.
	ResourceManagerFree(&manager);

#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif
#endif
}

#ifndef TEST

int main(int argc, char* argv[]) {


	StatusCode status = SUCCESS;
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


	fiftyoneDegreesIpiFindProfiles(dataFilePath, CONFIG);

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