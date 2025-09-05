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
#include "Base.hpp"
#include "StringCollection.hpp"
#include "HeadersContainer.hpp"

extern "C" {
#include "../headers.h"
#include "../fiftyone.h"
}

/**
 * Header test class used to test the functionality of header.c using a string
 * collection built from the StringCollection class.
 */
class HeadersTests : public Base {
protected:
    HeadersContainer container;
    Headers *headers = nullptr;
	/**
	 * Calls the base setup method to enable memory leak checking and memory
	 * allocation checking.
	 */
	void SetUp() {
		Base::SetUp();
	}

	/**
	 * Frees the headers structure and string collection if either were
	 * created. Then calls the base teardown method to check for memory leaks
	 * and compare expected and actual memory allocations.
	 */
	void TearDown() {
        container.Dealloc();
        headers = nullptr;
		Base::TearDown();
	}

	/**
	 * Create an headers structure with the specified capacity and a string
	 * collection constructed by the StringCollection class from the list of
	 * header names provided, using the create method in headers.c. The
	 * expected memory allocation is calculated, and the actual memory
	 * allocation is tracked. The structure is freed automatically after each
	 * test, at which point the expected and actual memory allocation is
	 * checked for equality.
	 */
	void CreateHeaders(
		const char** headersList,
		int headersCount,
		bool expectUpperPrefixedHeaders) {
        container.CreateHeaders(headersList, headersCount, expectUpperPrefixedHeaders);
        headers = container.headers;
	}
};

// ----------------------------------------------------------------------
// Check that header collection creation works properly when passed a
// collection containing a single header
// ----------------------------------------------------------------------
const char* testHeaders_Single[] = {
	"Red"
};

