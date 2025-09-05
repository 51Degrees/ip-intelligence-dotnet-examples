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

#include "ResultsBase.hpp"

#include "fiftyone.h"

using namespace FiftyoneDegrees::Common;

ResultsBase::ResultsBase(
	fiftyoneDegreesResultsBase *results,
	shared_ptr<fiftyoneDegreesResourceManager> manager) {
	this->available = ((DataSetBase*)results->dataSet)->available;
	this->manager = manager;
}

ResultsBase::~ResultsBase() {
}

int ResultsBase::getAvailableProperties() const {
	return available->count;
}

bool ResultsBase::containsProperty(
	const string &propertyName) const {
	return PropertiesGetRequiredPropertyIndexFromName(
		available,
		propertyName.c_str()) >= 0;
}

vector<string> ResultsBase::getProperties() const {
	int i, size = getAvailableProperties();
	vector<string> result;
	for (i = 0; i < size; i++) {
		result.push_back(getPropertyName(i));
	}
	return result;
}

string ResultsBase::getPropertyName(
	int requiredPropertyIndex) const {
	string name;
	const char *cName;
	if (requiredPropertyIndex >= 0 &&
		requiredPropertyIndex < (int)available->count) {
		cName = STRING(PropertiesGetNameFromRequiredIndex( // name is string
			available,
			requiredPropertyIndex));
		if (cName != nullptr) {
			name.assign(cName);
		}
	}
	return name;
}

FiftyoneDegrees::Common::Value<string> ResultsBase::getValueAsString(int requiredPropertyIndex) {
	Value<string> result;
	if (hasValuesInternal(requiredPropertyIndex) == false) {
		fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
	}
	else {
		vector<string> values;
		getValuesInternal(requiredPropertyIndex, values);
		if (values.size() > 1) {
			stringstream stream;
			for (vector<string>::iterator it = values.begin();
				it != values.end();
				it++) {
				if (it != values.begin()) {
					stream << "|";
				}
				stream << *it;
			}
			result.setValue(stream.str());
		}
		else if (values.size() != 0) {
			result.setValue(*values.begin());
		}
	}
	return result;
}

FiftyoneDegrees::Common::Value<string> ResultsBase::getValueAsString(const char* propertyName) {
	return getValueAsString(getRequiredPropertyIndex(propertyName));
}

FiftyoneDegrees::Common::Value<string> ResultsBase::getValueAsString(const string &propertyName) {
	return getValueAsString(propertyName.c_str());
}

FiftyoneDegrees::Common::Value<string> ResultsBase::getValueAsString(const string *propertyName) {
	return getValueAsString(propertyName->c_str());
}

FiftyoneDegrees::Common::Value<bool> ResultsBase::getValueAsBool(int requiredPropertyIndex) {
	Value<bool> result;
	if (hasValuesInternal(requiredPropertyIndex) == false) {
		fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
	}
	else {
		vector<string> values;
		getValuesInternal(requiredPropertyIndex, values);
		if (values.size() > 1) {
			result.setNoValueReason(
				FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
				nullptr);
		}
		else if (values.size() != 0) {
			result.setValue((*values.begin()).compare("True") == 0);
		}
	}
	return result;
}

FiftyoneDegrees::Common::Value<bool> ResultsBase::getValueAsBool(const char* propertyName) {
	return getValueAsBool(getRequiredPropertyIndex(propertyName));
}

FiftyoneDegrees::Common::Value<bool> ResultsBase::getValueAsBool(const string &propertyName) {
	return getValueAsBool(propertyName.c_str());
}

FiftyoneDegrees::Common::Value<bool> ResultsBase::getValueAsBool(const string *propertyName) {
	return getValueAsBool(propertyName->c_str());
}

FiftyoneDegrees::Common::Value<int> ResultsBase::getValueAsInteger(int requiredPropertyIndex) {
	Value<int> result;
	if (hasValuesInternal(requiredPropertyIndex) == false) {
		fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
	}
	else {
		vector<string> values;
		getValuesInternal(requiredPropertyIndex, values);
		if (values.size() > 1) {
			result.setNoValueReason(
				FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
				nullptr);
		}
		else if (values.size() != 0) {
			result.setValue(atoi(values.begin()->c_str()));
		}
	}
	return result;
}

FiftyoneDegrees::Common::Value<int> ResultsBase::getValueAsInteger(const char* propertyName) {
	return getValueAsInteger(getRequiredPropertyIndex(propertyName));
}

FiftyoneDegrees::Common::Value<int> ResultsBase::getValueAsInteger(const string &propertyName) {
	return getValueAsInteger(propertyName.c_str());
}

FiftyoneDegrees::Common::Value<int> ResultsBase::getValueAsInteger(const string *propertyName) {
	return getValueAsInteger(propertyName->c_str());
}

FiftyoneDegrees::Common::Value<double> ResultsBase::getValueAsDouble(int requiredPropertyIndex) {
	Value<double> result;
	if (hasValuesInternal(requiredPropertyIndex) == false) {
		fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
	}
	else {
		vector<string> values;
		getValuesInternal(requiredPropertyIndex, values);
		if (values.size() > 1) {
			result.setNoValueReason(
				FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
				nullptr);
		}
		else if (values.size() != 0) {
			result.setValue(strtod(values.begin()->c_str(), nullptr));
		}
	}
	return result;
}

FiftyoneDegrees::Common::Value<double> ResultsBase::getValueAsDouble(const char* propertyName) {
	return getValueAsDouble(getRequiredPropertyIndex(propertyName));
}

FiftyoneDegrees::Common::Value<double> ResultsBase::getValueAsDouble(const string &propertyName) {
	return getValueAsDouble(propertyName.c_str());
}

FiftyoneDegrees::Common::Value<double> ResultsBase::getValueAsDouble(const string *propertyName) {
	return getValueAsDouble(propertyName->c_str());
}

FiftyoneDegrees::Common::Value<vector<string>> ResultsBase::getValues(
	int requiredPropertyIndex) {
	Value<vector<string>> result;
	if (hasValuesInternal(requiredPropertyIndex) == false) {
		fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
	}
	else {
		vector<string> values;
		getValuesInternal(requiredPropertyIndex, values);
		result.setValue(values);
	}
	return result;
}

FiftyoneDegrees::Common::Value<vector<string>> ResultsBase::getValues(
	const char *propertyName) {
	return getValues(getRequiredPropertyIndex(propertyName));
}

FiftyoneDegrees::Common::Value<vector<string>> ResultsBase::getValues(
	const string &propertyName) {
	return getValues(propertyName.c_str());
}

FiftyoneDegrees::Common::Value<vector<string>> ResultsBase::getValues(
	const string *propertyName) {
	return getValues(propertyName->c_str());
}

int ResultsBase::getRequiredPropertyIndex(
	const char *propertyName) {
	return PropertiesGetRequiredPropertyIndexFromName(
		available,
		propertyName);
}
