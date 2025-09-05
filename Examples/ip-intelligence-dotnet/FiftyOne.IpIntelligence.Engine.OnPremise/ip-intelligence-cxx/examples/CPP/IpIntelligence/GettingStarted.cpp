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
@example IpIntelligence/GettingStarted.cpp
Getting started example of using 51Degrees IP intelligence.

The example shows how to user 51Degrees on-premise IP intelligence to
determine the country of a given IP address.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/CPP/IpIntelligence/GettingStarted.cpp).

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

3. Create a evidence instance and add a single IP intelligence string to be
processed.
```
using namespace FiftyoneDegrees;

IpIntelligence::EvidenceIpi *evidence =
	new IpIntelligence::EvidenceIpi();
evidence->operator[]("ipiv4.ip") = ipv4;
```

4. Process the evidence using the engine to retrieve the values associated
with the User-Agent for the selected properties.
```
using namespace FiftyoneDegrees;

IpIntelligence::ResultsIpi *results = engine->process(evidence);
```

5. Extract the value of a property as a weighted string from the results.
```
Value<vector<WeightedValue<string>>> value = results->getValuesAsWeightedStringList("RegisteredCountry");
for (WeightedValue<string> w : value.getValue()) {
	cout << "   Country Code: " <<
		w.getValue() <<
		", Percentage: " <<
		w.getWeight() << "\n";
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
   Country Code: gb, Percentage: 100%

Ipv6 Address: 2001:4860:4860::8888
   Country Code: ZZ, Percentage: 100%
...
```

*/

namespace FiftyoneDegrees {
	namespace Examples {
		namespace IpIntelligence {
			/**
			 * IP Intelligence Getting Started Example.
			 */
			class GettingStarted : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				GettingStarted(string dataFilePath)
					: ExampleBase(dataFilePath) {
				};

				/**
				 * Check that the value is populated before using the result.
				 */
				static void printResults(const std::unique_ptr<ResultsIpi> &results) {
					const std::vector props {
						"RegisteredName",
						"RegisteredCountry",
						"IpRangeStart",
						"IpRangeEnd",
						"Latitude",
						"Longitude",
						"Areas",
					};
					for (auto const &nextProp : props) {
						Value<vector<WeightedValue<string>>> value
							= results->getValuesAsWeightedStringList(nextProp);
						if (value.hasValue()) {
							for (const WeightedValue<string>& w : value.getValue()) {
								cout << "   " << nextProp << ": " <<
									w.getValue() <<
									", Percentage: " <<
									w.getWeight() * 100 << "%\n";
							}
						} else {
							cout << "   " << nextProp << ": " <<
								value.getNoValueMessage() << "\n";
						}
					}
				}

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() override {
					std::unique_ptr<ResultsIpi> results;

					// Create an evidence instance to store and process
					// IP Address.
					const auto evidence = make_unique<EvidenceIpi>();

					cout << "Starting Getting Started Example.\n";

					// Carries out a match for a ipv4 address.
					cout << "\nIpv4 Address: " << ipv4Address << "\n";
					(*evidence)["query.client-ip"] = ipv4Address;
					results = std::unique_ptr<ResultsIpi>(engine->process(evidence.get()));
					printResults(results);

					// Carries out a match for a ipv4 address.
					cout << "\nIpv6 Address: " << ipv6Address << "\n";
					(*evidence)["query.client-ip"] = ipv6Address;
					results = std::unique_ptr<ResultsIpi>(engine->process(evidence.get()));
					printResults(results);
				}
			};
		}
	}
}

int main(int argc, char *argv[]) {
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

	auto const gettingStarted = std::make_unique<GettingStarted>(dataFilePath);
	gettingStarted->run();

#ifdef _DEBUG
	#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#else
	printf("Log file is %s\r\n", dmalloc_logpath);
#endif
#endif

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}
