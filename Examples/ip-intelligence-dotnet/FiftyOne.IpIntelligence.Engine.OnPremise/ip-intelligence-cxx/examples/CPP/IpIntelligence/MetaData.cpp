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
@example IpIntelligence/MetaData.cpp
Meta data example of using 51Degrees IP intelligence.

The example shows how to retrieve meta data from 51Degrees on-premise
IP intelligence. This feature can be used to get information such as
the category that a property belons to or the possible values a property
can have.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/tree/main/examples/CPP/IpIntelligence/MetaData.cpp).

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

3. Fetch a collection containing the meta data of all the properties in the
engine's data set.
```
using namespace FiftyoneDegrees;

Common::Collection<string, Common::PropertyMetaData> *properties =
	engine->getMetaData()->getProperties();
```

4. Iterate over all the properties and print the name and description. Note
that getting a property meta data instance will return a new instance which
holds no reference to the engine nor any of its internals. Therefore it is the
responsibility of the caller to delete it once it is no longer needed.
```
using namespace FiftyoneDegrees;

for (uint32_t i = 0; i < properties->getSize(); i++){
	Common::PropertyMetaData *property = properties->getByIndex(i);
	cout << property->getName() << " - " << property->getDescription() << "\n";
	delete property;
}
```

5. Release the memory used by the properties collection, and its reference to
the engine's underlying data set.
```
delete properties;
```

7. Finally release the memory used by the engine.
```
delete engine;
```

Expected output:
```
...
ConnectivityTypes - The connectivity type
MCC - Mobile country code. Only valid if type = mobile
MNC - Mobile network code. Only valid if type = mobile
...
Get specific property value
IpRangeStart - 0.0.0.0 -
...
```

*/

namespace FiftyoneDegrees {
	namespace Examples {
		namespace IpIntelligence {
			/**
			 * IP Intelligence Meta Data Example.
			 */
			class MetaDataExample : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				MetaDataExample(string dataFilePath)
					: ExampleBase(dataFilePath)
				{};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					cout << "Starting MetaData Example.\n";
					auto const properties = std::unique_ptr<Collection<string, PropertyMetaData>>(engine->getMetaData()->getProperties());
					for (uint32_t i = 0; i < properties->getSize(); i++){
						auto const property = std::unique_ptr<PropertyMetaData>(properties->getByIndex(i));
						if (!property) {
							continue;
						}
						auto const propName = property->getName();
						auto const propDesc = property->getDescription();
						cout << propName << " - " << propDesc << "\n";
						auto const defaultValue = std::unique_ptr<ValueMetaData>(
							engine->getMetaData()->getDefaultValueForProperty(property.get()));
						if (defaultValue) {
							auto const defName = defaultValue->getName();
							auto const defDesc = defaultValue->getDescription();
							cout << "   -> default value = '" << defName << "', description = '" << defDesc << "'\n";
						}
					}


					cout << "\n\nGet specific property value\n";
					auto const values = std::unique_ptr<Collection<ValueMetaDataKey, ValueMetaData>>(engine->getMetaData()->getValues());
					auto const value = std::unique_ptr<ValueMetaData>(values->getByKey(ValueMetaDataKey("IpRangeStart", "0.0.0.0")));
					if (value != nullptr) {
						cout << value->getKey().getPropertyName() << " - " << value->getName() 
							<< " - " << value->getDescription() << "\n";
					}
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

	auto const metaData = std::make_unique<MetaDataExample>(dataFilePath);
	metaData->run();

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
