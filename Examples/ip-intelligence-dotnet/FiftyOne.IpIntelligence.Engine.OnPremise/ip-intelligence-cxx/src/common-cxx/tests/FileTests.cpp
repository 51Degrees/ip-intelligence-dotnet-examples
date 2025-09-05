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
#include <stdio.h>
#include <sys/stat.h>

#include "../exceptions.h"
#include "../file.h"
#include "../snprintf.h"


#ifndef _MSC_VER
#define _rmdir rmdir
#endif

static const char *tempPath1 = "tmp1";
static const char* tempPath2 = "tmp2";
static const char *fileName = "tempfile";
static const char someData[] = "some data";

/**
 * File test class used to test the functionality of file.c.
 */
class File : public Base {
public:
	File() : Base() {}

protected:
	fiftyoneDegreesFilePool pool;

	/**
	 * Calls the base setup method to enable memory leak checking and memory
	 * allocation checking.
	 */
	void SetUp() {
		Base::SetUp();
		createFile(fileName);
	}

	/**
	 * Releases the file pool structure if one was created. Then calls the base
	 * teardown method to check for memory leaks and compare expected and
	 * actual memory allocations.
	 */
	void TearDown() {
		removeFile(fileName);
		Base::TearDown();
	}

	/**
	 * Initialises a file pool structure with the specified concurrency using
	 * the init method in file.c. The expected memory allocation is calculated,
	 * and the actual memory allocation is tracked. The structure is freed
	 * automatically after each test, at which point the expected and actual
	 * memory allocation is checked for equality.
	 * @param concurrency the concurrency of the file pool
	 */
	void InitPool(int16_t concurrency) {
		fiftyoneDegreesStatusCode status;
		FIFTYONE_DEGREES_EXCEPTION_CREATE
		status = fiftyoneDegreesFilePoolInit(
			&pool,
			fileName, 
			concurrency, 
			exception);
		EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY);
		EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS, status) <<
			"The file pool was not successfully initialised.";
	}

	/**
	 * Write a file to the file path provided using some sample data. The path
	 * to the file must exist. This method checks that the 51Degrees file
	 * operation was successful.
	 * @param file path to the file to create (including the file name)
	 */
	static void createFile(const char *file) {
		EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
			fiftyoneDegreesFileWrite(file, someData, strlen(someData))) <<
			"Data was not written to the file.";
	}

	/**
	 * Check if a file exists.
	 * @param file path to check
	 */
	static bool fileExists(const char* file) {
		if (FILE* f = fopen(file, "r")) {
			fclose(f);
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * Create a directory at the path provided. The path to the directory must
	 * exist. This method checks that the 51Degrees file operation was
	 * successful.
	 * @param dir directory path to create
	 */
	static void createDir(const char *dir) {
		fiftyoneDegreesStatusCode status =
			fiftyoneDegreesFileCreateDirectory(dir);

		EXPECT_TRUE(
			status == FIFTYONE_DEGREES_STATUS_SUCCESS ||
			status == FIFTYONE_DEGREES_STATUS_FILE_EXISTS_ERROR) <<
			"The directory could not be created";
	}

	/**
	 * Delete the file at the file path provided. The file must exist. This
	 * method checks that the 51Degrees file operation was successful.
	 * @param file path to the file to remove (including the file name)
	 */
	static void removeFile(const char *file) {
		EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
			fiftyoneDegreesFileDelete(file)) <<
			"The file was not deleted.";
	}

	/**
	 * Delete the directory at the path provided. The directory must exist.
	 * This method checks that the file operation was successful.
	 * @param dir directory path to delete
	 */
	static void removeDir(const char *dir) {
		EXPECT_EQ(0, _rmdir(dir)) <<
			"The temporary directory was not removed.";
	}

	static bool isNumber(char ch) {
		return ch >= '0' && ch <= '9';
	}

	static bool isAlphabet(char ch) {
		return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
	}

	static void checkTempFileName(const char* mainFileName, char* dest) {
		char* tmp = dest;
		char* charPos = strchr(tmp, '-');
		EXPECT_EQ(0, strncmp(mainFileName, tmp, charPos - dest)) <<
			"First segment of the temporary name is not the main file name. " <<
			"Temp name is " << dest;

		tmp = charPos + 1;
		charPos = strchr(tmp, '-');
		EXPECT_NE(tmp, charPos);
		for (char* i = tmp; i < charPos; i++) {
			EXPECT_EQ(true, isNumber(*i)) <<
				"Expected a number but get '" << *i << "'";
		}

		tmp = charPos + 1;
		charPos = strchr(tmp, '\0');
		EXPECT_NE(tmp, charPos);
		EXPECT_EQ(TEMP_UNIQUE_STRING_LENGTH, charPos - tmp);
		for (char* i = tmp; i < charPos; i++) {
			EXPECT_EQ(true, isAlphabet(*i)) <<
				"Expected an alphabet but get '" << *i << "'";
		}
	}

	static void createTempFileName(const char* mainFileName, char* dest, size_t length) {
		EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
			fiftyoneDegreesFileAddTempFileName(mainFileName, dest, 0, length)) <<
			"Failed to create and add a temporary file name to a buffer.";
		checkTempFileName(mainFileName, dest);
	}
};

