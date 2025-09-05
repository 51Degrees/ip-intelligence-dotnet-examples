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
@example IpIntelligence/GettingStarted.c
Getting started example of using 51Degrees IP intelligence.

The example shows how to use 51Degrees on-premise IP intelligence to
determine the country of a given IP address.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/C/IpIntelligence/GettingStarted.c).

@include{doc} example-require-datafile-ipi.txt

@include{doc} example-how-to-run-ipi.txt

In detail, the example shows how to

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

4. Process a single IP address string to retrieve the values associated
with the IP address for the selected properties.
```
fiftyoneDegreesResultsIpiFromIpAddressString(
	results,
	ipv4Address,
	strlen(ipv4Address),
	exception);
```

5. Extract the value of a property as a string from the results.
```
fiftyoneDegreesResultsIpiGetValuesString(
	results,
	propertyName,
	valueBuffer,
	sizeof(valueBuffer),
	"|",
	exception);
```

6. Release the memory used by the results.
```
fiftyoneDegreesResultsIpiFree(results);
```

7. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```

Expected output:
```
...
Ipv4 Address: 185.28.167.77
IpRangeStart: "185.28.167.0":1
IpRangeEnd: "185.28.167.127":1
AccuracyRadius: "147033":1
RegisteredCountry: "GB":1
RegisteredName: "CUSTOMERS-Subnet9":1
Longitude: "5.789971617786187":1
Latitude: "43.828547013763846":1
Areas: "POLYGON((6.839197973570971 43.240760521256142,6.800744651631215 43.227027191991944,6.762291329691458 43.21878719
4433425,6.718344676046022 43.216040528580585,6.679891354106266 43.218787194433425,6.635944700460829 43.227027191991944,6
.602984710226752 43.240760521256142,5.619678334910123 43.682973723563343,5.586718344676046 43.702200384533221,5.55925168
6147649 43.726920377208778,5.53727835932493 43.754387035737174,5.520798364207892 43.784600360118411,5.515305032502212 43
.814813684499647,5.520798364207892 43.847773674733723,5.531785027619251 43.877986999114967,5.54826502273629 43.908200323
496203,5.575731681264687 43.93292031617176,5.614185003204444 43.952146977141638,5.630664998321482 43.963133640552996,5.6
526383251442 43.974120303964355,5.691091647083956 43.987853633228553,5.735038300729393 43.998840296639912,5.784478286080
508 44.001586962492752,7.234717856379895 43.990600299081393,7.27866451002533 43.987853633228553,7.328104495376445 43.976
866969817195,7.366557817316203 43.957640308847317,7.525864436780908 43.867000335703601,7.558824427014985 43.845027008880
884,7.586291085543382 43.817560350352487,7.60277108066042 43.787347025971251,7.61375774407178 43.754387035737174,7.61375
774407178 43.724173711355938,7.60277108066042 43.691213721121862,7.586291085543382 43.661000396740626,7.558824427014985
43.633533738212229,7.49839777825251 43.584093752861108,7.459944456312754 43.556627094332711,6.938077944273202 43.2874538
40754416,6.927091280861843 43.281960509048737,6.839197973570971 43.240760521256142,6.839197973570971 43.240760521256142)
)":1

Ipv6 Address: fdaa:bbcc:ddee:0:995f:d63a:f2a1:f189
IpRangeStart: "fc00:0000:0000:0000:0000:0000:0000:0000":1
IpRangeEnd: "fdff:ffff:ffff:ffff:ffff:ffff:ffff:ffff":1
AccuracyRadius: "-1":1
RegisteredCountry: "Unknown":1
RegisteredName: "IANA-V6-ULA":1
Longitude: "0":1
Latitude: "0.00274666585284":1
Areas: "POLYGON EMPTY":1
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
#include "../../../src/ipi_weighted_results.h"

static const char* dataDir = "ip-intelligence-data";

static const char* dataFileName = "51Degrees-LiteV41.ipi";

static char valueBuffer[4096] = "";

