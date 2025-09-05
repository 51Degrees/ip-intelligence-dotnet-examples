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

#include "ResultsIpi.hpp"
#include "fiftyone.h"
#include "common-cxx/wkbtot_pp.hpp"
#include "constantsIpi.h"
#include "common-cxx/string_pp.hpp"

using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

#define RESULT(r,i) ((ResultIpi*)r->b.items + i)

IpIntelligence::ResultsIpi::ResultsIpi(
	fiftyoneDegreesResultsIpi *results,
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: ResultsBase(&results->b, manager) {
	this->results = results;
}


IpIntelligence::ResultsIpi::~ResultsIpi() {
	ResultsIpiFree(results);
}

/* 
 * This is used to hold the string value of the item
 * Maximum size of an IP address string is 39
 * Maximum single precision floating point is ~ 3.4 * 10^38
 * 128 should be adequate to hold the string value
 * for a pair of coordinate:percentage
 * or ipaddress:percentage
 */
#define MAX_PROFILE_PERCENTAGE_STRING_LENGTH 128

/*
 * This will returns the profile percentages results
 * for a required property index in string form.
 * @param requiredPropertyIndex the required property index
 * @param values the array which will hold the returned value string
 */
void 
IpIntelligence::ResultsIpi::getValuesInternal(int requiredPropertyIndex, vector<string> &values) {
    EXCEPTION_CREATE;
	uint32_t i;
	const ProfilePercentage *valuesItems;

    // We should not have any undefined data type in the data file
    // If there is, the data file is not good to use so terminates.
    getPropertyValueType(requiredPropertyIndex, exception);
    EXCEPTION_THROW;

	// Get a pointer to the first value item for the property.
	valuesItems = ResultsIpiGetValues(
		results,
		requiredPropertyIndex,
		exception);
	EXCEPTION_THROW;

	if (valuesItems == NULL) {
		// No pointer to values was returned. 
		throw NoValuesAvailableException();
	}

    const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;
    const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
        dataSet->b.b.available,
        requiredPropertyIndex);
    const PropertyValueType storedValueType = PropertyGetStoredTypeByIndex(
        dataSet->propertyTypes,
        propertyIndex,
        exception);

	// Set enough space in the vector for all the strings that will be 
	// inserted.
	values.reserve(results->values.count);

    stringstream stream;
	// Add the values in their original form to the result.
	for (i = 0; i < results->values.count; i++) {
        // Clear the string stream
        stream.str("");
	    const StoredBinaryValue * const binaryValue = (StoredBinaryValue *)valuesItems[i].item.data.ptr;
	    writeStoredBinaryValueToStringStream(
	        binaryValue,
	        storedValueType,
	        stream,
	        MAX_DOUBLE_DECIMAL_PLACES,
	        exception);
        if (EXCEPTION_OKAY) {
            stream << ":";
            stream << (float)valuesItems[i].rawWeighting / 65535.f;
            values.push_back(stream.str());
        } else {
            break;
        }
	}
    // The value format in the data file should never be
    // in incorrect. If it happens the data file or
    // the memory is corrupted and we should terminate
    EXCEPTION_THROW
}

fiftyoneDegreesPropertyValueType
IpIntelligence::ResultsIpi::getPropertyValueType(
    int requiredPropertyIndex,
    fiftyoneDegreesException *exception) {
    // Default to string type. Consumers of
    // this function should always check for exception status
    fiftyoneDegreesPropertyValueType valueType
        = FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING; // overwritten later
    DataSetIpi *dataSet = (DataSetIpi*)results->b.dataSet;

	// Work out the property index from the required property index.
	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	// Set the property that will be available in the results structure. 
	// This may also be needed to work out which of a selection of results 
	// are used to obtain the values.
	Property *property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);
    if (property != NULL && EXCEPTION_OKAY) {
        valueType = (fiftyoneDegreesPropertyValueType)property->valueType;
    }
    return valueType;
}

