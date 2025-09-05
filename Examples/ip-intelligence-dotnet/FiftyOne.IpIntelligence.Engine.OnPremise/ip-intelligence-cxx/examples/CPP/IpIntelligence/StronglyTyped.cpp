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

#include <string>
#include <iostream>
#include "../../../src/EngineIpi.hpp"
#include "ExampleBase.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;
using namespace FiftyoneDegrees::Examples::IpIntelligence;

/**
@example IpIntelligence/StronglyTyped.cpp
Strongly typed example of using 51Degrees IP intelligence.

The example shows how to extract the strongly typed value from the
returned results of the 51Degrees on-premise IP intelligence.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/CPP/IpIntelligence/StronglyTyped.cpp).

@include{doc} example-require-datafile-ipi.txt

@include{doc} example-how-to-run-ipi.txt

In detail, the example shows how to:

1. Specify the name of the data file, properties the data set should be
initialised with, and the configuration.
```
using namespace FiftyoneDegrees;

string fileName = "51Degrees-V4.1.ipi";
string propertiesString = "IpRangeStart,IpRangeEnd,"
	"RegisteredCountry,CityName,AccuracyRadius";
Common::RequiredPropertiesConfig *properties =
	new Common::RequiredPropertiesConfig(&propertiesString);
IpIntelligence::ConfigIpi *config =
	new IpIntelligence::ConfigIpi();
```

2. Construct a new engine from the specified data file with the required
properties and the specified configuration.
```
using namespace FiftyoneDegrees;

IpIntelligence::EngineIpi *engine =
	new IpIntelligence::EngineIpi(
		dataFilePath,
		config,
		properties);
```

3. Create a evidence instance and add a single IP address byte array to be
processed.
```
using namespace FiftyoneDegrees;

IpIntelligence::EvidenceIpi *evidence =
	new IpIntelligence::EvidenceIpi();
evidence->operator[]("ipv4.ip") = ipAddress;
```

4. Process the evidence using the engine to retrieve the values associated
with the IP address for the selected properties.
```
using namespace FiftyoneDegrees;

IpIntelligence::ResultsIpi *results = engine->process(evidence);
```

5. Extract the value of a property as a weighted pair of floats from the results.
```
Value<vector<WeightedValue<pair<float, float>>>> value = 
	results->getValuesAsWeightedCoordinateList("AccuracyRadius");
for (WeightedValue<pair<float, float>> w : value.getValue()) {
	cout << "   AccuracyRadius: " <<
		w.getValue().first <<
		"," <<
		w.getValue().second << "\n";
}
```

6. Release the memory used by the results and the evidence.
```
delete results;
delete evidence;
```

7. Finally release the memory used by the engine.
```
delete engine;
```

Expected output:
```
...
Ipv4 Address: 185.28.167.77
   AccuracyRadius: 53.5763,-2.32811

Ipv6 Address: 2001:4860:4860::8888
   AccuracyRadius: 0,0
...
```

 */

namespace FiftyoneDegrees {
	namespace Examples {
		namespace IpIntelligence {
			/**
			 * IP Intelligence Strongly Typed Example
			 */
			class StronglyTyped : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				StronglyTyped(string dataFilePath)
					: ExampleBase(dataFilePath)
				{};

				static void printResults(const std::unique_ptr<ResultsIpi> &results) {
					const std::vector doubleProps = { "Longitude", "Latitude" };
					for (auto const &nextProperty : doubleProps) {
						Value<vector<WeightedValue<double>>> propValue
							= results->getValuesAsWeightedDoubleList(nextProperty);
						if (propValue.hasValue()) {
							cout << "   " << nextProperty << " (" << propValue.getValue().size() << "):\n";
							for (auto const &nextValue : propValue.getValue()) {
								cout << "    - " << nextValue.getValue() << " x" << nextValue.getWeight() << "\n";
							}
						} else {
							cout << "   " << nextProperty << " -- " << propValue.getNoValueMessage() << "\n";
						}
					}
					{
						Value<vector<WeightedValue<string>>> areasValue
							= results->getValuesAsWeightedStringList("Areas");
						if (areasValue.hasValue()) {
							cout << "   Areas (" << areasValue.getValue().size() << "):\n";
							for (auto const &nextValue : areasValue.getValue()) {
								cout << "    - '" << nextValue.getValue() << "' x" << nextValue.getWeight() << "\n";
							}
						} else {
							cout << "   Areas -- " << areasValue.getNoValueMessage() << "\n";
						}
					}
				}

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() override {
					std::unique_ptr<ResultsIpi> results;

					// Create an evidence instance to store and process Ip Addresses.
					auto const evidence = std::make_unique<EvidenceIpi>();

					cout << "Starting Strongly Typed Example.\n";

					// Carries out a match for a ipv4 address.
					cout << "\nIpv4 Address: " << ipv4Address << "\n";
					(*evidence)["query.client-ip-51d"] = ipv4Address;
					results = std::unique_ptr<ResultsIpi>(engine->process(evidence.get()));
					printResults(results);

					// Carries out a match for a ipv6 address.
					cout << "\nIpv6 Address: " << ipv6Address << "\n";
					(*evidence)["query.client-ip-51d"] = ipv6Address;
					results = std::unique_ptr<ResultsIpi>(engine->process(evidence.get()));
					printResults(results);
				}
			};
		}
	}
}

int main(int argc, char* argv[]) {
	fiftyoneDegreesStatusCode status = FIFTYONE_DEGREES_STATUS_SUCCESS;
	char dataFilePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = fiftyoneDegreesFileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
	}
	if (status != FIFTYONE_DEGREES_STATUS_SUCCESS) {
		ExampleBase::reportStatus(status, dataFileName);
#ifndef TEST
		fgetc(stdin);
#endif
		return 1;
	}


#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif

	auto const stronglyTyped = std::make_unique<StronglyTyped>(dataFilePath);
	stronglyTyped->run();

#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#else
	printf("Log file is %s\r\n", dmalloc_logpath);
#endif
#endif

	// Wait for a character to be pressed.
#ifndef TEST
	fgetc(stdin);
#endif

	return 0;
}
