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

#ifndef FIFTYONE_DEGREES_EXAMPLE_IP_INTELLIGENCE_TESTS_HPP
#define FIFTYONE_DEGREES_EXAMPLE_IP_INTELLIGENCE_TESTS_HPP

#include "../src/common-cxx/file.h"
#include "../src/common-cxx/tests/ExampleTests.hpp"
#include "Constants.hpp"

class ExampleIpIntelligenceTest : public ExampleTests {
public:
    ExampleIpIntelligenceTest();

    ExampleIpIntelligenceTest(
        const char * const *dataFileNames,
        int dataFileNamesLength,
        const char *ipAddressFileName,
        const char *evidenceFileName);

protected:
    string dataFilePath;
    string ipAddressFilePath;
    string evidenceFilePath;
    const char *requiredProperties;
};


#define EXAMPLE_TESTS(c)                                        \
    TEST_F(c, Default) {                                             \
        if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {     \
            run(fiftyoneDegreesIpiDefaultConfig);                      \
        }                                                              \
    }                                                                \
    TEST_F(c, BalancedTemp) {                                        \
        if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {     \
            run(fiftyoneDegreesIpiBalancedTempConfig);                 \
        }                                                              \
    }                                                                \
    TEST_F(c, Balanced) {                                            \
        if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {     \
            run(fiftyoneDegreesIpiBalancedConfig);                     \
        }                                                              \
    }                                                                \
    TEST_F(c, LowMemory) {                                           \
        if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {     \
            run(fiftyoneDegreesIpiLowMemoryConfig);                    \
        }                                                              \
    }                                                                \
    TEST_F(c, HighPerformance) {                                     \
        run(fiftyoneDegreesIpiHighPerformanceConfig);                \
    }                                                                \
    TEST_F(c, InMemory) { run(fiftyoneDegreesIpiInMemoryConfig); }

#endif