/**
 * Check that a file pool can be initialised and released without error.
 */
TEST_F(File, PoolInit) {
	int16_t concurrency = 4;
	InitPool(concurrency);
	fiftyoneDegreesFilePoolRelease(&pool);
}

TEST_F(File, TempCreateFileNameWithoutExtension) {
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	createTempFileName(fileName, tempFileName, FIFTYONE_DEGREES_FILE_MAX_PATH);
}

TEST_F(File, TempCreateFileNameWithExtension) {
	const char* fileNameWithExtension = "tempfile.ext";
	const char* extension = strrchr(fileNameWithExtension, '.');
	int lenWithoutExtension = (int)(extension - fileNameWithExtension);
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	createTempFileName(fileNameWithExtension, tempFileName, FIFTYONE_DEGREES_FILE_MAX_PATH);
	EXPECT_EQ(0, strncmp(fileNameWithExtension, tempFileName, lenWithoutExtension));
	EXPECT_EQ(0, strncmp("-", tempFileName + lenWithoutExtension, 1));
}

TEST_F(File, TempCreateFilenameSmallBuffer) {
	char tempFileName[1];

	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY,
		fiftyoneDegreesFileAddTempFileName(fileName, tempFileName, 0, 1)) <<
		"Failed to create and add a temporary file name to a buffer.";
	EXPECT_EQ('\0', tempFileName[0]);
}

TEST_F(File, TempCreateFileNameNoDup) {
	char tempFileNames[2][FIFTYONE_DEGREES_FILE_MAX_PATH];
	for (int i = 0; i < 2; i++) {
		createTempFileName(fileName, tempFileNames[i], FIFTYONE_DEGREES_FILE_MAX_PATH);
	}

	EXPECT_NE(0, strcmp(tempFileNames[0], tempFileNames[1]));
}

