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
#include "EvidenceTests.hpp"
#include "../evidence.h"
#include "../headers.h"

// Header names
const char** testEvidenceHeaders_None = new const char*[0];

// Class that sets up the headers test structure when there are no headers. 
// This stops us having to do it multiple times.
class EvidenceWithHeadersTest_NoHeader : public Evidence
{
protected:
	StringCollection *strings;
	int count;
	fiftyoneDegreesHeaders *headers;

	void SetUp() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		Evidence::SetUp();
		count = 0;
		strings = new StringCollection(testEvidenceHeaders_None, count);
		headers = fiftyoneDegreesHeadersCreate(
			false,
			strings->getState(),
			getHeaderUniqueId,
			exception);
		FIFTYONE_DEGREES_EXCEPTION_THROW
	}
	void TearDown() {
		fiftyoneDegreesHeadersFree(headers);
		delete strings;
		Evidence::TearDown();
	}
};

// These tests use a naming convention suffix of *h_*e_*m.
// This corresponds to the number of possible headers, 
// evidence and matches between headers and evidence respectively.
// 
// The * can be:
// s = single
// m = multiple
// n = none
//
// e.g. sh_me_sm 
// Means that there is only one header expected, multiple evidence
// is supplied and one is expected to match.

//------------------------------------------------------------------
// Check that the intersection of a single piece of evidence and 
// no expected headers works as expected.
//------------------------------------------------------------------

fiftyoneDegreesEvidenceKeyValuePair intersection_nh_se_nm[2];
int intersection_nh_se_nm_count = 0;

bool evidenceHeaderIntersection_nh_se_nm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	if (fiftyoneDegreesHeaderGetIndex(
		(fiftyoneDegreesHeaders*)state,
		pair->item.key,
		pair->item.keyLength) >= 0) {
		intersection_nh_se_nm[intersection_nh_se_nm_count] = *pair;
		intersection_nh_se_nm_count++;
	}
	return true;
}
TEST_F(EvidenceWithHeadersTest_NoHeader, Intersection_nh_se_nm) {
	CreateEvidence(1);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Red",
		"Value");
	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		headers,
		evidenceHeaderIntersection_nh_se_nm);
	ASSERT_EQ(1, result);
	ASSERT_EQ(0, intersection_nh_se_nm_count);
}

//------------------------------------------------------------------
// Check that the intersection of multiple evidence and no
// expected headers matches the expected items.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_nh_me_nm[2];
int intersection_multiple_nh_me_mm_count = 0;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
bool evidenceHeaderIntersection_nh_me_mm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	if (fiftyoneDegreesHeaderGetIndex(
		(fiftyoneDegreesHeaders*)state,
		pair->item.key,
		pair->item.keyLength) >= 0) {
		intersection_nh_me_nm[intersection_multiple_nh_me_mm_count] = *pair;
		intersection_multiple_nh_me_mm_count++;
	}
	return true;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

TEST_F(EvidenceWithHeadersTest_NoHeader, Intersection_nh_me_nm) {
	CreateEvidence(2);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Black",
		"Value");
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Red",
		"Value2");

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		headers,
		evidenceHeaderIntersection_nh_me_mm);

	ASSERT_EQ(2, result);
	ASSERT_EQ(0, intersection_multiple_nh_me_mm_count);
}

//------------------------------------------------------------------
// Check that the intersection of no evidence and no
// expected headers functions as expected
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_nh_ne_nm[2];
int intersection_nh_ne_nm_count = 0;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
bool evidenceHeaderIntersection_nh_ne_nm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_nh_ne_nm[intersection_nh_ne_nm_count] = *pair;
	intersection_nh_ne_nm_count++;
	return true;
}
#ifdef _MSC_VER
#pragma warning(pop)
#pragma warning(disable : 4100)
#endif

TEST_F(EvidenceWithHeadersTest_NoHeader, Intersection_nh_ne_nm) {
	CreateEvidence(1);

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		headers,
		evidenceHeaderIntersection_nh_ne_nm);

	ASSERT_EQ(0, result);
}