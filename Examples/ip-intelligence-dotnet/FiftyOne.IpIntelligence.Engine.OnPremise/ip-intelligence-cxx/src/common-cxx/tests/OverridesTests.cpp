/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2023 51 Degrees Mobile Experts Limited, Davidson House,
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

#include "pch.h"
#include "../overrides.h"
#include "../string.h"

#ifdef _MSC_VER
// This is a mock implementation of the method
#pragma warning (disable: 4100)
#endif
static void overrideProfileId(void *state, uint32_t profileId) {
	(*(int*)state)++;
}
#ifdef _MSC_VER
#pragma warning (default: 4100)
#endif
// This test whether the ProfileIds evidence key is treated
// as case insensitive.
// Since the test only targets the case insensitivity aspect
// it does not check how the process ids string is processed,
// but whether the evidence is picked up.
TEST(OverrideProfileIdsTests, CaseSensitivity) {
	fiftyoneDegreesEvidenceKeyValuePairArray* evidence =
		fiftyoneDegreesEvidenceCreate(1);

	// Test against upper cases
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_QUERY,
		"51D_PROFILEIDS",
		"11-22-33-44");
	int state = 0;
	fiftyoneDegreesOverrideProfileIdMethod override
		= overrideProfileId;
	fiftyoneDegreesOverrideProfileIds(evidence, &state, override);
	EXPECT_EQ(4, state) <<
		"Case insentivity should be honoured for ProfileIds with upper case.";

	// Test against lower cases
	evidence->count = 0;
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_QUERY,
		"51d_profileids",
		"55-66-77-88-99");

	state = 0;
	fiftyoneDegreesOverrideProfileIds(evidence, &state, override);
	EXPECT_EQ(5, state) <<
		"Case insentivity should be honoured for ProfileIds with lower case.";
    
    fiftyoneDegreesEvidenceFree(evidence);
}

// Check if overrides are set correctly
TEST(OverrideValuesResetTests, Positive) {
	uint32_t i, capacity = 10;
	fiftyoneDegreesOverrideValueArray *overrides = 
		fiftyoneDegreesOverrideValuesCreate(capacity);

	for (i = 0; i < capacity; i++) {
		fiftyoneDegreesOverridesAdd(overrides, i, "TestValue");
		EXPECT_EQ(i + 1, overrides->count);
		EXPECT_EQ(
			strlen("TestValue") + sizeof(fiftyoneDegreesString),
			overrides->items[i].string.allocated);
		EXPECT_STREQ(
			"TestValue",
			FIFTYONE_DEGREES_STRING(overrides->items[i].string.ptr));
	}

	fiftyoneDegreesOverrideValuesReset(overrides);
	EXPECT_EQ(0, overrides->count) << "Failed to reset overrides count.";
	for (i = 0; i < capacity; i++) {
		EXPECT_EQ(0, overrides->items[i].string.used);
		EXPECT_EQ(0, overrides->items[i].requiredPropertyIndex);
		EXPECT_STREQ("", FIFTYONE_DEGREES_STRING(overrides->items[i].string.ptr));
	}

	fiftyoneDegreesOverrideValuesFree(overrides);
}

// Check if test doest not crash if reset a null pointer.
TEST(OverrideValuesResetTests, Negative) {
	fiftyoneDegreesOverrideValuesReset(NULL);
}