TEST_F(File, TempCreateEmptyPath) {
	const char* paths[] = { "" };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(fileName, paths, 1, tempFileName, FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_NE('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
	EXPECT_TRUE(fileExists(tempFileName)) <<
		"The temporary file was not created.";
	removeFile(tempFileName);
}

/**
 * Check that a temp file can be successfully created in the same directory as
 * the master file.
 */
TEST_F(File, TempFileCreateEmptyPath) {
	const char *paths[] = { "" };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			paths,
			1,
			tempFileName,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_NE('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
	EXPECT_TRUE(fileExists(tempFileName)) <<
		"The temporary file was not created.";
	removeFile(tempFileName);
}

/**
 * Check that a temp file can be successfully created in the same directory as
 * the master file when no directories are passed to the method.
 */
TEST_F(File, TempCreateDefaultPath) {
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(fileName, NULL, 0, tempFileName, FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_NE('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
	EXPECT_TRUE(fileExists(tempFileName)) <<
		"The temporary file was not created.";
	removeFile(tempFileName);
}

/**
 * Check that a temp file can be successfully created in the same directory as
 * the master file when no directories are passed to the method.
 */
TEST_F(File, TempFileCreateDefaultPath) {
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			NULL,
			0,
			tempFileName,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_NE('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
	removeFile(tempFileName);
}

/**
 * Check that a temp file cannot be created in a directory which does not
 * exist.
 */
TEST_F(File, TempCreateInvalidPath) {
	const char* paths[] = { tempPath1 };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	EXPECT_NE(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(fileName, paths, 1, tempFileName, FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A missing directory was not reported.";
	EXPECT_EQ('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
}

/**
 * Check that a temp file cannot be created in a directory which does not
 * exist.
 */
TEST_F(File, TempFileCreateInvalidPath) {
	const char *paths[] = { tempPath1 };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	EXPECT_NE(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			paths,
			1,
			tempFileName,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A missing directory was not reported.";
	EXPECT_EQ('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
}

/**
 * Check that a temp file can be successfully created in a directory passed to
 * the method.
 */
TEST_F(File, TempCreateValidPath) {
	const char* paths[] = { tempPath1 };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(fileName, paths, 1, tempFileName, FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_NE('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
	EXPECT_TRUE(fileExists(tempFileName)) <<
		"The temporary file was not created.";
	removeFile(tempFileName);
	removeDir(tempPath1);
}

/**
 * Check that a temp file can be successfully created in a directory passed to
 * the method.
 */
TEST_F(File, TempFileCreateValidPath) {
	const char *paths[] = { tempPath1 };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			paths,
			1,
			tempFileName,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_NE('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
	EXPECT_TRUE(fileExists(tempFileName)) <<
		"The temporary file was not created.";
	removeFile(tempFileName);
	removeDir(tempPath1);
}

/*
 * Check is the right status code is returned if the destination length is not
 * big enough without paths
 */
TEST_F(File, TempCreateSmallBufferWithoutPaths) {
	char tempFileName[1];
	tempFileName[0] = '\0';
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY,
		fiftyoneDegreesFileNewTempFile(fileName, NULL, 0, tempFileName, 1)) <<
		"Expected file path too long status.";
	EXPECT_EQ('\0', tempFileName[0]) <<
		"Expected string buffer to be set to zeros.";
	removeDir(tempPath1);
}

/*
 * Check is the right status code is returned if the destination length is not
 * big enough with paths
 */
TEST_F(File, TempCreateSmallBufferWithPaths) {
	const char* paths[] = { tempPath1 };
	char tempFileName[1];
	tempFileName[0] = '\0';
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY,
		fiftyoneDegreesFileNewTempFile(fileName, paths, 1, tempFileName, 1)) <<
		"Expected file path too long status.";
	EXPECT_EQ('\0', tempFileName[0]) <<
		"Expected string buffer to be set to zeros.";
	removeDir(tempPath1);
}

/*
 * Check is the right status code is returned if the destination length is 0
 * with paths
 */
TEST_F(File, TempCreateZeroBufferWithPaths) {
	const char* paths[] = { tempPath1 };
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY,
		fiftyoneDegreesFileNewTempFile(fileName, paths, 1, NULL, 0)) <<
		"Expected file path too long status.";
	removeDir(tempPath1);
}

/*
 * Check is the right status code is returned if the destination length is 0
 * without paths
 */
TEST_F(File, TempCreateZeroBufferWithoutPaths) {
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY,
		fiftyoneDegreesFileNewTempFile(fileName, NULL, 0, NULL, 0)) <<
		"Expected file path too long status.";
	removeDir(tempPath1);
}

/*
 * Check if only one temp file is created in the target path
 */
TEST_F(File, TempFileCreateInvalidAndValidPaths) {
	const char* paths[] = { tempPath1, tempPath2 };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	createDir(tempPath2);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName, paths, 2, tempFileName, FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_NE('\0', tempFileName[0]) <<
		"The temporary file's path was not returned.";
	EXPECT_TRUE(fileExists(tempFileName)) <<
		"The temporary file was not created.";
	EXPECT_EQ(0, strncmp(tempPath2, tempFileName, strlen(tempPath2))) <<
		"The temporary file was created in the wrong path.";
	removeFile(tempFileName);
	removeDir(tempPath2);
}

/**
 * Check that a file can be copied successfully, and that all the data has been
 * copied.
 */
TEST_F(File, FileCopy) {
	FILE *orig, *copy;
	const char *copiedFileName = "copiedfile";
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
			fiftyoneDegreesFileCopy(fileName, copiedFileName));
	fiftyoneDegreesFileOpen(fileName, &orig);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileOpen(copiedFileName, &copy));
	fseek(orig, 0, SEEK_END);
	fseek(copy, 0, SEEK_END);
	EXPECT_EQ(ftell(orig), ftell(copy));
	fclose(copy);
	fclose(orig);
	removeFile(copiedFileName);
}

/**
 * Check that a directory can be created, and returns the correct status.
 */
TEST_F(File, DirectoryCreate) {
	struct stat dir;

	ASSERT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileCreateDirectory(tempPath1)) <<
		"The method did not report the operation successful.";
	ASSERT_TRUE(
		(stat(tempPath1, &dir) == 0 &&
			(dir.st_mode & S_IFMT) == S_IFDIR)) <<
		"The directory was not created.";
	removeDir(tempPath1);
}

/**
 * Check that an existing temporary file can be found by the get existing temp
 * file method, and that the path returned is correct.
 */
TEST_F(File, TempExists) {
	const char *paths[] = { tempPath1 };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char foundFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	// Make the temp directory to use.
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(fileName, paths, 1, tempFileName, FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";

	EXPECT_TRUE(fiftyoneDegreesFileGetExistingTempFile(
		fileName,
		paths,
		1,
		-1,
		foundFileName)) <<
		"The existing temporary file was not found.";
	EXPECT_STREQ(tempFileName, foundFileName) <<
		"The matching file name was not written to the memory passed in.";

	removeFile(tempFileName);
	removeDir(tempPath1);
}

/**
 * Check that an existing temporary file can be found and deleted by the
 * delete existing temp file method, and that the number of deleted files
 * returned is correct.
 */
TEST_F(File, TempExists_Delete) {
	const char *paths[] = { tempPath1 };
	char tempFileName1[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char tempFileName2[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char foundFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName1[0] = '\0';
	tempFileName2[0] = '\0';
	// Make the temp directory to use.
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(fileName, paths, 1, tempFileName1, FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(fileName, paths, 1, tempFileName2, FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";

	EXPECT_TRUE(fiftyoneDegreesFileGetExistingTempFile(
		fileName,
		paths,
		1,
		-1,
		foundFileName)) <<
		"The existing temporary file was not found.";

	EXPECT_EQ(2,
		fiftyoneDegreesFileDeleteUnusedTempFiles(fileName, paths, 1, -1)) <<
		"2 temporary files should have been deleted.";

	EXPECT_FALSE(fiftyoneDegreesFileGetExistingTempFile(
		fileName,
		paths,
		1,
		-1,
		foundFileName)) <<
		"The existing temporary file was not deleted.";

	removeDir(tempPath1);
}

#ifndef __APPLE__

/**
 * Check that an existing temporary file (which is in use) can be found and is
 * not deleted by the delete existing temp file method, and that the number of
 * deleted files returned is correct.
 */
TEST_F(File, TempExists_DeleteInUse) {
	const char *paths[] = { tempPath1 };
	char tempFileName1[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char tempFileName2[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char foundFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName1[0] = '\0';
	tempFileName2[0] = '\0';
	// Make the temp directory to use.
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			paths,
			1,
			tempFileName1,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			paths,
			1,
			tempFileName2,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";

	EXPECT_TRUE(fiftyoneDegreesFileGetExistingTempFile(
		fileName,
		paths,
		1,
		-1,
		foundFileName)) <<
		"The existing temporary file was not found.";

	FILE *handle;
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileOpen(foundFileName, &handle)) <<
		"The temp file could not be opened.";

	EXPECT_EQ(1,
		fiftyoneDegreesFileDeleteUnusedTempFiles(fileName, paths, 1, -1)) <<
		"1 temporary file should have been deleted.";

	EXPECT_TRUE(fiftyoneDegreesFileGetExistingTempFile(
		fileName,
		paths,
		1,
		-1,
		foundFileName)) <<
		"The existing temporary file was deleted even though it was in use.";

	fclose(handle);
	removeFile(foundFileName);
	removeDir(tempPath1);
}

/**
 * Check that temporary file which is not prefixed with the same base name is
 * not deleted by the delete existing temp file method, and that the number of
 * deleted files returned is correct.
 */
TEST_F(File, TempExists_DeleteDifferentPrefix) {
	const char *paths[] = { tempPath1 };
	char tempFileName1[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char tempFileName2[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName1[0] = '\0';
	tempFileName2[0] = '\0';
	// Make the temp directory to use.
	createDir(tempPath1);
	createFile("a-different-prefix.dat");
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			paths,
			1,
			tempFileName1,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			"a-different-prefix.dat",
			paths,
			1,
			tempFileName2,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_EQ(1,
		fiftyoneDegreesFileDeleteUnusedTempFiles(fileName, paths, 1, -1)) <<
		"1 temporary file should have been deleted.";

	EXPECT_TRUE(fiftyoneDegreesFileGetExistingTempFile(
		"a-different-prefix.dat",
		paths,
		1,
		-1,
		tempFileName2)) <<
		"The existing temporary file was deleted even though it had a different prefix.";

	removeFile(tempFileName2);
	removeFile("a-different-prefix.dat");
	removeDir(tempPath1);
}

#endif

/**
 * Check that an existing temporary file can be found by the get existing temp
 * file method when only the first n bytes are compared, and that the path
 * returned is correct.
 */
TEST_F(File, TempExists_ComparePortion) {
	const char *paths[] = { tempPath1 };
	long bytesToCompare = 4;
	char alteredData[sizeof(someData)];
	strcpy(alteredData, someData);

	// Modify some of the data in altered data to make the temp file different
	// for the purposes of this test.
	alteredData[bytesToCompare]++;

	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char foundFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	// Make the temp directory to use.
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			paths,
			1,
			tempFileName,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileWrite(
			tempFileName,
			(void*)alteredData,
			strlen(alteredData))) <<
		"Data was not written to the file.";

	EXPECT_FALSE(fiftyoneDegreesFileGetExistingTempFile(
		fileName,
		paths,
		1,
		bytesToCompare + 1,
		foundFileName)) <<
		"An incorrect temporary file was returned.";

	EXPECT_TRUE(fiftyoneDegreesFileGetExistingTempFile(
		fileName,
		paths,
		1,
		bytesToCompare,
		foundFileName)) <<
		"The existing temporary file was not found.";
	EXPECT_STREQ(tempFileName, foundFileName) <<
		"The matching file name was not written to the memory passed in.";

	removeFile(tempFileName);
	removeDir(tempPath1);
}

/**
 * Check that the get existing temp file method returns false if a matching
 * file is not found.
 */
TEST_F(File, TempDoesNotExist) {
	const char *paths[] = { tempPath1 };
	char foundFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	// Make the temp directory to use.
	createDir(tempPath1);
	EXPECT_FALSE(fiftyoneDegreesFileGetExistingTempFile(
		fileName,
		paths,
		1,
		-1,
		foundFileName)) <<
		"An existing temporary file does not exist yet, so false should " <<
		"have been returned.";
	removeDir(tempPath1);
}

/**
 * Check that the get existing temp file method returns false if a file matches
 * but is the master file passed in.
 */
TEST_F(File, TempDoesNotMatchSelf) {
	const char *paths[] = { tempPath1 };
	char tempFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char foundFileName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	tempFileName[0] = '\0';
	// Make the temp directory to use.
	createDir(tempPath1);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileNewTempFile(
			fileName,
			paths,
			1,
			tempFileName,
			FIFTYONE_DEGREES_FILE_MAX_PATH)) <<
		"A temporary file was not created.";

	EXPECT_FALSE(fiftyoneDegreesFileGetExistingTempFile(
		tempFileName,
		paths,
		1,
		-1,
		foundFileName)) <<
		"The same file was reported as a match for itself.";

	removeFile(tempFileName);
	removeDir(tempPath1);
}

/**
 * Check that an existing file is found by the getPath method, and that the
 * path it was found at is written correctly.
 */
TEST_F(File, GetPath) {
	char relativePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char absolutePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char foundPath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	Snprintf(relativePath, FIFTYONE_DEGREES_FILE_MAX_PATH, "%s/%s", tempPath1, fileName);
	createDir(tempPath1);
	createFile(relativePath);

	EXPECT_EQ(
		FIFTYONE_DEGREES_STATUS_SUCCESS,
		fiftyoneDegreesFileGetPath(
			tempPath1,
			fileName,
			foundPath,
			sizeof(foundPath))) <<
		"The file was not found.";
	
    EXPECT_NE(GET_CURRENT_DIR(absolutePath, sizeof(absolutePath)), (char *)NULL);
	
	int written = Snprintf(absolutePath + strlen(absolutePath), FIFTYONE_DEGREES_FILE_MAX_PATH - strlen(absolutePath), "/%s", relativePath);
	if (written < 0 || written >= (int)(FIFTYONE_DEGREES_FILE_MAX_PATH - strlen(absolutePath))) {
		FAIL() << "The absolute path was too long.";
	}
	
	EXPECT_STREQ(absolutePath, foundPath) <<
		"The found file did not match the actual path.";
	
	removeFile(relativePath);
	removeDir(tempPath1);
}

/**
 * Check that a missing file is reported correctly with the file not found
 * status.
 */
TEST_F(File, GetPath_NoFile) {
	char relativePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	createDir(tempPath1);
	Snprintf(relativePath, FIFTYONE_DEGREES_FILE_MAX_PATH, "%s/%s", tempPath1, fileName);

	EXPECT_EQ(
		FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND,
		fiftyoneDegreesFileGetPath(tempPath1, fileName, NULL, 0)) <<
		"File not found was not reported.";

	removeDir(tempPath1);
}

/**
 * Check that a missing directory is reported correctly with the file not found
 * status.
 */
TEST_F(File, GetPath_NoDirectory) {
	char relativePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	Snprintf(relativePath, FIFTYONE_DEGREES_FILE_MAX_PATH, "%s/%s", tempPath1, fileName);

	EXPECT_EQ(
		FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND,
		fiftyoneDegreesFileGetPath(tempPath1, fileName, NULL, 0)) <<
		"File not found was not reported.";
}

/**
 * Check that an insufficient memory status is returned when the memory to
 * write the found path to is not enough to store the string.
 */
TEST_F(File, GetPath_InsufficientMemory) {
	char relativePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char foundPath[1];
	Snprintf(relativePath, FIFTYONE_DEGREES_FILE_MAX_PATH, "%s/%s", tempPath1, fileName);
	createDir(tempPath1);
	createFile(relativePath);

	EXPECT_EQ(
		FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY,
		fiftyoneDegreesFileGetPath(
			tempPath1,
			fileName,
			foundPath,
			sizeof(foundPath))) <<
		"Insufficient memory to write the path was not reported.";

	removeFile(relativePath);
	removeDir(tempPath1);
}

/**
 * Check that an exception is not thrown when a data directory parameter is
 * null.
 */
TEST_F(File, GetPath_NullPath) {
	char foundPath[FIFTYONE_DEGREES_FILE_MAX_PATH];

	EXPECT_EQ(
		FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND,
		fiftyoneDegreesFileGetPath(
			NULL,
			fileName,
			foundPath,
			sizeof(foundPath))) <<
		"A file should not have been found for a null path.";
}

/**
 * Check that an exception is not thrown when the file name parameter is null.
 */
TEST_F(File, GetPath_NullFile) {
	char foundPath[FIFTYONE_DEGREES_FILE_MAX_PATH];

	EXPECT_EQ(
		FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND,
		fiftyoneDegreesFileGetPath(
			tempPath1,
			NULL,
			foundPath,
			sizeof(foundPath))) <<
		"A file should not have been found for a null file name.";
}

/**
 * Check that an insufficient memory status is reported if the parameters
 * passed result in a full path which cannot be stored in the method's array.
 */
TEST_F(File, GetPath_PathTooLong) {
	size_t i;
	char longName[FIFTYONE_DEGREES_FILE_MAX_PATH + 1];
	char foundPath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	for (i = 0; i < sizeof(longName) / sizeof(char); i++) {
		longName[i] = 'A';
	}
	longName[sizeof(longName) / sizeof(char) - 1] = '\0';
	EXPECT_EQ(
		FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY,
		fiftyoneDegreesFileGetPath(
			tempPath1,
			longName,
			foundPath,
			sizeof(foundPath))) <<
		"Insufficient memory to store the path was not reported.";
}
