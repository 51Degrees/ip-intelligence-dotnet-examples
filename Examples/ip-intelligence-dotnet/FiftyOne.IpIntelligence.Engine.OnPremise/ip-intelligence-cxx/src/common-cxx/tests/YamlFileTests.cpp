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
#include <cstdio>
#include <string>
#include <sstream>
#include "../yamlfile.h"
#include "../fiftyone.h"
#include <vector>

using namespace std;

// Append to a file to indicate that it should be
// cleaned at the end of the test
#define TEMP_FILE_EXT "_tmp"

struct TestValuePair {
	string key;
	string value;
};

typedef vector<TestValuePair> TestDocument;
typedef vector<TestDocument> TestState;

/*
 * Create a file name used for testing. The file
 * name will be pointed to the current path and
 * appended with '_tmp' suffix.
 * @param fileName plain file name to be created
 * @return the string object of the newly constructed
 * file name.
 */
static string getFileWithPath(string fileName) {
	stringstream ss = stringstream("");
	ss << "./" << fileName << TEMP_FILE_EXT;
	return ss.str();
}

/*
 * Create a file at the given path and given records
 * The file will end without a newline
 * @param fileNamePath path to the test file
 * @param testRecords the records will be written to
 * the file
 * @param size the number of records being written
 * @param withEOFNewLine whether a new line should
 * be appended to the end of the file
 */
static void createTestFile(string fileNamePath, vector<string> testRecords, bool withEOFNewLine)
{
	// Create a new file
	FILE* fp = fopen(fileNamePath.c_str(), "w+");

	if (fp != NULL) {
		for (size_t i = 0; i < testRecords.size(); i++) {
			if (i != 0) {
				// Write a new line if not first
				// The end result will always be
				// a file without EOF newline
				fwrite("\n", 1, 1, fp);
			}
			fwrite(testRecords[i].c_str(), testRecords[i].size(), 1, fp);
		}
	}

	if (withEOFNewLine) {
		fwrite("\n", 1, 1, fp);
	}

	// Create a new file
	fclose(fp);
}

/*
 * Close and delete a file
 * @param fileName path to the file to delete
 */
static void deleteTestFile(string fileName)
{
	remove(fileName.c_str());
}

/*
 * Callback function to count the number of records
 * being read from the file
 * @param testRecord the current record
 * @param state the test state that hold the current
 * progress of the iteration
 */
static void testCallBack(KeyValuePair *pairs, uint16_t size, void* state) {
	TestState *tState = (TestState*)state;
	TestDocument newDocument;
	for (int i = 0; i < size; i++) {
		newDocument.push_back({ pairs[i].key, pairs[i].value });
	}
	tState->push_back(newDocument);
}

struct TestOptions {
	bool eofNewLine = true;
	int limit = -1;
	bool addDotsLine = true;
	bool addEmptyDoc = false;
};

static void testYamlFileIterator(const TestOptions testOptions, const TestState &expected) {
	string filePath = getFileWithPath("testfile");
	vector<string> testRecords =
	{
		"---",
		"header.TestKey1: 'TestRecord1'",
		"header.TestKey2: TestRecord2",
		"---",
		"---",
		"header.TestKey3: TestRecord3",
		"header.TestKey4: TestRecord4",
		"header.TestKey5: TestRecord5",
		"---",
		"header.TestKey6: TestRecord6",
	};
	if (testOptions.addEmptyDoc) {
		testRecords.push_back("---");
	}
	if (testOptions.addDotsLine) {
		testRecords.push_back("...");
	}
	char buffer[500] = "";
	TestState state;

	createTestFile(filePath, testRecords, testOptions.eofNewLine);

	char key[4][100];
	char value[4][100];
	KeyValuePair pairs[4] = {
		{key[0], 100, value[0], 100},
		{key[1], 100, value[1], 100},
		{key[2], 100, value[2], 100},
		{key[3], 100, value[3], 100}
	};

	// Iterate through file and count the number of records
	fiftyoneDegreesYamlFileIterateWithLimit(
		filePath.c_str(),
		buffer,
		500,
		pairs,
		4,
		testOptions.limit,
		&state,
		testCallBack);

	// Remove the test file
	deleteTestFile(filePath);

	ASSERT_EQ(expected.size(), state.size()) << expected.size() << " records should have been read.";
	for (uint32_t i = 0; i < state.size(); i++) {
		ASSERT_EQ(expected[i].size(), state[i].size());
		for (uint32_t j = 0; j < state[i].size(); j++) {
			ASSERT_EQ(0, strcmp(expected[i][j].key.c_str(), state[i][j].key.c_str())) <<
				"Key is not as expected. Expected " << expected[i][j].key <<
				" but get " << state[i][j].key;
			ASSERT_EQ(0, strcmp(expected[i][j].value.c_str(), state[i][j].value.c_str())) <<
				"Key is not as expected. Expected " << expected[i][j].value <<
				" but get " << state[i][j].value;
		}
	}
}

