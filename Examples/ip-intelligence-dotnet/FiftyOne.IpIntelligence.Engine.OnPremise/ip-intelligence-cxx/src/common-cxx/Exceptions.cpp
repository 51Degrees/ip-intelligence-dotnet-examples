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

#include "Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::Common;

StatusCodeException::StatusCodeException() {}

StatusCodeException::StatusCodeException(
	fiftyoneDegreesStatusCode code) : exception() {
	const char *statusMessage = StatusGetMessage(code, nullptr);
	if (statusMessage != nullptr) {
		message.append(statusMessage);
		Free((void*)statusMessage);
		this->statusCode = code;
	}
}

StatusCodeException::StatusCodeException(
	fiftyoneDegreesStatusCode code,
	const char *fileName) : exception() {
	const char *statusMessage = StatusGetMessage(code, fileName);
	if (statusMessage != nullptr) {
		message.append(statusMessage);
		Free((void*)statusMessage);
		this->statusCode = code;
	}
}
StatusCodeException::~StatusCodeException() {}

const char* StatusCodeException::what() const noexcept {
	return message.c_str();
}

fiftyoneDegreesStatusCode StatusCodeException::getCode() const noexcept {
	return statusCode;
}

#ifndef FIFTYONE_DEGREES_EXCEPTIONS_DISABLED 
FatalException::FatalException(
    fiftyoneDegreesException *exception) : StatusCodeException(exception->status) {
	const char *exceptionMessage = ExceptionGetMessage(exception);
	if (exceptionMessage != nullptr) {
		message.append(exceptionMessage);
		Free((void*)exceptionMessage);
	}
}
#else 
FatalException::FatalException(
	fiftyoneDegreesException* exception) : StatusCodeException(NOT_SET) {
	message.append(
		"Exceptions disabled with FIFTYONE_DEGREES_EXCEPTIONS_DISABLED");
}
#endif

NotImplementedException::NotImplementedException() 
	: runtime_error("Function not yet implemented") { 
}

InvalidPropertyException::InvalidPropertyException()
	: runtime_error("Requested property is not available") {
}

InvalidPropertyException::InvalidPropertyException(const char *message)
	: runtime_error(message) {
}

NoValuesAvailableException::NoValuesAvailableException()
	: NoValuesAvailableException(nullptr) {
}

NoValuesAvailableException::NoValuesAvailableException(const char *message)
	: runtime_error(message == nullptr ?
		"No values are available for the property requested" : message) {
}

TooManyValuesException::TooManyValuesException() :	runtime_error(
	"Type conversation only available for properties with a single value") {
}

EvidenceException::EvidenceException() : 
	runtime_error("Could not create usable evidence from keys provided") {
}