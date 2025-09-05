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
#include "../exceptions.h"

// Header names
const char* testEvidenceHeaders_Multiple[] = {
	"Red",
	"Yellow",
	"Green",
	"Blue",
	"Brown",
	"Black",
	"White"
};

// Class that sets up the headers test structure when there are multiple 
// headers. This stops us having to  do it multiple times.
class EvidenceWithHeadersTest_MultipleHeaders : public Evidence
{
protected:
	StringCollection *strings;
	int count;
	fiftyoneDegreesHeaders *headers;

	void SetUp() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		Evidence::SetUp();
		count = sizeof(testEvidenceHeaders_Multiple) / sizeof(const char*);
		strings = new StringCollection(testEvidenceHeaders_Multiple, count);
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
// multiple expected headers matches the expected item.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_mh_se_sm[2];
int intersection_mh_se_sm_count = 0;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
bool evidenceHeaderIntersection_mh_se_sm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_mh_se_sm[intersection_mh_se_sm_count] = *pair;
	intersection_mh_se_sm_count++;
	return true;
}
#ifdef _MSC_VER
#pragma warning(pop)
#pragma warning(disable : 4100)
#endif

TEST_F(EvidenceWithHeadersTest_MultipleHeaders, Intersection_mh_se_sm) {
	CreateEvidence(1);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Black",
		"Value");

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		headers,
		evidenceHeaderIntersection_mh_se_sm);

	ASSERT_EQ(1, result);
	ASSERT_STREQ("Black", intersection_mh_se_sm[0].item.key);
	ASSERT_STREQ("Value", (char*)intersection_mh_se_sm[0].item.value);
}


//------------------------------------------------------------------
// Check that the intersection of multiple evidence and multiple
// expected headers matches the expected items when there
// are multiple matches.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_mh_me_mm[2];
int intersection_multiple_mh_me_mm_count = 0;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
bool evidenceHeaderIntersection_mh_me_mm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_mh_me_mm[intersection_multiple_mh_me_mm_count] = *pair;
	intersection_multiple_mh_me_mm_count++;
	return true;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

TEST_F(EvidenceWithHeadersTest_MultipleHeaders, Intersection_mh_me_mm) {
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
		evidenceHeaderIntersection_mh_me_mm);

	ASSERT_EQ(2, result);
	EXPECT_STREQ("Black", intersection_mh_me_mm[0].item.key);
	EXPECT_STREQ("Value", (char*)intersection_mh_me_mm[0].item.value);
	EXPECT_STREQ("Red", intersection_mh_me_mm[1].item.key);
	EXPECT_STREQ("Value2", (char*)intersection_mh_me_mm[1].item.value);
}

//------------------------------------------------------------------
// Check that the intersection of multiple evidence and multiple
// expected headers matches the expected items when there are no 
// matches.
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_mh_me_nm[2];
int intersection_mh_me_nm_count = 0;
bool evidenceHeaderIntersection_mh_me_nm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	if (fiftyoneDegreesHeaderGetIndex(
		(fiftyoneDegreesHeaders*)state,
		pair->item.key,
		pair->item.keyLength) >= 0) {
		intersection_mh_me_nm[intersection_mh_me_nm_count] = *pair;
		intersection_mh_me_nm_count++;
	}
	return true;
}
TEST_F(EvidenceWithHeadersTest_MultipleHeaders, Intersection_mh_me_nm) {
	CreateEvidence(2);
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Lilac",
		"Value");
	fiftyoneDegreesEvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"Indigo",
		"Value2");

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		headers,
		evidenceHeaderIntersection_mh_me_nm);
	
	ASSERT_EQ(2, result);
	ASSERT_EQ(0, intersection_mh_me_nm_count);
}

//------------------------------------------------------------------
// Check that the intersection of no evidence and multiple
// expected headers functions as expected
//------------------------------------------------------------------
fiftyoneDegreesEvidenceKeyValuePair intersection_mh_ne_nm[2];
int intersection_mh_ne_nm_count = 0;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
bool evidenceHeaderIntersection_mh_ne_nm(void *state,
	fiftyoneDegreesEvidenceKeyValuePair *pair) {
	intersection_mh_ne_nm[intersection_mh_ne_nm_count] = *pair;
	intersection_mh_ne_nm_count++;
	return true;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

TEST_F(EvidenceWithHeadersTest_MultipleHeaders, Intersection_mh_ne_nm) {
	CreateEvidence(2);

	int result = fiftyoneDegreesEvidenceIterate(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		headers,
		evidenceHeaderIntersection_mh_ne_nm);

	ASSERT_EQ(0, result);
}