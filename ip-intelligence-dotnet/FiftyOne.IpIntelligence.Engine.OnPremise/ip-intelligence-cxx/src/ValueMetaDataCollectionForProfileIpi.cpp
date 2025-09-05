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

#include "ValueMetaDataCollectionForProfileIpi.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

struct FilterResult {
	DataSetIpi *dataSet = nullptr;
	string valueName;
	Value value {0, 0, 0, 0};
	bool found = false;
};

ValueMetaDataCollectionForProfileIpi::ValueMetaDataCollectionForProfileIpi(
	fiftyoneDegreesResourceManager *manager,
	ProfileMetaData *profile) : ValueMetaDataCollectionBaseIpi(manager) {
	EXCEPTION_CREATE;
	DataReset(&profileItem.data);
	ProfileGetByProfileIdIndirect(
		dataSet->profileOffsets,
		dataSet->profiles,
		profile->getProfileId(),
		&profileItem,
		exception);
	EXCEPTION_THROW;
}

ValueMetaDataCollectionForProfileIpi::~ValueMetaDataCollectionForProfileIpi() {
	COLLECTION_RELEASE(dataSet->profiles, &profileItem);
}

ValueMetaData* ValueMetaDataCollectionForProfileIpi::getByIndex(
	uint32_t index) const {
	EXCEPTION_CREATE;
	ValueMetaData *result = nullptr;
	const Value *value;
	Item item;
	DataReset(&item.data);
	uint32_t valueIndex = ((uint32_t*)(getProfile() + 1))[index];
	value = ValueGet(
		dataSet->values,
		valueIndex,
		&item,
		exception);
	EXCEPTION_THROW;
	if (value != nullptr) {
		result = ValueMetaDataBuilderIpi::build(dataSet, value);
		COLLECTION_RELEASE(dataSet->values, &item);
	}
	return result;
}

bool ValueMetaDataCollectionForProfileIpi::valueFilter(
	void *state, 
	fiftyoneDegreesCollectionItem *valueItem) {
	EXCEPTION_CREATE;
	Item nameItem;
	const StoredBinaryValue *valueContent;
	const Value *value;
	FilterResult *result = (FilterResult*)state;
	value = (const Value*)valueItem->data.ptr;
	PropertyValueType const storedValueType = PropertyGetStoredTypeByIndex(
		result->dataSet->propertyTypes,
		value->propertyIndex,
		exception);
	EXCEPTION_THROW;
	DataReset(&nameItem.data);
	valueContent = ValueGetContent(
		result->dataSet->strings,
		value,
		storedValueType,
		&nameItem,
		exception);
	EXCEPTION_THROW;
	if (valueContent != nullptr) {
		const size_t cmpSize = result->valueName.size() + 3;
		char * const buffer = (char *)Malloc(cmpSize);
		if (buffer) {
			StringBuilder builder{ buffer, cmpSize };
			StringBuilderInit(&builder);
			if ((StoredBinaryValueCompareWithString(
				valueContent,
				storedValueType,
				result->valueName.c_str(),
				&builder,
				exception) == 0) && EXCEPTION_OKAY) {
				memcpy(&result->value, value, sizeof(Value));
				result->found = true;
			}
			Free(buffer);
		}
		COLLECTION_RELEASE(result->dataSet->strings, &nameItem);
	}
	COLLECTION_RELEASE(result->dataSet->values, valueItem);
	return true;
}

ValueMetaData* ValueMetaDataCollectionForProfileIpi::getByKey(
	ValueMetaDataKey key) const {
	EXCEPTION_CREATE;
	Item propertyItem;
	const Property *property;
	uint32_t count;
	ValueMetaData *result = nullptr;
	FilterResult state;
	DataReset(&propertyItem.data);
	property = PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		key.getPropertyName().c_str(),
		&propertyItem,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		state.dataSet = dataSet;
		state.valueName = key.getValueName();
		count = ProfileIterateValuesForProperty(
			dataSet->values,
			getProfile(),
			property,
			&state,
			&valueFilter,
			exception);
		EXCEPTION_THROW;
		if (count > 0 && state.found == true) {
			result = ValueMetaDataBuilderIpi::build(dataSet, &state.value);
		}
	}
	return result;
}

uint32_t ValueMetaDataCollectionForProfileIpi::getSize() const {
	return getProfile()->valueCount;
}

fiftyoneDegreesProfile* 
ValueMetaDataCollectionForProfileIpi::getProfile() const {
	return (Profile*)profileItem.data.ptr;
}