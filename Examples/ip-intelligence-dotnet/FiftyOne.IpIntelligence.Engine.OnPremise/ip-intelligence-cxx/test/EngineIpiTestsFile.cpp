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

#include "EngineIpIntelligenceTests.hpp"

class EngineIpiTestsFile : public EngineIpIntelligenceTests {
public:
	EngineIpiTestsFile(
		ConfigIpi *config,
		RequiredPropertiesConfig *properties,
		const char *dataDirectory,
		const char **ipiFileNames,
		int ipiFileNamesLength,
		const char *ipAddressesFileName)
		: EngineIpIntelligenceTests(
			config, 
			properties, 
			dataDirectory,
			ipiFileNames, 
			ipiFileNamesLength,
			ipAddressesFileName) {
	};
	void SetUp() override {
		EngineIpIntelligenceTests::SetUp();
		engine = new EngineIpi(fullName, config, requiredProperties);
	};
	void TearDown() override {
		EngineIpIntelligenceTests::TearDown();
	};
	void reload() { 
		reloadFile();
	}
	void metaDataReload() { 
		verifyMetaDataReload(engine);
	};
	void size() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		EXPECT_GT(fiftyoneDegreesIpiSizeManagerFromFile(
			&config->getConfig(),
			requiredProperties->getConfig(), 
			fullName,
			exception),
			(size_t)0) << "Size method should always return more than 0 "
			"bytes";
		if (FIFTYONE_DEGREES_EXCEPTION_FAILED) {
			FAIL() << "Getting the manager size failed with: " <<
				fiftyoneDegreesExceptionGetMessage(exception);
		}
	}
};

ENGINE_FILE_TESTS(Ipi)