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

#include "MetaData.hpp"

#include <sstream>

#include "Exceptions.hpp"
#include "fiftyone.h"
#include "string_pp.hpp"

using namespace FiftyoneDegrees::Common;

MetaData::MetaData(shared_ptr<fiftyoneDegreesResourceManager> manager) {
	this->manager = manager;
}

MetaData::~MetaData() {
}

string MetaData::getValue(
	fiftyoneDegreesCollection *strings,
	uint32_t offset,
	fiftyoneDegreesPropertyValueType storedValueType) {
	EXCEPTION_CREATE;
	Item item;
	const StoredBinaryValue *binaryValue;
	DataReset(&item.data);
	binaryValue = StoredBinaryValueGet(
		strings,
		offset,
		storedValueType,
		&item,
		exception);
	EXCEPTION_THROW;
	std::stringstream ss;
	if (binaryValue != nullptr) {
		writeStoredBinaryValueToStringStream(
			binaryValue,
			storedValueType,
			ss,
			(uint8_t)ss.precision(),
			exception);
	}
	COLLECTION_RELEASE(strings, &item);
	return ss.str();
}