Common::Value<IpIntelligence::IpAddress>
IpIntelligence::ResultsIpi::getValueAsIpAddress(int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    const ProfilePercentage *valuesItems;
    Common::Value<IpAddress> result;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
            const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;
            const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
                dataSet->b.b.available,
                requiredPropertyIndex);
            const PropertyValueType storedValueType = PropertyGetStoredTypeByIndex(
                dataSet->propertyTypes,
                propertyIndex,
                exception);
            EXCEPTION_THROW;

            if (storedValueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS) {
                // Get a pointer to the first value item for the property.
                valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
                EXCEPTION_THROW;
                
                if (valuesItems == NULL) {
                    // No pointer to values was returned.
                    throw NoValuesAvailableException();
                }
                
                // Add the values in their original form to the result.
                if (results->values.count > 1) {
                    result.setNoValueReason(
                        FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
                        nullptr);
                }
                else {
                    IpAddress ipAddress;
                    const VarLengthByteArray * const rawIpAddress = (const VarLengthByteArray *)valuesItems->item.data.ptr;
                    const unsigned char * const ipAddressBytes =
                        &rawIpAddress->firstByte;
                    const IpType rawIpType = (rawIpAddress->size == IPV4_LENGTH) ? IP_TYPE_IPV4 :
                    ((rawIpAddress->size == IPV6_LENGTH) ? IP_TYPE_IPV6
                        : IP_TYPE_INVALID);
                    if (ipAddressBytes != NULL) {
                        ipAddress = IpAddress(
                            ipAddressBytes, rawIpType);
                    }
                    result.setValue(ipAddress);
                }
            }
            else {
                // Default to the smallest IP address
                if (results->items[0].type == IP_TYPE_IPV4) {
                    result.setValue(IpAddress("0.0.0.0"));
                }
                else {
                    result.setValue(IpAddress("0000:0000:0000:0000:0000:0000:0000:0000"));
                }
            }
        }
    }
    return result;
}

Common::Value<IpIntelligence::IpAddress>
IpIntelligence::ResultsIpi::getValueAsIpAddress(const char *propertyName) {
    return getValueAsIpAddress(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<IpIntelligence::IpAddress>
IpIntelligence::ResultsIpi::getValueAsIpAddress(const string &propertyName) {
    return getValueAsIpAddress(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<IpIntelligence::IpAddress>
IpIntelligence::ResultsIpi::getValueAsIpAddress(const string *propertyName) {
    return getValueAsIpAddress(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

/*
 * Override the default getValueAsBool function.
 * Since for each property, we will always get a list of profile percentage pairs,
 * it is not appropriate to process the value as boolean here.
 * Thus always return #FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES
 */
Common::Value<bool> 
IpIntelligence::ResultsIpi::getValueAsBool(int requiredPropertyIndex) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(requiredPropertyIndex);
#	endif
    Common::Value<bool> result;
    result.setNoValueReason(
        FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
        nullptr);
    return result;
}

/*
 * Override the default getValueAsInteger function.
 * Since for each property, we will always get a list of profile percentage pairs,
 * it is not appropriate to process the value as integer here.
 * Thus always return #FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES
 */
Common::Value<int>
IpIntelligence::ResultsIpi::getValueAsInteger(int requiredPropertyIndex) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(requiredPropertyIndex);
#	endif
    Common::Value<int> result;
    result.setNoValueReason(
        FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
        nullptr);
    return result;
}

/*
 * Override the default getValueAsDouble function.
 * Since for each property, we will always get a list of profile percentage pairs,
 * it is not appropriate to process the value as double here.
 * Thus always return #FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES
 */
Common::Value<double> 
IpIntelligence::ResultsIpi::getValueAsDouble(int requiredPropertyIndex) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(requiredPropertyIndex);
#	endif
    Common::Value<double> result;
    result.setNoValueReason(
        FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
        nullptr);
    return result;
}

Common::Value<vector<WeightedValue<bool>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedBoolList(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<bool>>> result;
    vector<WeightedValue<bool>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
            // Get a pointer to the first value item for the property.
            valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
            EXCEPTION_THROW;

            if (valuesItems == NULL) {
                // No pointer to values was returned.
                throw NoValuesAvailableException();
            }

            const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;
            const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
                dataSet->b.b.available,
                requiredPropertyIndex);
            const PropertyValueType storedValueType = PropertyGetStoredTypeByIndex(
                dataSet->propertyTypes,
                propertyIndex,
                exception);
            EXCEPTION_THROW;

            // Set enough space in the vector for all the strings that will be
            // inserted.
            values.reserve(results->values.count);

            // Add the values in their original form to the result.
            for (i = 0; i < results->values.count; i++) {
                WeightedValue<bool> weightedBool;
                weightedBool.setValue(StoredBinaryValueToBoolOrDefault(
                    (const StoredBinaryValue *)valuesItems[i].item.data.ptr,
                    storedValueType,
                    false));
                weightedBool.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedBool);
            }
            result.setValue(values);
        }
    }
    return result;
}

