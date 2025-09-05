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

#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/ProcIpi.c"

class ExampleTestProc : public ExampleIpIntelligenceTest {
public:
    void run(fiftyoneDegreesConfigIpi config) {
        string line;
        int lines = 0;

        // Associate the file of Ip Addresses with stdin.
        freopen(this->ipAddressFilePath.c_str(), "r", stdin);

        // Capture stdout for the test.
        testing::internal::CaptureStdout();

        // Start to process Ip Addresses.
        int count =
            fiftyoneDegreesProcIpiRun(dataFilePath.c_str(), "RegisteredCountry", &config);

        // Get the output from the processing.
        stringstream output = stringstream(testing::internal::GetCapturedStdout());

        // Loop through the output lines.
        while (getline(output, line)) {
            if (!line.empty()) {
                lines++;
            }
        }

        // Check lines in and out are the same.
        EXPECT_EQ(count, lines) << "Same number of IP addresses in and out required";
    }
};

EXAMPLE_TESTS(ExampleTestProc)