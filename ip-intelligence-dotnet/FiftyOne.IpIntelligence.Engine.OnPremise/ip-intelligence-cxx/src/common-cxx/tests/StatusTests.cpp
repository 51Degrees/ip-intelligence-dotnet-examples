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
#include "../string.h"
#include "../status.h"
#include "../snprintf.h"

/* Test file name to use in tests. */
static const char *fileName = "somefilename.dat";

/**
 * Check that the message string is neither null, nor an empty string. Also
 * check that the final character is a full stop, this means that enough memory
 * has been allocated and the message is properly formatted.
 */
void assertValidMessage(const char *message) {
	ASSERT_NE(message, (const char*)NULL) <<
		L"Message should never be null.";
	ASSERT_STRNE(message, "") <<
		L"Message should never be empty.";
	size_t length = strlen(message);
	ASSERT_EQ(message[length - 1], '.');
}

/**
 * Check that the message contains a string. Neither parameters can be null.
 * @param message to search
 * @param subString to search for
 */
void assertContains(const char *message, const char *subString) {
	ASSERT_TRUE(strstr(message, subString) != NULL) <<
		L"Message does not contain '" << subString <<
		"'. Message was '" << message << "'.";
}

/**
 * Check that a message is returned for a 'success' status and contains the
 * correct information.
 */
TEST(Status, Get_SuccessMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_SUCCESS,
		NULL);
	assertValidMessage(message);
	assertContains(message, "successful");
	free((void*)message);
}

/**
 * Check that a message is returned for an 'insufficient memory' status
 * contains the correct information.
 */
TEST(Status, Get_InsufficientMemoryMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY,
		NULL);
	assertValidMessage(message);
	assertContains(message, "Insufficient memory");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'corrupt data' status and contains
 * the correct information.
 */
TEST(Status, Get_CorruptDataMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_CORRUPT_DATA,
		NULL);
	assertValidMessage(message);
	assertContains(message, "not in the correct format");
	free((void*)message);
}

/**
 * Check that a message is returned for an 'incorrect version' status and
 * contains the correct information.
 */
TEST(Status, Get_IncorrectVersionMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_INCORRECT_VERSION,
		NULL);
	assertValidMessage(message);
	assertContains(message, "unsupported version");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'file not found' status and contains
 * the correct information, including the name of the file which was passed to
 * the method.
 */
TEST(Status, Get_FileNotFoundMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND,
		fileName);
	assertValidMessage(message);
	assertContains(message, fileName);
	assertContains(message, "could not be found");
	assertContains(message, "read permissions");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'file not found' status, where null
 * is passed as the file name, and contains the correct information, including
 * the string 'null' as the file name.
 */
TEST(Status, Get_FileNotFoundMessage_NullFile) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND,
		NULL);
	assertValidMessage(message);
	assertContains(message, "null");
	assertContains(message, "could not be found");
	assertContains(message, "read permissions");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'not set' status and contains the
 * correct information. This is a status which will never be returned to the
 * user level, but should still be tested.
 */
TEST(Status, Get_NotSetMessage) {
	char code[3];
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_NOT_SET,
		NULL);
	assertValidMessage(message);
	Snprintf(code, 3, "%d", FIFTYONE_DEGREES_STATUS_NOT_SET);
	assertContains(message, code);
	free((void*)message);
}

/**
 * Check that a message is returned for a 'pointer out of bound's status and
 * contains the correct information.
 */
TEST(Status, Get_PointerOutOfBoundsMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_POINTER_OUT_OF_BOUNDS,
		NULL);
	assertValidMessage(message);
	assertContains(message, "smaller than expected");
	assertContains(message, "not fully loaded");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'null pointer' status and contains
 * the correct information.
 */
TEST(Status, Get_NullPointerMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_NULL_POINTER,
		NULL);
	assertValidMessage(message);
	assertContains(message, "Null pointer");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'too many files' status and contains
 * the correct information.
 */
TEST(Status, Get_TooManyFilesMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_TOO_MANY_OPEN_FILES,
		NULL);
	assertValidMessage(message);
	assertContains(message, "Too many file handles");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'properties not present' status and
 * contains the correct information.
 */
TEST(Status, Get_PropertiesNotPresentMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_REQ_PROP_NOT_PRESENT,
		NULL);
	assertValidMessage(message);
	assertContains(message, "None of the properties requested could be found");
	assertContains(message, "set the field to null");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'profile empty' status and contains
 * the correct information.
 */
TEST(Status, Get_ProfileEmptyMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_PROFILE_EMPTY,
		NULL);
	assertValidMessage(message);
	assertContains(message, "empty profile");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'collection failure' status and
 * contains the correct information.
 */
TEST(Status, Get_CollectionFailureMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_COLLECTION_FAILURE,
		NULL);
	assertValidMessage(message);
	assertContains(message, "getting an item from a collection");
	assertContains(message, "too many concurrent operations");
	assertContains(message, "Increase the concurrency");
	free((void*)message);
}

/**
 * Check that a message is returned for a 'encoding error' status and contains
 * the correct information.
 */
TEST(Status, Get_EncodingErrorMessage) {
	const char *message = fiftyoneDegreesStatusGetMessage(
		FIFTYONE_DEGREES_STATUS_ENCODING_ERROR,
		NULL);
	assertValidMessage(message);
	assertContains(message, "encoding characters of the string");
	free((void*)message);
}

/**
 * Check that a message is still returned for a status code which does not
 * exist and the message contains the code as a string.
 */
TEST(Status, Get_NonExistantCode) {
	char code[5];
	fiftyoneDegreesStatusCode invalidStatus = (fiftyoneDegreesStatusCode)1000;
	const char *message = fiftyoneDegreesStatusGetMessage(invalidStatus, NULL);
	assertValidMessage(message);
	Snprintf(code, 5, "%d", invalidStatus);
	assertContains(message, code);
	free((void*)message);
}

/**
 * Check that a message is still returned for a negative status code which does
 * not (and cannot) exist and the message contains the code as a string.
 */
TEST(Status, Get_NegativeCode) {
	char code[5];
	fiftyoneDegreesStatusCode invalidStatus = (fiftyoneDegreesStatusCode)-1;
	const char *message = fiftyoneDegreesStatusGetMessage(invalidStatus, NULL);
	assertValidMessage(message);
	Snprintf(code, 3, "%d", invalidStatus);
	assertContains(message, code);
	free((void*)message);
}

/**
 * Check that a message is still returned for a negative status code which does
 * not (and cannot) exist and the message contains the code as a string.
 */
TEST(Status, Get_InvalidInput) {
    const char *message = fiftyoneDegreesStatusGetMessage(
                                                          FIFTYONE_DEGREES_STATUS_INVALID_INPUT,
                                                          NULL);
    assertValidMessage(message);
    assertContains(message, "invalid");
    assertContains(message, "input value");
    free((void*)message);
}