static const char* getPropertyValueAsString(
	ResultsIpi* results,
	const char* propertyName) {
	EXCEPTION_CREATE;
	ResultsIpiGetValuesString(
		results,
		propertyName,
		valueBuffer,
		sizeof(valueBuffer),
		"|",
		exception);
	EXCEPTION_THROW;
	return valueBuffer;
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

static void printPropertyValueFromResults(ResultsIpi *results) {
	if (results != NULL && results->count > 0) {
		printf("- IpRangeStart: %s\n", getPropertyValueAsString(results, "IpRangeStart"));
		printf("- IpRangeEnd: %s\n", getPropertyValueAsString(results, "IpRangeEnd"));
		printf("- AccuracyRadius: %s\n", getPropertyValueAsString(results, "AccuracyRadius"));
		printf("- RegisteredCountry: %s\n", getPropertyValueAsString(results, "RegisteredCountry"));
		printf("- RegisteredName: %s\n", getPropertyValueAsString(results, "RegisteredName"));
		printf("- Longitude: %s\n", getPropertyValueAsString(results, "Longitude"));
		printf("- Latitude: %s\n", getPropertyValueAsString(results, "Latitude"));
		printf("- Areas: %s\n", getPropertyValueAsString(results, "Areas"));
		// printf("- Mcc: %s\n", getPropertyValueAsString(results, "Mcc"));
	}
	else {
		printf("No results.");
	}
}

static void printPropertyValueFromResults2(ResultsIpi *results) {
	printf("\n(Results using ResultsIpiGetValuesCollection):\n");
	if (results != NULL && results->count > 0) {
		EXCEPTION_CREATE;
		fiftyoneDegreesWeightedValuesCollection collection =
			fiftyoneDegreesResultsIpiGetValuesCollection(
				results,
				NULL,
				0,
				NULL,
				exception);
		if (EXCEPTION_OKAY) {
			const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;
			for (uint32_t i = 0, n = collection.itemsCount; i < n; i++) {
				const fiftyoneDegreesWeightedValueHeader * const nextHeader =
					collection.items[i];
				const int requiredPropertyIndex =
					nextHeader->requiredPropertyIndex;
				const String * const propNameRaw = (const String *)(
					dataSet->b.b.available->items[requiredPropertyIndex]
					.name.data.ptr);
				const char * const propName = &propNameRaw->value;
				const double weight =
					(float)(nextHeader->rawWeighting) / (float)(UINT16_MAX);
				switch (nextHeader->valueType) {
					case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER: {
						printf("- [%s] (int) <x%g> %d\n",
							propName,
							weight,
							((const fiftyoneDegreesWeightedInt*)nextHeader)
							->value);
						break;
					}
					case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DOUBLE: {
						printf("- [%s] (double) <x%g> %g\n",
							propName,
							weight,
							((const fiftyoneDegreesWeightedDouble*)nextHeader)
							->value);
						break;
					}
					case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_BOOLEAN: {
						printf("- [%s] (bool) <x%g> %d\n",
							propName,
							weight,
							((const fiftyoneDegreesWeightedBool*)nextHeader)
							->value);
						break;
					}
					case FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE: {
						printf("- [%s] (byte) <x%g> %d\n",
							propName,
							weight,
							((const fiftyoneDegreesWeightedByte*)nextHeader)
							->value);
						break;
					}
					case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING:
					default: {
						printf("- [%s] (string) <x%g> %s\n",
							propName,
							weight,
							((const fiftyoneDegreesWeightedString*)nextHeader)
							->value);
						break;
					}
				}
			}
			fiftyoneDegreesWeightedValuesCollectionRelease(&collection);
		}
	}
	else {
		printf("No results.");
	}
}

void fiftyoneDegreesIpiGettingStarted(
	const char* dataFilePath,
	ConfigIpi* config) {
	ResourceManager manager;
	EXCEPTION_CREATE;

	// Set the properties to be returned for each ip
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "IpRangeStart,IpRangeEnd,AccuracyRadius,RegisteredCountry,RegisteredName,Longitude,Latitude,Areas";

	StatusCode status = IpiInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
	EXCEPTION_THROW;
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
#ifndef TEST
		fgetc(stdin);
#endif
		return;
	}

	// Create a results instance to store and process ip
	ResultsIpi* results = ResultsIpiCreate(&manager);

	// an ipv4 address string
	const char* ipv4Address = "185.28.167.77";

	// an ipv6 address string
	const char* ipv6Address = "fdaa:bbcc:ddee:0:995f:d63a:f2a1:f189";

	printf("Starting Getting Started Example.\n");

	// Carries out a match for the ipv4 address
	printf("\nIpv4 Address: %s\n\n", ipv4Address);
	ResultsIpiFromIpAddressString(
		results,
		ipv4Address,
		strlen(ipv4Address),
		exception);
	if (EXCEPTION_FAILED) {
		printf("%s\n", ExceptionGetMessage(exception));
	}
	printPropertyValueFromResults(results);
	printPropertyValueFromResults2(results);

	// Carries out a match for the ipv6 address
	printf("\nIpv6 Address: %s\n\n", ipv6Address);
	ResultsIpiFromIpAddressString(
		results,
		ipv6Address,
		strlen(ipv6Address),
		exception);
	if (EXCEPTION_FAILED) {
		printf("%s\n", ExceptionGetMessage(exception));
	}
	printPropertyValueFromResults(results);
	printPropertyValueFromResults2(results);

	// Ensure the results are freed to avoid memory leaks.
	ResultsIpiFree(results);

	// Free the resources used by the manager
	ResourceManagerFree(&manager);
}

#ifndef TEST

int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	// ConfigIpi config = IpiDefaultConfig;
	ConfigIpi config = IpiInMemoryConfig;
	// ConfigIpi config = IpiHighPerformanceConfig;
	// ConfigIpi config = IpiLowMemoryConfig;
	// ConfigIpi config = IpiBalancedConfig;
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


	fiftyoneDegreesIpiGettingStarted(
		dataFilePath,
		&config);

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