TEST_F(HeadersTests, Single) {
	CreateHeaders(
		testHeaders_Single,
		sizeof(testHeaders_Single) / sizeof(const char*),
		false);
	ASSERT_EQ(1, headers->count);
	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	const char* str = headers->items[0].name;
	EXPECT_STREQ("Red", str);
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when passed a
// collection containing multiple headers
// ----------------------------------------------------------------------
const char* testHeaders_Multiple[] = {
	"Red",
	"Green",
	"Blue",
	"Yellow",
};

TEST_F(HeadersTests, Multiple) {
	CreateHeaders(
		testHeaders_Multiple,
		sizeof(testHeaders_Multiple) / sizeof(const char*),
		false);

	ASSERT_EQ(4, headers->count);
	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	EXPECT_STREQ("Red", headers->items[0].name);
	EXPECT_STREQ("Green", headers->items[1].name);
	EXPECT_STREQ("Blue", headers->items[2].name);
	EXPECT_STREQ("Yellow", headers->items[3].name);
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when passed a
// collection containing a single duplicate
// ----------------------------------------------------------------------
const char* testHeaders_SingleDuplicate[] = {
	"Red",
	"Red",
};

TEST_F(HeadersTests, SingleDuplicate) {
	CreateHeaders(
		testHeaders_SingleDuplicate,
		sizeof(testHeaders_SingleDuplicate) / sizeof(const char*),
		false);

	ASSERT_EQ(1, headers->count);
	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	EXPECT_STREQ("Red", headers->items[0].name);
}


// ----------------------------------------------------------------------
// Check that header collection creation works properly when passed a
// collection containing multiple duplicates
// ----------------------------------------------------------------------
const char* testHeaders_MultipleDuplicate[] = {
	"Green",
	"Red",
	"Red",
	"Black",
	"Green"
};

TEST_F(HeadersTests, MultipleDuplicate) {
	CreateHeaders(
		testHeaders_MultipleDuplicate,
		sizeof(testHeaders_MultipleDuplicate) / sizeof(const char*),
		false);

	ASSERT_EQ(3, headers->count);
	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	EXPECT_STREQ("Green", headers->items[0].name);
	EXPECT_STREQ("Red", headers->items[1].name);
	EXPECT_STREQ("Black", headers->items[2].name);
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when one of
// the headers is an empty string
// ----------------------------------------------------------------------
const char* testHeaders_EmptyString[] = {
	"Green",
	"",
	"Black",
};

TEST_F(HeadersTests, EmptyString) {
	CreateHeaders(
		testHeaders_EmptyString,
		sizeof(testHeaders_EmptyString) / sizeof(const char*),
		false);

	ASSERT_EQ(2, headers->count);
	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	EXPECT_STREQ("Green", headers->items[0].name);
	EXPECT_STREQ("Black", headers->items[1].name);
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when one of
// the headers is NULL
// ----------------------------------------------------------------------
const char* testHeaders_NullString[] = {
	"Green",
	NULL,
	"Black",
};

TEST_F(HeadersTests, NullString) {
	CreateHeaders(
		testHeaders_NullString,
		sizeof(testHeaders_NullString) / sizeof(const char*),
		false);

	ASSERT_EQ(2, headers->count);
	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	EXPECT_STREQ("Green", headers->items[0].name);
	EXPECT_STREQ("Black", headers->items[1].name);
}

// ----------------------------------------------------------------------
// Check that header collection creation works properly when two of
// the headers are the same text but different case.
// ----------------------------------------------------------------------
const char* testHeaders_Case[] = {
	"Green",
	"green",
	"Black",
};

TEST_F(HeadersTests, CheckCase) {
	CreateHeaders(
		testHeaders_Case,
		sizeof(testHeaders_Case) / sizeof(const char*),
		false);
	ASSERT_EQ(2, headers->count);
	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	EXPECT_STREQ("Green", headers->items[0].name);
	EXPECT_STREQ("Black", headers->items[1].name);
}

// ----------------------------------------------------------------------
// Check that header collection creation works correctly when the
// 'expectUpperPrefixedHeaders' option is enabled.
// ----------------------------------------------------------------------
const char* testHeaders_HttpPrefix[] = {
	"Red",
	"Black",
};

TEST_F(HeadersTests, HttpPrefix) {
	CreateHeaders(
		testHeaders_HttpPrefix,
		sizeof(testHeaders_HttpPrefix) / sizeof(const char*),
		true);

	ASSERT_EQ(0,
		fiftyoneDegreesHeaderGetIndex(
			container.headers,
			"HTTP_Red",
			strlen("HTTP_Red")));
	ASSERT_EQ(1,
		fiftyoneDegreesHeaderGetIndex(
            container.headers,
			"HTTP_Black",
			strlen("HTTP_Black")));
}

TEST_F(HeadersTests, IsHttp) {
    CreateHeaders(
        testHeaders_HttpPrefix,
        sizeof(testHeaders_HttpPrefix) / sizeof(const char*),
        true);
    EXPECT_TRUE(fiftyoneDegreesHeadersIsHttp(container.headers, "HTTP_Red", strlen("HTTP_Red")));
    EXPECT_TRUE(fiftyoneDegreesHeadersIsHttp(container.headers, "Red", strlen("Red")));
}

TEST_F(HeadersTests, IsHttpNoPrefix) {
    CreateHeaders(
        testHeaders_HttpPrefix,
        sizeof(testHeaders_HttpPrefix) / sizeof(const char*),
        false);
    EXPECT_TRUE(fiftyoneDegreesHeadersIsHttp(container.headers, "Red", strlen("Red")));
    EXPECT_FALSE(fiftyoneDegreesHeadersIsHttp(container.headers, "Yellow", strlen("Yellow")));
}

TEST_F(HeadersTests, FromUniqueId) {
    CreateHeaders(
        testHeaders_HttpPrefix,
        sizeof(testHeaders_HttpPrefix) / sizeof(const char*),
        true);
    Header *header = &container.headers->items[0];
    HeaderID uniqueId = header->headerId;
    Header *actual = fiftyoneDegreesHeadersGetHeaderFromUniqueId(container.headers, uniqueId);
    EXPECT_EQ(actual, header);
    EXPECT_NE(actual, nullptr);
    
    Header *shouldBeNull = fiftyoneDegreesHeadersGetHeaderFromUniqueId(container.headers, 123456789);
    EXPECT_EQ(shouldBeNull,nullptr);
}


// ----------------------------------------------------------------------
// Check that header collection creation works correctly when a 
// collection with no headers is passed
// ----------------------------------------------------------------------
const char** testHeaders_None = new const char*[0];

TEST_F(HeadersTests, None) {
	CreateHeaders(
		testHeaders_None,
		0,
		false);
	ASSERT_EQ(0, headers->count);
}

// ----------------------------------------------------------------------
// Check that header collection creation construct pseudo headers
// correctly
// ----------------------------------------------------------------------
const char* testHeaders_PseudoHeaders[] = {
	"header1",
	"header2",
	"header3",
	"header1\x1Fheader2",
	"header2\x1Fheader3",
	"header1\x1Fheader2\x1Fheader3"
};

TEST_F(HeadersTests, PseudoHeadersPositive) {
	CreateHeaders(
		testHeaders_PseudoHeaders,
		sizeof(testHeaders_PseudoHeaders) / sizeof(const char*),
		false);
	EXPECT_EQ(6, headers->count);

	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	EXPECT_STREQ("header1", headers->items[0].name);
	EXPECT_EQ(0, headers->items[0].segmentHeaders->count); // a simple header does not have segments
    EXPECT_EQ(2, headers->items[0].pseudoHeaders->count); // mentioned in 2 pseudoheaders
    EXPECT_EQ(&headers->items[3], headers->items[0].pseudoHeaders->items[0]);
    EXPECT_EQ(&headers->items[5], headers->items[0].pseudoHeaders->items[1]);

	EXPECT_STREQ("header2", headers->items[1].name);
	EXPECT_EQ(0, headers->items[1].segmentHeaders->count); // a simple header does not have segments
    EXPECT_EQ(3, headers->items[1].pseudoHeaders->count); //mentioned in 3 pseudoheaders
    EXPECT_EQ(&headers->items[3], headers->items[1].pseudoHeaders->items[0]);
    EXPECT_EQ(&headers->items[4], headers->items[1].pseudoHeaders->items[1]);
    EXPECT_EQ(&headers->items[5], headers->items[1].pseudoHeaders->items[2]);

	EXPECT_STREQ("header3", headers->items[2].name);
	EXPECT_EQ(0, headers->items[2].segmentHeaders->count); // a simple header does not have segments
    EXPECT_EQ(2, headers->items[2].pseudoHeaders->count); // mentioned in 2 pseudoheaders
    EXPECT_EQ(&headers->items[4], headers->items[2].pseudoHeaders->items[0]);
    EXPECT_EQ(&headers->items[5], headers->items[2].pseudoHeaders->items[1]);

	EXPECT_STREQ("header1\x1Fheader2", headers->items[3].name);
	EXPECT_EQ(2, headers->items[3].segmentHeaders->count);
	EXPECT_EQ(7, headers->items[3].segmentHeaders->items[0]->nameLength);
	EXPECT_EQ(7, headers->items[3].segmentHeaders->items[1]->nameLength);
	EXPECT_EQ(0, StringCompareLength(
		"header1", 
		headers->items[3].segmentHeaders->items[0]->name,
		7));
    EXPECT_EQ(&headers->items[0], headers->items[3].segmentHeaders->items[0]);
    
	EXPECT_EQ(0, StringCompareLength(
		"header2",
		headers->items[3].segmentHeaders->items[1]->name,
		7));
    EXPECT_EQ(&headers->items[1], headers->items[3].segmentHeaders->items[1]);

	EXPECT_STREQ("header2\x1Fheader3", headers->items[4].name);
	EXPECT_EQ(2, headers->items[4].segmentHeaders->count);
	EXPECT_EQ(7, headers->items[4].segmentHeaders->items[0]->nameLength);
	EXPECT_EQ(7, headers->items[4].segmentHeaders->items[1]->nameLength);
	EXPECT_EQ(0, StringCompareLength(
		"header2",
		headers->items[4].segmentHeaders->items[0]->name,
		7));
    EXPECT_EQ(&headers->items[1], headers->items[4].segmentHeaders->items[0]);
    EXPECT_EQ(0, StringCompareLength(
		"header3",
		headers->items[4].segmentHeaders->items[1]->name,
		7));
    EXPECT_EQ(&headers->items[2], headers->items[4].segmentHeaders->items[1]);
    

	EXPECT_STREQ("header1\x1Fheader2\x1Fheader3", headers->items[5].name);
	EXPECT_EQ(3, headers->items[5].segmentHeaders->count);
	EXPECT_EQ(7, headers->items[5].segmentHeaders->items[0]->nameLength);
	EXPECT_EQ(7, headers->items[5].segmentHeaders->items[1]->nameLength);
	EXPECT_EQ(7, headers->items[5].segmentHeaders->items[2]->nameLength);
	
    EXPECT_EQ(0, StringCompareLength(
		"header1",
		headers->items[5].segmentHeaders->items[0]->name,
		7));
    EXPECT_EQ(&headers->items[0], headers->items[5].segmentHeaders->items[0]);
    
	EXPECT_EQ(0, StringCompareLength(
		"header2",
		headers->items[5].segmentHeaders->items[1]->name,
		7));
    EXPECT_EQ(&headers->items[1], headers->items[5].segmentHeaders->items[1]);
    
    EXPECT_EQ(0, StringCompareLength(
		"header3",
		headers->items[5].segmentHeaders->items[2]->name,
		7));
    EXPECT_EQ(&headers->items[2], headers->items[5].segmentHeaders->items[2]);
}

// ----------------------------------------------------------------------
// Check that header collection creation adds headers contained in a
// pseudo header if it is not already present.
// ----------------------------------------------------------------------

const char* testHeaders_PseudoHeadersMissing[] = {
	"header1\x1Fheader2"
};

TEST_F(HeadersTests, PseudoHeadersMissing) {
	CreateHeaders(
		testHeaders_PseudoHeadersMissing,
		sizeof(testHeaders_PseudoHeadersMissing) / sizeof(const char*),
		false);
	EXPECT_EQ(3, headers->count);

	EXPECT_STREQ("header1\x1Fheader2", headers->items[0].name);
	EXPECT_EQ(true, headers->items[0].isDataSet);
	EXPECT_EQ(2, headers->items[0].segmentHeaders->count);
	EXPECT_EQ(7, headers->items[0].segmentHeaders->items[0]->nameLength);
    EXPECT_EQ(1, headers->items[0].segmentHeaders->items[0]->pseudoHeaders->count);
    EXPECT_EQ(&headers->items[0], headers->items[0].segmentHeaders->items[0]->pseudoHeaders->items[0]);
    
	EXPECT_EQ(7, headers->items[0].segmentHeaders->items[1]->nameLength);
    EXPECT_EQ(1, headers->items[0].segmentHeaders->items[1]->pseudoHeaders->count);
    EXPECT_EQ(&headers->items[0], headers->items[0].segmentHeaders->items[1]->pseudoHeaders->items[0]);
    
	EXPECT_EQ(0, StringCompareLength(
		"header1",
		headers->items[0].segmentHeaders->items[0]->name,
		7));
	EXPECT_EQ(0, StringCompareLength(
		"header2",
		headers->items[0].segmentHeaders->items[1]->name,
		7));

	EXPECT_STREQ("header1", headers->items[1].name);
	EXPECT_EQ(false, headers->items[1].isDataSet);
	EXPECT_EQ(1, headers->items[1].pseudoHeaders->count);
	EXPECT_EQ(headers->items[0].segmentHeaders->items[0], &headers->items[1]);
	

	EXPECT_STREQ("header2", headers->items[2].name);
	EXPECT_EQ(false, headers->items[2].isDataSet);
	EXPECT_EQ(1, headers->items[2].pseudoHeaders->count);
	EXPECT_EQ(headers->items[0].segmentHeaders->items[1], &headers->items[2]);
}

// ----------------------------------------------------------------------
// Check that header collection creation construct pseudo headers
// correctly when pseudo header contains special cases.
// These special cases are very unlikely to happen but are valid. Thus
// added to test the robustness of the code.
// ----------------------------------------------------------------------

const char* testHeaders_PseudoHeadersSpecialCases[] = {
	"header1",
	"header2",
	"\x1Fheader1",
	"header1\x1F",
	"\x1F\x1F\x1F",
	"header1\x1F\x1Fheader2"
};

TEST_F(HeadersTests, PseudoHeadersSpecialCases) {
	CreateHeaders(
		testHeaders_PseudoHeadersSpecialCases,
		sizeof(testHeaders_PseudoHeaders) / sizeof(const char*),
		false);
	EXPECT_EQ(5, headers->count);

	for (uint32_t i = 0; i < headers->count; i++) {
		EXPECT_EQ(true, headers->items[i].isDataSet);
	}

	EXPECT_STREQ("header1", headers->items[0].name);
	EXPECT_EQ(0, headers->items[0].segmentHeaders->count); // simple header
    EXPECT_EQ(3, headers->items[0].pseudoHeaders->count); // mentioned in 3 pseudoheaders

	EXPECT_STREQ("header2", headers->items[1].name);
	EXPECT_EQ(0, headers->items[1].segmentHeaders->count); // simple header

	EXPECT_STREQ("\x1Fheader1", headers->items[2].name);
	EXPECT_EQ(1, headers->items[2].segmentHeaders->count);
	EXPECT_EQ(7, headers->items[2].segmentHeaders->items[0]->nameLength);
	EXPECT_EQ(0, StringCompareLength(
		"header1",
		headers->items[2].segmentHeaders->items[0]->name,
		7));

    //child parent links:
    EXPECT_EQ(&headers->items[0], headers->items[2].segmentHeaders->items[0]);
    EXPECT_EQ(headers->items[0].pseudoHeaders->items[0], &headers->items[2]);

	EXPECT_STREQ("header1\x1F", headers->items[3].name);
	EXPECT_EQ(1, headers->items[3].segmentHeaders->count);
	EXPECT_EQ(7, headers->items[3].segmentHeaders->items[0]->nameLength);
	EXPECT_EQ(0, StringCompareLength(
		"header1",
		headers->items[3].segmentHeaders->items[0]->name,
		7));
    //child parent links:
    EXPECT_EQ(&headers->items[0], headers->items[3].segmentHeaders->items[0]);
    EXPECT_EQ(headers->items[0].pseudoHeaders->items[1], &headers->items[3]);
    

	EXPECT_STREQ("header1\x1F\x1Fheader2", headers->items[4].name);
	EXPECT_EQ(2, headers->items[4].segmentHeaders->count);
	EXPECT_EQ(7, headers->items[4].segmentHeaders->items[0]->nameLength);
	EXPECT_EQ(7, headers->items[4].segmentHeaders->items[1]->nameLength);
	EXPECT_EQ(0, StringCompareLength(
		"header1",
		headers->items[4].segmentHeaders->items[0]->name,
		7));
	EXPECT_EQ(0, StringCompareLength(
		"header2",
		headers->items[4].segmentHeaders->items[1]->name,
		7));
    //child parent links:
    EXPECT_EQ(&headers->items[0], headers->items[4].segmentHeaders->items[0]);
    EXPECT_EQ(headers->items[0].pseudoHeaders->items[2], &headers->items[4]);
}
