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

#include "PropertyMetaData.hpp"

using namespace FiftyoneDegrees::Common;

PropertyMetaData::PropertyMetaData()
	: PropertyMetaData::PropertyMetaData(
		"invalid", // name
		vector<string>(), // data files
		"invalid", // type
		"invalid", // category
		"invalid", // url
		false, // available
		static_cast<byte>(-1), // display order
		false, // mandatory
		false, // list
		false, // obsolete
		false, // show
		false, // show values
		"invalid", // description
		"invalid", // default value,
		static_cast<byte>(-1), // component id
		vector<uint32_t>()
	) { }

PropertyMetaData::PropertyMetaData(PropertyMetaData *property)
	: PropertyMetaData::PropertyMetaData(
		property->getName(),
		property->dataFilesWherePresent,
		property->type,
		property->category,
		property->url,
		property->available,
		property->displayOrder,
		property->isMandatory,
		property->isList,
		property->isObsolete,
		property->show,
		property->showValues,
		property->description,
		property->defaultValue,
		property->componentId,
		property->evidenceProperties) {
}

PropertyMetaData::PropertyMetaData(
	string name,
	vector<string> dataFilesWherePresent,
	string type,
	string category,
	string url,
	bool available,
	byte displayOrder,
	bool isMandatory,
	bool isList,
	bool isObsolete,
	bool show,
	bool showValues,
	string description,
	string defaultValue,
	byte componentId,
	vector<uint32_t> evidenceProperties) : EntityMetaData(name) {
	this->dataFilesWherePresent = dataFilesWherePresent;
	this->type = type;
	this->category = category;
	this->url = url;
	this->available = available;
	this->displayOrder = (int)displayOrder;
	this->isMandatory = isMandatory;
	this->isList = isList;
	this->isObsolete = isObsolete;
	this->show = show;
	this->showValues = showValues;
	this->description = description;
	this->defaultValue = defaultValue;
	this->componentId = componentId;
	this->evidenceProperties = evidenceProperties;
}

string PropertyMetaData::getName() const {
	return getKey();
}

vector<string> PropertyMetaData::getDataFilesWherePresent() const {
	return dataFilesWherePresent;
}

string PropertyMetaData::getType() const {
	return type;
}

string PropertyMetaData::getCategory() const {
	return category;
}

string PropertyMetaData::getUrl() const {
	return url;
}

bool PropertyMetaData::getAvailable() const {
	return available;
}

int PropertyMetaData::getDisplayOrder() const {
	return (int)displayOrder;
}

bool PropertyMetaData::getIsMandatory() const {
	return isMandatory;
}

bool PropertyMetaData::getIsList() const {
	return isList;
}

bool PropertyMetaData::getIsObsolete() const {
	return isObsolete;
}

bool PropertyMetaData::getShow() const {
	return show;
}

bool PropertyMetaData::getShowValues() const {
	return showValues;
}

string PropertyMetaData::getDescription() const {
	return description;
}

byte PropertyMetaData::getComponentId() const {
	return componentId;
}

string PropertyMetaData::getDefaultValue() const {
	return defaultValue;
}

vector<uint32_t> PropertyMetaData::getEvidenceProperties() const {
	return evidenceProperties;
}