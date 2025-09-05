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
#include "../PropertyMetaData.hpp"
#include "../ComponentMetaData.hpp"
#include "../ValueMetaData.hpp"

using namespace FiftyoneDegrees::Common;

string testName = "propertyname";

vector<string> testFiles = { "firstfile", "secondfile" };

string testType = "valuetype";


string testCategory = "category";
string testUrl = "url";
bool testAvailable = true;
byte testDisplayOrder = 0;
bool testIsMandatory = false;
bool testIsList = false;
bool testIsObsolete = false;
bool testShow = false;
bool testShowValues = false;
string testDescription = "description";
string testValue = "value";
byte testComponentId = 0;
vector<uint32_t> testEvidenceProperties = { 0 };
ComponentMetaData *testComponent = nullptr;
vector<ValueMetaData*> *testValues = nullptr;
ValueMetaData *testDefaultValue = nullptr;

/**
 * Check two string vectors are equal. This means they each contain all, and
 * only, the values of the other.
 */
void assertVectorsAreEqual(vector<string> *actual, vector<string> *expected) {
	int i;
	ASSERT_EQ(actual->size(), expected->size()) <<
		L"The vectors are different sizes.";
	for (i = 0; i < (int)actual->size(); i++) {
		ASSERT_STREQ(
			actual->operator[](i).c_str(),
			expected->operator[](i).c_str()) <<
			L"String values are not equal.";
	}
}

/**
 * Check that the values set in the constructor are returned correctly.
 */
TEST(PropertyMetaData, CreateWithValues) {
	PropertyMetaData *meta =
		new PropertyMetaData(
			testName,
			testFiles,
			testType,
			testCategory,
			testUrl,
			testAvailable,
			testDisplayOrder,
			testIsMandatory,
			testIsList,
			testIsObsolete,
			testShow,
			testShowValues,
			testDescription,
			testValue,
			testComponentId,
			testEvidenceProperties);
	ASSERT_STREQ(meta->getName().c_str(), testName.c_str()) <<
		L"Property name was not set correctly.";
	vector<string> returnedFiles = meta->getDataFilesWherePresent();
	assertVectorsAreEqual(&returnedFiles, &testFiles);
	ASSERT_STREQ(meta->getType().c_str(), testType.c_str()) <<
		L"Type was not set correctly.";
	delete meta;
}

/**
 * Check that the vector returned by the instance is a copy of that passed to
 * the constructor, and not a pointer to it.
 */
TEST(PropertyMetaData, VectorIsCopied) {
	PropertyMetaData *meta =
		new PropertyMetaData(
			testName,
			testFiles,
			testType,
			testCategory,
			testUrl,
			testAvailable,
			testDisplayOrder,
			testIsMandatory,
			testIsList,
			testIsObsolete,
			testShow,
			testShowValues,
			testDescription,
			testValue,
			testComponentId,
			testEvidenceProperties);
	// Check the pointer returned does not point to 
	vector<string> returnedFiles = meta->getDataFilesWherePresent();
	ASSERT_NE(&returnedFiles, &testFiles);
	delete meta;
}


/**
 * Check that the values returned by the instance are copies of the arguments
 * passed to the constructor, and not a pointer.
 */
TEST(PropertyMetaData, ValuesAreCopied) {
	string name = testName;
	vector<string> files = testFiles;
	string type = testType;
	string category = testCategory;
	string url = testUrl;
	string description = testDescription;
	PropertyMetaData *meta =
		new PropertyMetaData(
			name,
			files,
			type,
			category,
			url,
			testAvailable,
			testDisplayOrder,
			testIsMandatory,
			testIsList,
			testIsObsolete,
			testShow,
			testShowValues,
			description,
			testValue,
			testComponentId,
			testEvidenceProperties);
	name = "non";
	files = { "non" };
	type = "non";
	category = "non";
	url = "non";
	description = "non";
	ASSERT_STREQ(meta->getName().c_str(), testName.c_str()) <<
		L"Property name was pointing to an external value.";
	vector<string> returnedFiles = meta->getDataFilesWherePresent();
	assertVectorsAreEqual(&returnedFiles, &testFiles);
	ASSERT_STREQ(meta->getType().c_str(), testType.c_str()) <<
		L"Type was pointing to an external value.";
	ASSERT_STREQ(meta->getCategory().c_str(), testCategory.c_str()) <<
		L"Category was pointing to an external value.";
	ASSERT_STREQ(meta->getUrl().c_str(), testUrl.c_str()) <<
		L"Url was pointing to an external value.";
	ASSERT_STREQ(meta->getDescription().c_str(), testDescription.c_str()) <<
		L"Description was pointing to an external value.";
	delete meta;
}