Common::Value<vector<WeightedValue<bool>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedBoolList(
    const char *propertyName) {
  return getValuesAsWeightedBoolList(
      ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<bool>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedBoolList(
    const string &propertyName) {
  return getValuesAsWeightedBoolList(
      ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<bool>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedBoolList(
    const string *propertyName) {
  return getValuesAsWeightedBoolList(
      ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedStringList(
    const int requiredPropertyIndex) {

    vector<WeightedValue<string>> values;
    Common::Value<vector<WeightedValue<string>>> result;
    stringstream stream;
    iterateWeightedValues(
        requiredPropertyIndex,
        [this, &result](const fiftyoneDegreesResultsNoValueReason reason, const char * const reasonStr) {
            result.setNoValueReason(reason, reasonStr);
        },
        [&values](const uint32_t count) {
            values.reserve(count);
        },
        [&values, &stream](
            const StoredBinaryValue * const binaryValue,
            const PropertyValueType storedValueType,
            const uint16_t rawWeighting,
            Exception * const exception) {
            WeightedValue<string> weightedString;
            // Clear stream before the construction
            stream.str("");
            writeStoredBinaryValueToStringStream(
                binaryValue,
                storedValueType,
                stream,
                DefaultWktDecimalPlaces,
                exception);
            EXCEPTION_THROW;
            weightedString.setValue(stream.str());
            weightedString.setRawWeight(rawWeighting);
            values.push_back(weightedString);
        },
        [&result, &values] {
            result.setValue(values);
        });
    return result;
}

void IpIntelligence::ResultsIpi::iterateWeightedValues(
    int requiredPropertyIndex,
    const std::function<void(
        fiftyoneDegreesResultsNoValueReason reason,
        const char *reasonStr)>& onNoValue,
    const std::function<void(uint32_t count)>& onValuesCount,
    const std::function<void(
        const StoredBinaryValue *binaryValue,
        PropertyValueType storedValueType,
        uint16_t rawWeighting,
        Exception *exception)>& onEachValue,
    const std::function<void()>& onAfterValues) {

    EXCEPTION_CREATE;
    uint32_t i;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		onNoValue(reason, getNoValueMessageInternal(reason));
    }
    else {
        const DataSetIpi * const dataSet = static_cast<DataSetIpi *>(results->b.dataSet);
        const uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
            dataSet->b.b.available,
            requiredPropertyIndex);
        fiftyoneDegreesPropertyValueType storedValueType = PropertyGetStoredTypeByIndex(
            dataSet->propertyTypes,
            propertyIndex,
            exception);
        EXCEPTION_THROW;
        // Get a pointer to the first value item for the property.
        const ProfilePercentage * const valuesItems = ResultsIpiGetValues(
            results,
            requiredPropertyIndex,
            exception);
        EXCEPTION_THROW;

        if (valuesItems == nullptr) {
            // No pointer to values was returned.
            throw NoValuesAvailableException();
        }

        // Set enough space in the vector for all the strings that will be
        // inserted.
        onValuesCount(results->values.count);

        // Add the values in their original form to the result.
        for (i = 0; i < results->values.count; i++) {
            onEachValue(
                reinterpret_cast<const StoredBinaryValue *>(valuesItems[i].item.data.ptr),
                storedValueType,
                valuesItems[i].rawWeighting,
                exception);
        }
        onAfterValues();
    }
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedStringList(
    const char *propertyName) {
    return getValuesAsWeightedStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<std::vector<uint8_t>>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedUTF8StringList(const string &propertyName) {
    return getValuesAsWeightedUTF8StringList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<std::vector<uint8_t>>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedUTF8StringList(
    const char *propertyName) {
    return getValuesAsWeightedUTF8StringList(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<std::vector<uint8_t>>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedUTF8StringList(
    int requiredPropertyIndex) {

    vector<WeightedValue<std::vector<uint8_t>>> values;
    Common::Value<vector<WeightedValue<std::vector<uint8_t>>>> result;
    stringstream stream;
    std::vector<uint8_t> byteVector;
    iterateWeightedValues(
        requiredPropertyIndex,
        [this, &result](const fiftyoneDegreesResultsNoValueReason reason, const char * const reasonStr) {
            result.setNoValueReason(reason, reasonStr);
        },
        [&values](const uint32_t count) {
            values.reserve(count);
        },
        [&values, &stream, &byteVector](
            const StoredBinaryValue * const binaryValue,
            const PropertyValueType storedValueType,
            const uint16_t rawWeighting,
            Exception * const exception) {
            WeightedValue<std::vector<uint8_t>> weightedByteVector;
            if (storedValueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING) {
                const String * const rawString = reinterpret_cast<const String *>(binaryValue);
                byteVector.reserve(rawString->size);
                if (rawString->size) {
                    const uint8_t * const firstByte = reinterpret_cast<const uint8_t *>(&rawString->value);
                    const uint8_t * pastLastByte = firstByte + rawString->size;
                    // strip NUL-terminator
                    if (!*(pastLastByte - 1)) {
                        --pastLastByte;
                    }
                    byteVector.assign(firstByte, pastLastByte);
                }
            } else {
                // Clear stream before the construction
                stream.str("");
                writeStoredBinaryValueToStringStream(
                    binaryValue,
                    storedValueType,
                    stream,
                    DefaultWktDecimalPlaces,
                    exception);
                EXCEPTION_THROW;
                const std::string valueAsString = stream.str();
                byteVector.reserve(valueAsString.size());
                const uint8_t * const firstByte = reinterpret_cast<const uint8_t *>(valueAsString.c_str());
                byteVector.assign(firstByte, firstByte + valueAsString.size());
            }
            weightedByteVector.setValue(byteVector);
            weightedByteVector.setRawWeight(rawWeighting);
            values.push_back(weightedByteVector);
        },
        [&result, &values] {
            result.setValue(values);
        });
    return result;
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedStringList(
    const string &propertyName) {
    return getValuesAsWeightedStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedStringList(
    const string *propertyName) {
    return getValuesAsWeightedStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedWKTStringList(
    const int requiredPropertyIndex,
    const byte decimalPlaces) {
    EXCEPTION_CREATE;
    uint32_t i;
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<string>>> result;
    vector<WeightedValue<string>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
            // Get a pointer to the first value item for the property.
            valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
            EXCEPTION_THROW;

            if (valuesItems == nullptr) {
                // No pointer to values was returned.
                throw NoValuesAvailableException();
            }

            // Set enough space in the vector for all the strings that will be
            // inserted.
            values.reserve(results->values.count);

            stringstream stream;
            // Add the values in their original form to the result.
            for (i = 0; i < results->values.count; i++) {
                WeightedValue<string> weightedString;
                // Clear stream before the construction
                stream.str("");
                const StoredBinaryValue * const binaryValue = (StoredBinaryValue *)valuesItems[i].item.data.ptr;
                writeStoredBinaryValueToStringStream(
                    binaryValue,
                    FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WKB,
                    stream,
                    decimalPlaces,
                    exception);
                EXCEPTION_THROW;
                weightedString.setValue(stream.str());
                weightedString.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedString);
            }
            result.setValue(values);
        }
    }
    return result;
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedWKTStringList(
const char *propertyName,
byte decimalPlaces) {
    return getValuesAsWeightedWKTStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName),
        decimalPlaces);
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedWKTStringList(
const string &propertyName,
byte decimalPlaces) {
    return getValuesAsWeightedWKTStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()),
        decimalPlaces);
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedWKTStringList(
const string *propertyName,
byte decimalPlaces) {
    return getValuesAsWeightedWKTStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()),
        decimalPlaces);
}

Common::Value<vector<WeightedValue<int>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedIntegerList(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<int>>> result;
    vector<WeightedValue<int>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
            // Get a pointer to the first value item for the property.
            valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
            EXCEPTION_THROW;

            if (valuesItems == NULL) {
                // No pointer to values was returned.
                throw NoValuesAvailableException();
            }

            const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;
            const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
                dataSet->b.b.available,
                requiredPropertyIndex);
            const PropertyValueType storedValueType = PropertyGetStoredTypeByIndex(
                dataSet->propertyTypes,
                propertyIndex,
                exception);
            EXCEPTION_THROW;

            // Set enough space in the vector for all the strings that will be
            // inserted.
            values.reserve(results->values.count);

            // Add the values in their original form to the result.
            for (i = 0; i < results->values.count; i++) {
                WeightedValue<int> weightedInteger;
                weightedInteger.setValue(StoredBinaryValueToIntOrDefault(
                    (const StoredBinaryValue *)valuesItems[i].item.data.ptr,
                    storedValueType,
                    0));
                weightedInteger.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedInteger);
            }
            result.setValue(values);
        }
    }
    return result;
}

Common::Value<vector<WeightedValue<int>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedIntegerList(
    const char *propertyName) {
    return getValuesAsWeightedIntegerList(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<int>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedIntegerList(
    const string &propertyName) {
    return getValuesAsWeightedIntegerList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<int>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedIntegerList(
    const string *propertyName) {
    return getValuesAsWeightedIntegerList(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<double>>> result;
    vector<WeightedValue<double>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
            // Get a pointer to the first value item for the property.
            valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
            EXCEPTION_THROW;

            if (valuesItems == NULL) {
                // No pointer to values was returned.
                throw NoValuesAvailableException();
            }

            const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;
            const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
                dataSet->b.b.available,
                requiredPropertyIndex);
            const PropertyValueType storedValueType = PropertyGetStoredTypeByIndex(
                dataSet->propertyTypes,
                propertyIndex,
                exception);
            EXCEPTION_THROW;

            // Set enough space in the vector for all the strings that will be
            // inserted.
            values.reserve(results->values.count);

            // Add the values in their original form to the result.
            for (i = 0; i < results->values.count; i++) {
                WeightedValue<double> weightedDouble;
                weightedDouble.setValue(StoredBinaryValueToDoubleOrDefault(
                    (const StoredBinaryValue *)valuesItems[i].item.data.ptr,
                    storedValueType,
                    0));
                weightedDouble.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedDouble);
            }
            result.setValue(values);
        }
    }
    return result;
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    const char *propertyName) {
    return getValuesAsWeightedDoubleList(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    const string &propertyName) {
    return getValuesAsWeightedDoubleList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    const string *propertyName) {
    return getValuesAsWeightedDoubleList(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

bool IpIntelligence::ResultsIpi::hasValuesInternal(
	int requiredPropertyIndex) {
	EXCEPTION_CREATE;
	bool hasValues = fiftyoneDegreesResultsIpiGetHasValues(
		results,
		requiredPropertyIndex,
		exception);
	EXCEPTION_THROW;
	return hasValues;
}

const char* IpIntelligence::ResultsIpi::getNoValueMessageInternal(
	fiftyoneDegreesResultsNoValueReason reason) {
	return fiftyoneDegreesResultsIpiGetNoValueReasonMessage(reason);
}

fiftyoneDegreesResultsNoValueReason
IpIntelligence::ResultsIpi::getNoValueReasonInternal(
	int requiredPropertyIndex) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsNoValueReason reason =
		fiftyoneDegreesResultsIpiGetNoValueReason(
			results,
			requiredPropertyIndex,
			exception);
	EXCEPTION_THROW;
	return reason;
}
