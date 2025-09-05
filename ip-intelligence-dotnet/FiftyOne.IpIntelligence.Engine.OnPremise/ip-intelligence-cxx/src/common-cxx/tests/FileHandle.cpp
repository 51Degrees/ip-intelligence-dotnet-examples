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
#include "FileHandle.hpp"
#include "../Exceptions.hpp"

using namespace FiftyoneDegrees::Common;

/**
 * Opens the file specified by the file name, and writes the data provided to
 * it. A file pool is then created with the concurrency specified. If the file
 * already exists, then it is deleted first.
 * @param fileName full path to the file to create
 * @param data bytes to write to the file
 * @param number of bytes to write
 * @param concurrency to use for the file pool
 */
FileHandle::FileHandle(
	const char *fileName,
	void *data,
	size_t dataSize,
	uint16_t concurrency) {
	FIFTYONE_DEGREES_EXCEPTION_CREATE
	_fileName = fileName;
	fiftyoneDegreesFileDelete(fileName);
	fiftyoneDegreesStatusCode status =
		fiftyoneDegreesFileWrite(_fileName, data, dataSize);
	checkStatus(status);
	if (status == FIFTYONE_DEGREES_STATUS_SUCCESS) {
		status = fiftyoneDegreesFileOpen(_fileName, &_file);
	}
	checkStatus(status);
	if (status == FIFTYONE_DEGREES_STATUS_SUCCESS) {
		status = fiftyoneDegreesFilePoolInit(
			&_filePool,
			_fileName,
			concurrency,
			exception);
		FIFTYONE_DEGREES_EXCEPTION_THROW
	}
	else {
		fclose(_file);
	}
	checkStatus(status);
}

/**
 * Close the file and file pool, then delete the file.
 */
FileHandle::~FileHandle() {
	fiftyoneDegreesFilePoolRelease(&_filePool);
	fclose(_file);
	fiftyoneDegreesFileDelete(_fileName);
}

/**
 * Gets the name of the file.
 * @return file name
 */
const char* FileHandle::getFileName() {
	return _fileName;
}

/**
 * Gets the open file at the position it was left at.
 * @return file
 */
FILE* FileHandle::getFile() {
	return _file;
}

/**
 * Gets the file pool.
 * @return file pool
 */
fiftyoneDegreesFilePool* FileHandle::getFilePool() {
	return &_filePool;
}

/**
 * Check that the file was opened without error. Fails the test if it was not.
 * @param status returned by the file operation
 */
void FileHandle::checkStatus(fiftyoneDegreesStatusCode status) {
	ASSERT_EQ(status, FIFTYONE_DEGREES_STATUS_SUCCESS);
}