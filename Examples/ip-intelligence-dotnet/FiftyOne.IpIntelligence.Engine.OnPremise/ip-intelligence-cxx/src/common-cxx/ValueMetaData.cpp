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

#include "ValueMetaData.hpp"
#include "PropertyMetaData.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::Common;

ValueMetaDataKey::ValueMetaDataKey()
    : ValueMetaDataKey::ValueMetaDataKey(
        "invalid", // property
        "invalid" // value
    ) { }

ValueMetaDataKey::ValueMetaDataKey(string propertyName, string valueName) {
	this->propertyName = propertyName;
	this->valueName = valueName;
}

ValueMetaDataKey::ValueMetaDataKey(
	const string &propertyName,
	const uint8_t *valueName,
	const long valueNameLength) {
	this->propertyName = propertyName;
	this->valueName = std::string((const char *)valueName, valueNameLength);
}

/**
 * Get the name of the property which the key relates to.
 * @return name of property
 */
const string ValueMetaDataKey::getPropertyName() const {
	return propertyName;
}

/**
 * Get the name of the value which the key relates to.
 * @return name of the value
 */
const string ValueMetaDataKey::getValueName() const {
	return valueName;
}

/**
 * Compare two keys for ordering. Keys are ordered first on property name, then
 * value name.
 * @param other key to compare
 * @return true if the other key comes before this one
 */
const bool ValueMetaDataKey::operator< (ValueMetaDataKey other) const {
	if (other.getPropertyName() == getPropertyName()) {
		// Keys are the same property, so compare the secondary key (value)
		return other.getValueName() < getValueName();
	}
	return other.getPropertyName() < getPropertyName();
}

/**
 * Return true if the two keys are equal. To be equal, both keys must have the
 * same property and value names.
 * @param other key to compare
 * @return true if equal
 */
const bool ValueMetaDataKey::operator== (ValueMetaDataKey other) const {
	return (other.getPropertyName() == getPropertyName()) &&
		(other.getValueName() == getValueName());
}

ValueMetaData::ValueMetaData()
    : ValueMetaData(
        ValueMetaDataKey("invalid", "invalid"), // key
        "invalid", // description
        "invalid" // url
    ) { }

ValueMetaData::ValueMetaData(ValueMetaData *value)
	: ValueMetaData(
		value->getKey(),
		value->description,
		value->url) {}

ValueMetaData::ValueMetaData(
	ValueMetaDataKey key,
	string description,
	string url) : EntityMetaData(key) {
	this->description = description;
	this->url = url;
}

string ValueMetaData::getName() const {
	return getKey().getValueName();
}

string ValueMetaData::getDescription() const {
	return description;
}

string ValueMetaData::getUrl() const {
	return url;
}

std::vector<uint8_t> ValueMetaData::getUtf8ValueName() const {
	const std::string& name = getName();
	return std::vector<uint8_t>(name.begin(), name.end());
}
