/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
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

#include <sstream>
#include "ValueMetaDataBuilderIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "constantsIpi.h"
#include "fiftyone.h"
#include "common-cxx/string_pp.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

/* Maximum buffer length to hold an IP address string */
#define IP_ADDRESS_STRING_MAX_LENGTH 50
/* Coordinate floating point precision */
#define COORDINATE_PRECISION 7

/**
 * Get the string representation of the data stored in
 * the strings collection
 * @param stringsCollection the string collection
 * @param offset the offset in the string collection
 * @param storedValueType format of byte array representation.
 */
static string getDynamicString(
	fiftyoneDegreesCollection *stringsCollection,
	uint32_t offset,
	fiftyoneDegreesPropertyValueType storedValueType) {
	EXCEPTION_CREATE;
	string result;
	Item item;
	const StoredBinaryValue *binaryValue;
	DataReset(&item.data);
	binaryValue = StoredBinaryValueGet(
		stringsCollection,
		offset,
		storedValueType,
		&item,
		exception);
	EXCEPTION_THROW;

	stringstream stream;
	if (binaryValue != nullptr && EXCEPTION_OKAY) {
		writeStoredBinaryValueToStringStream(
			binaryValue,
			storedValueType,
			stream,
			DefaultWktDecimalPlaces,
			exception);
		FIFTYONE_DEGREES_COLLECTION_RELEASE(
			stringsCollection,
			&item);
	}
	return stream.str();
}



ValueMetaData* ValueMetaDataBuilderIpi::build(
	fiftyoneDegreesDataSetIpi *dataSet,
	const fiftyoneDegreesValue *value) {
	EXCEPTION_CREATE;
	ValueMetaData *result = nullptr;
	Item item;
	Property *property;
	DataReset(&item.data);
	property = PropertyGet(
		dataSet->properties,
		value->propertyIndex, 
		&item,
		exception);
	EXCEPTION_THROW;
	PropertyValueType const storedValueType = PropertyGetStoredTypeByIndex(
		dataSet->propertyTypes,
		value->propertyIndex,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		result = new ValueMetaData(
			ValueMetaDataKey(
				getValue(
					dataSet->strings,
					property->nameOffset,
					FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING), // name is string
				getDynamicString(
					dataSet->strings,
					value->nameOffset,
					storedValueType)),
			value->descriptionOffset == -1 ?
			"" :
			getValue(
				dataSet->strings,
				value->descriptionOffset,
				FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING), // description is string
			value->urlOffset == -1 ?
			"" :
			getValue(
				dataSet->strings,
				value->urlOffset,
				FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING)); // URL is string
		COLLECTION_RELEASE(dataSet->properties, &item);
	}
	return result;
}