TEST(YamlFileIteratorTests, NoLimitWithoutEOFNewLine) {
	TestState expected = {
		{
			{"header.TestKey1", "TestRecord1"},
			{"header.TestKey2", "TestRecord2"}
		},
		{
			{"header.TestKey3", "TestRecord3"},
			{"header.TestKey4", "TestRecord4"},
			{"header.TestKey5", "TestRecord5"}
		},
		{
			{"header.TestKey6", "TestRecord6"},
		},
	};
	testYamlFileIterator({ false }, expected);
}

TEST(YamlFileIteratorTests, NoLimitWithEOFNewLine) {
	TestState expected = {
		{
			{"header.TestKey1", "TestRecord1"},
			{"header.TestKey2", "TestRecord2"},
		},
		{
			{"header.TestKey3", "TestRecord3"},
			{"header.TestKey4", "TestRecord4"},
			{"header.TestKey5", "TestRecord5"},
		},
		{
			{"header.TestKey6", "TestRecord6"},
		},
	};
	testYamlFileIterator({ true }, expected);
}

TEST(YamlFileIteratorTests, Limit) {
	TestState expected = {
		{
			{"header.TestKey1", "TestRecord1"},
			{"header.TestKey2", "TestRecord2"},
		},
		{
			{"header.TestKey3", "TestRecord3"},
			{"header.TestKey4", "TestRecord4"},
			{"header.TestKey5", "TestRecord5"},
		},
	};
	testYamlFileIterator({ false, 2 }, expected);
}

TEST(YamlFileIteratorTests, NoLimitWithoutEOFNewLineWithoutDotsLine) {
	TestState expected = {
		{
			{"header.TestKey1", "TestRecord1"},
			{"header.TestKey2", "TestRecord2"}
		},
		{
			{"header.TestKey3", "TestRecord3"},
			{"header.TestKey4", "TestRecord4"},
			{"header.TestKey5", "TestRecord5"}
		},
		{
			{"header.TestKey6", "TestRecord6"},
		},
	};
	testYamlFileIterator({ false, -1, false }, expected);
}

TEST(YamlFileIteratorTests, NoLimitWithEOFNewLineWithoutDotsLine) {
	TestState expected = {
		{
			{"header.TestKey1", "TestRecord1"},
			{"header.TestKey2", "TestRecord2"}
		},
		{
			{"header.TestKey3", "TestRecord3"},
			{"header.TestKey4", "TestRecord4"},
			{"header.TestKey5", "TestRecord5"}
		},
		{
			{"header.TestKey6", "TestRecord6"},
		},
	};
	testYamlFileIterator({ true, -1, false }, expected);
}

TEST(YamlFileIteratorTests, NoLimitLastDocEmptyNoDotsLineNoEOFNewLine) {
	TestState expected = {
		{
			{"header.TestKey1", "TestRecord1"},
			{"header.TestKey2", "TestRecord2"}
		},
		{
			{"header.TestKey3", "TestRecord3"},
			{"header.TestKey4", "TestRecord4"},
			{"header.TestKey5", "TestRecord5"}
		},
		{
			{"header.TestKey6", "TestRecord6"},
		},
	};
	testYamlFileIterator({ false, -1, false, true }, expected);
}

TEST(YamlFileIteratorTests, NoLimitLastDocEmptyNoDotsLine) {
	TestState expected = {
		{
			{"header.TestKey1", "TestRecord1"},
			{"header.TestKey2", "TestRecord2"}
		},
		{
			{"header.TestKey3", "TestRecord3"},
			{"header.TestKey4", "TestRecord4"},
			{"header.TestKey5", "TestRecord5"}
		},
		{
			{"header.TestKey6", "TestRecord6"},
		},
	};
	testYamlFileIterator({ true, -1, false, true }, expected);
}

TEST(YamlFileIteratorTests, NoLimitLastDocEmptyNoEOFNewLine) {
	TestState expected = {
		{
			{"header.TestKey1", "TestRecord1"},
			{"header.TestKey2", "TestRecord2"}
		},
		{
			{"header.TestKey3", "TestRecord3"},
			{"header.TestKey4", "TestRecord4"},
			{"header.TestKey5", "TestRecord5"}
		},
		{
			{"header.TestKey6", "TestRecord6"},
		},
	};
	testYamlFileIterator({ false, -1, true, true }, expected);
}