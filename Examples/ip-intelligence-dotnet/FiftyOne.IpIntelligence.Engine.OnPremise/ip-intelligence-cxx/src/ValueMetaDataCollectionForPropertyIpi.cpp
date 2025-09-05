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

#include "ValueMetaDataCollectionForPropertyIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

ValueMetaDataCollectionForPropertyIpi::ValueMetaDataCollectionForPropertyIpi(
	fiftyoneDegreesResourceManager *manager,
	PropertyMetaData *property) : ValueMetaDataCollectionBaseIpi(manager) {
	EXCEPTION_CREATE;
	DataReset(&propertyItem.data);
	PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		property->getName().c_str(),
		&propertyItem,
		exception);
	EXCEPTION_THROW;
}

ValueMetaDataCollectionForPropertyIpi::~ValueMetaDataCollectionForPropertyIpi() {
	COLLECTION_RELEASE(dataSet->properties, &propertyItem);
}

ValueMetaData* ValueMetaDataCollectionForPropertyIpi::getByIndex(
	uint32_t index) const {
	EXCEPTION_CREATE;
	Item item;
	const Value *value;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	value = ValueGet(
		dataSet->values, 
		getProperty()->firstValueIndex + index, 
		&item,
		exception);
	EXCEPTION_THROW;
	if (value != nullptr) {
		result = ValueMetaDataBuilderIpi::build(dataSet, value);
		COLLECTION_RELEASE(dataSet->values, &item);
	}
	return result;
}

ValueMetaData* ValueMetaDataCollectionForPropertyIpi::getByKey(
	ValueMetaDataKey key) const {
	EXCEPTION_CREATE;
	Item item;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	const String * const name = PropertyGetName(
		dataSet->strings, 
		getProperty(), 
		&item, 
		exception);
	EXCEPTION_THROW;
	if (name != nullptr) {
		result = ValueMetaDataCollectionBaseIpi::getByKey(key);
		if (result != nullptr &&
			strcmp(result->getKey().getPropertyName().c_str(), 
				&name->value) != 0) {
			delete result;
			result = nullptr;
		}
		dataSet->strings->release(&item);
	}
	return result;
}

uint32_t ValueMetaDataCollectionForPropertyIpi::getSize() const {
	const Property* const property = getProperty();
	return (int)property->firstValueIndex == -1 ?
		0 :
		property->lastValueIndex - property->firstValueIndex + 1;
}

fiftyoneDegreesProperty* ValueMetaDataCollectionForPropertyIpi::getProperty()
const {
	return (Property*)propertyItem.data.ptr;
}