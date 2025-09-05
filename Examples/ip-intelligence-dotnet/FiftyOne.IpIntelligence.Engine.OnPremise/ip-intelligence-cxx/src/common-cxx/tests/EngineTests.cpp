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

#include "EngineTests.hpp"

// If the library is thread safe then many threads can be run in parallel, if 
// not then one.
#define THREAD_COUNT (fiftyoneDegreesThreadingGetIsThreadSafe() ? 4 : 1)

using std::hash;
using std::thread;

EngineTests::EngineTests(
	RequiredPropertiesConfig *requiredProperties,
	const char *directory,
	const char **fileNames,
	int fileNamesLength) : Base() {
	this->requiredProperties = requiredProperties;
	this->directory = directory;
	this->fileNames = fileNames;
	this->fileNamesLength = fileNamesLength;
}

EngineTests::~EngineTests() {
	if (requiredProperties != nullptr) {
		delete requiredProperties;
	}
}

void EngineTests::SetUp() {
	Base::SetUp();
	fiftyoneDegreesMalloc = fiftyoneDegreesMemoryStandardMalloc;
	fiftyoneDegreesMallocAligned = fiftyoneDegreesMemoryStandardMallocAligned;
	fiftyoneDegreesFree = fiftyoneDegreesMemoryStandardFree;
	fiftyoneDegreesFreeAligned = fiftyoneDegreesMemoryStandardFreeAligned;

	fiftyoneDegreesStatusCode status = FIFTYONE_DEGREES_STATUS_NOT_SET;
	for (int i = 0;
		i < fileNamesLength && status != FIFTYONE_DEGREES_STATUS_SUCCESS;
		i++) {
		status = fiftyoneDegreesFileGetPath(
			directory,
			fileNames[i],
			fullName,
			sizeof(fullName));
		if (status == FIFTYONE_DEGREES_STATUS_SUCCESS) {
			fileName = fileNames[i];
		}
	}
	 
	ASSERT_EQ(status, FIFTYONE_DEGREES_STATUS_SUCCESS) << "File '" << 
		fileName << "' could not be found in directory '" << 
		directory << "'";
}

void EngineTests::TearDown() { Base::TearDown(); }

EngineBase* EngineTests::getEngine() { return nullptr; }

void EngineTests::properties() {
	Collection<string, PropertyMetaData> *properties =
		getEngine()->getMetaData()->getProperties();
	PropertyMetaData *property;
	uint32_t i;
	vector<string>::iterator mapIt;
	vector<string> maps;
	for (i = 0; i < properties->getSize(); i++) {
		property = properties->getByIndex(i);
		ASSERT_FALSE(property->getName().empty());
		ASSERT_FALSE(property->getType().empty());

		maps = property->getDataFilesWherePresent();
		for (mapIt = maps.begin(); mapIt < maps.end(); mapIt++) {
			ASSERT_FALSE((*mapIt).empty());
		}
		delete property;
	}
	delete properties;
}

void EngineTests::testProduct(string expected) {
	EXPECT_EQ(getEngine()->getProduct(), expected) << "Product mismatch";
}

void EngineTests::testType(string expected) {
	EXPECT_EQ(getEngine()->getType(), expected) << "Type mismatch";
}

void EngineTests::testPublishedDate() {
	Date test = getEngine()->getPublishedTime();
	EXPECT_NE(test.getYear(), 0) << "Published year mismatch";
	EXPECT_NE(test.getMonth(), 0) << "Published month mismatch";
	EXPECT_NE(test.getDay(), 0) << "Published day mismatch";
}

void EngineTests::testUpdateDate() {
	Date test = getEngine()->getUpdateAvailableTime();
	EXPECT_NE(test.getYear(), 0) << "Update year mismatch";
	EXPECT_NE(test.getMonth(), 0) << "Update month mismatch";
	EXPECT_NE(test.getDay(), 0) << "Update day mismatch";
}

void EngineTests::verify() { 
	// Do nothing as the inheriting class should implement 
}

bool EngineTests::isNameAvailable(ResultsBase *results, string *name) {
	vector<string> p = results->getProperties();
	return find(p.begin(), p.end(), *name) != p.end();
}

bool EngineTests::mustHaveValue(string *name) {
	Collection<string, PropertyMetaData> *properties =
		getEngine()->getMetaData()->getProperties();
	PropertyMetaData *property = properties->getByKey(*name);
	bool isMandatory = property != nullptr &&
		property->getIsMandatory();
	bool isJavaScript = property != nullptr &&
		property->getName() != "" &&
		property->getType().compare("javascript") == 0;
	bool hasDefault = property != nullptr &&
		property->getDefaultValue().empty() == false;
	delete property;
	delete properties;
	return (isMandatory || hasDefault) && (isJavaScript == false);
}

void EngineTests::validateName(ResultsBase *results, string *name) {
	if (isNameAvailable(results, name) == false) {

		// If the name is not available in the properties then when it's 
		// requested an exception should be thrown. 
		EXPECT_THROW(*results->getValues(name),
			InvalidPropertyException) << "Property '" << *name << "' is "
			"missing and should throw exception";
	}
	else {
		// If the name is available so the values should be retrieved with
		// out an exception.
		Value<vector<string>> values = results->getValues(name);

		if (values.hasValue() && values.getValue().size() > 0) {
			EXPECT_NO_THROW(*results->getValueAsString(name)) << "String value "
				"for property '" << *name << "' can't throw exception";
			if ((*values).size() == 1) {

				// One value is available. Check all the typed value methods return
				// a value and do not throw an error.
				EXPECT_NO_THROW(*results->getValueAsBool(name)) << "Bool value for "
					"property '" << *name << "' can't throw exception";
				EXPECT_NO_THROW(*results->getValueAsInteger(name)) << "Integer "
					"value for property '" << *name << "' can't throw exception";
				EXPECT_NO_THROW(*results->getValueAsDouble(name)) << "Double value "
					"for property '" << *name << "' can't throw exception";
			}
			else if ((*values).size() > 1) {
				// There are multiple values available so verify that the single 
				// value methods generate an exception.
				EXPECT_THROW(*results->getValueAsBool(name),
					TooManyValuesException) << "Bool value for property '"
					<< *name << "'should throw an exception";
				EXPECT_THROW(*results->getValueAsInteger(name),
					TooManyValuesException) << "Integer value for property '"
					<< *name << "'should throw an exception";
				EXPECT_THROW(*results->getValueAsDouble(name),
					TooManyValuesException) << "Double value for property '"
					<< *name << "'should throw an exception";
			}
		}
		else {

			// There are no values returned. This is only allowed in three
			// situations:
			// 1. If the property is of type JavaScript and there is no
			//    JavaScript value for the evidence available. For example;
			//    the property may relate  specifically to iPhone and the
			//    User-Agent(s) don't related to iPhone and there is no value
			//    for the property.
			// 2. If there was no evidence provided. This means the results can
			//    not be determined as there was nothing to process.
			// 3. If the property is not marked as mandatory, and does not have
			//    a default value.
			EXPECT_TRUE(mustHaveValue(name) == false ||
				values.getNoValueReason() ==
				FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS) <<
				L"Must get values for available property '" << *name << "'";
		}
	}
}

void EngineTests::validateIndex(ResultsBase *results, int index) {
	Value<vector<string>> values = results->getValues(index);
	if (values.hasValue()) {
		if ((*values).size() == 1) {
			EXPECT_NO_THROW(*results->getValueAsString(index)) << "String value "
				"for property '" << results->getPropertyName(index) << "' at "
				"index '" << index << "' can't throw exception";
			EXPECT_NO_THROW(*results->getValueAsBool(index)) << "Bool value for "
				"property '" << results->getPropertyName(index) << "' at "
				"index '" << index << "' can't throw exception";;
			EXPECT_NO_THROW(*results->getValueAsInteger(index)) << "Integer value "
				"for property '" << results->getPropertyName(index) << "' at "
				"index '" << index << "' can't throw exception";;
			EXPECT_NO_THROW(*results->getValueAsDouble(index)) << "Double value "
				"for property '" << results->getPropertyName(index) << "' at "
				"index '" << index << "' can't throw exception";;
		}
		else if ((*values).size() > 1) {
			EXPECT_NO_THROW(*results->getValueAsString(index)) << "String value "
				"for property '" << results->getPropertyName(index) << "' at "
				"index '" << index << "' can't throw exception";
			EXPECT_THROW(*results->getValueAsBool(index), TooManyValuesException);
			EXPECT_THROW(*results->getValueAsInteger(index), TooManyValuesException);
			EXPECT_THROW(*results->getValueAsDouble(index), TooManyValuesException);
		}
	}
}

void EngineTests::validateByIndex(ResultsBase *results) {
	for (int i = 0; i < results->getAvailableProperties(); i++) {
		validateIndex(results, i);
	}
}

void EngineTests::validateByBoth(ResultsBase *results) {
	for (int i = 0; i < results->getAvailableProperties(); i++) {
		string name = results->getPropertyName(i);
		validateName(results, &name);
	}
}

void EngineTests::validateByName(ResultsBase *results) {
	vector<string> properties = results->getProperties();
	for (vector<string>::const_iterator i = properties.begin();
		i != properties.end();
		i++) {
		validateName(results, (string*)&(*i));
	}
}

void EngineTests::validateAll(ResultsBase *results) {
	uint32_t i;
	PropertyMetaData *property;
	Collection<string, PropertyMetaData> *properties =
		getEngine()->getMetaData()->getProperties();
	for (i = 0; i < properties->getSize(); i++) {
		property = properties->getByIndex(i);
		string name = property->getName();
		validateName(results, &name);
		delete property;
	}
	delete properties;
}

void EngineTests::validate(ResultsBase *results) {
	validateByIndex(results);
	validateByName(results);
	validateByBoth(results);
	validateAll(results);
}

void EngineTests::validateQuick(ResultsBase *results) {
	for (int i = 0; i < results->getAvailableProperties(); i++) {
		vector<string> values;
		EXPECT_NO_THROW(values = *results->getValues(i)) << "Should not throw "
			"exception for property '" << results->getPropertyName(i) << "'";
	}
}

void EngineTests::verifyWithEvidence(EvidenceBase *evidence) {
	ResultsBase *results = getEngine()->processBase(evidence);
	validate(results);
	delete results;
}

void EngineTests::verifyComponentMetaDataDefaultProfile(
	MetaData *metaData, 
	ComponentMetaData *component) {
	ProfileMetaData *defaultProfile =
		metaData->getDefaultProfileForComponent(component);
	ComponentMetaData *otherComponent =
		metaData->getComponentForProfile(defaultProfile);
	ASSERT_EQ(*component, *otherComponent) <<
		L"The component and its default profile are not linked." <<
		L"\nComponent Id = " << (int)component->getComponentId() <<
		L"\nOther Component Id = " << (int)otherComponent->getComponentId() <<
		L"\nProfile Id = " << defaultProfile->getProfileId();

	delete otherComponent;
	delete defaultProfile;
}

void EngineTests::verifyComponentMetaData(MetaData *metaData) {
	Collection<byte, ComponentMetaData> *components =
		metaData->getComponents();
	ASSERT_NE(nullptr, components) << L"Components should not be null.";
	uint32_t componentIndex, propertyIndex;
	ComponentMetaData *component, *otherComponent;
	Collection<string, PropertyMetaData> *properties;
	PropertyMetaData *property;
	for (componentIndex = 0;
		componentIndex < components->getSize();
		componentIndex++) {
		component = components->getByIndex(componentIndex);
		otherComponent = components->getByKey(component->getKey());
		ASSERT_EQ(*component, *otherComponent) <<
			L"The same component should be returned for the same key.";
		ASSERT_NE(component, otherComponent) <<
			L"The component should be a unique instance.";
		delete otherComponent;
		verifyComponentMetaDataDefaultProfile(metaData, component);
		properties = metaData->getPropertiesForComponent(component);
		for (propertyIndex = 0;
			propertyIndex < properties->getSize();
			propertyIndex++) {
			property = properties->getByIndex(propertyIndex);
			otherComponent = metaData->getComponentForProperty(property);
			ASSERT_EQ(*component, *otherComponent) <<
				L"The component and its properties are not linked." <<
				L"\nComponent Id = " << (int)component->getComponentId() <<
				L"\nOther Component Id = " << (int)otherComponent->getComponentId() <<
				L"\nProperty = " << property->getName() << L".";
			delete property;
			delete otherComponent;
		}
		delete properties;
		delete component;
	}
	delete components;
}

void EngineTests::verifyPropertyMetaData(MetaData *metaData) {
	Collection<string, PropertyMetaData> *properties =
		metaData->getProperties();
	ASSERT_NE(nullptr, properties) << L"Properties should not be null.";
	PropertyMetaData *property, *otherProperty;
	ValueMetaData *value;
	ComponentMetaData *component;
	Collection<string, PropertyMetaData> *componentProperties;
	Collection<ValueMetaDataKey, ValueMetaData> *values;
	uint32_t propertyIndex, valueIndex, componentPropertyIndex;
	for (propertyIndex = 0;
		propertyIndex < properties->getSize();
		propertyIndex += 
			properties->getSize() > PROPERTY_SAMPLE_SIZE ?
			properties->getSize() / PROPERTY_SAMPLE_SIZE :
			1) {
		property = properties->getByIndex(propertyIndex);
		value = metaData->getDefaultValueForProperty(property);
		if (value != nullptr) {
			otherProperty = metaData->getPropertyForValue(value);
			ASSERT_EQ(*property, *otherProperty) <<
				L"The property and its default value are not linked.";
			delete otherProperty;
		}
		delete value;
		values = metaData->getValuesForProperty(property);
		for (valueIndex = 0;
			valueIndex < values->getSize();
			valueIndex++) {
			value = values->getByIndex(valueIndex);
			otherProperty = metaData->getPropertyForValue(value);
			ASSERT_EQ(*property, *otherProperty) <<
				L"The property and its values are not linked." <<
				L"\nProperty = " << property->getName() <<
				L"\nOther Property = " << otherProperty->getName();
			delete otherProperty;
			delete value;
		}
		delete values;
		bool found = false;
		component = metaData->getComponentForProperty(property);
		componentProperties = metaData->getPropertiesForComponent(component);
		for (componentPropertyIndex = 0;
			componentPropertyIndex < componentProperties->getSize();
			componentPropertyIndex++) {
			otherProperty = componentProperties->getByIndex(
				componentPropertyIndex);
			if (*property == *otherProperty) {
				found = true;
				delete otherProperty;
				break;
			}
			delete otherProperty;
		}
		delete componentProperties;
		ASSERT_TRUE(found) << L"The property was not added to its component." <<
			L"\nProperty = " << property->getName() <<
			L"\nCopmonent Id = " << component->getComponentId();
		delete component;
		delete property;
	}
	delete properties;
}

void EngineTests::verifyProfileMetaData(MetaData *metaData) {
	Collection<uint32_t, ProfileMetaData> *profiles =
		metaData->getProfiles();
	ASSERT_NE(nullptr, profiles) << L"Profiles should not be null.";
	ProfileMetaData *profile;
	ComponentMetaData *component, *otherComponent;
	ValueMetaData *value;
	PropertyMetaData *property;
	Collection<ValueMetaDataKey, ValueMetaData> *values;
	uint32_t profileIndex, valueIndex;
	for (profileIndex = 0;
		profileIndex < profiles->getSize();
		profileIndex +=
			profiles->getSize() > PROFILE_SAMPLE_SIZE ?
			profiles->getSize() / PROFILE_SAMPLE_SIZE :
			1) {
		profile = profiles->getByIndex(profileIndex);
		component = metaData->getComponentForProfile(profile);
		ASSERT_NE(nullptr, component) <<
			L"The component was not set in the profile";
		values = metaData->getValuesForProfile(profile);
		for (valueIndex = 0;
			valueIndex < values->getSize();
			valueIndex++) {
			value = values->getByIndex(valueIndex);
			property = metaData->getPropertyForValue(value);
			otherComponent = metaData->getComponentForProperty(property);
			ASSERT_EQ(*component, *otherComponent) <<
				L"The profile should not contain a value for a different component." <<
				L"\nComponent Id = " << (int)component->getComponentId() <<
				L"\nOther Component Id = " << (int)otherComponent->getComponentId() <<
				L"\nValue = " << value->getName();
			delete otherComponent;
			delete property;
			delete value;
		}
		delete values;
		delete component;
		delete profile;
	}
	delete profiles;
}

void EngineTests::verifyValueMetaData(MetaData *metaData) {
	Collection<ValueMetaDataKey, ValueMetaData> *values =
		metaData->getValues();
	ASSERT_NE(nullptr, values) << L"Values should not be null.";
	ValueMetaData *value, *otherValue;
	PropertyMetaData *property;
	Collection<ValueMetaDataKey, ValueMetaData> *otherValues;
	uint32_t valueIndex, propertyValueIndex;
	for (valueIndex = 0;
		valueIndex < values->getSize();
		valueIndex += 
			values->getSize() > VALUE_SAMPLE_SIZE ?
			values->getSize() / VALUE_SAMPLE_SIZE :
			1) {
		value = values->getByIndex(valueIndex);
		bool found = false;
		property = metaData->getPropertyForValue(value);
		otherValues = metaData->getValuesForProperty(property);
		for (propertyValueIndex = 0;
			propertyValueIndex < otherValues->getSize();
			propertyValueIndex++) {
			otherValue = otherValues->getByIndex(propertyValueIndex);
			if (*value == *otherValue) {
				found = true;
				delete otherValue;
				break;
			}
			delete otherValue;
		}
		ASSERT_TRUE(found) << L"The value was not added to its property." <<
			L"\nValue = " << value->getName() <<
			L"\nProperty = " << property->getName();
		delete otherValues;
		delete property;
		delete value; 

	}
	delete values;
}

void EngineTests::verifyMetaData(EngineBase *engine) {
	MetaData *metaData = engine->getMetaData();
	ASSERT_NE(nullptr, metaData) << L"Meta data should not be null.";
	// Components
	verifyComponentMetaData(metaData);
	// Properties
	verifyPropertyMetaData(metaData);
	// Profiles
	verifyProfileMetaData(metaData);
	// Values
	verifyValueMetaData(metaData);
}

class MetaDataReloadState {
public:
	MetaDataReloadState() {
		engine = nullptr;
	}
	MetaDataReloadState(EngineBase *engine, int tests) {
		hashValues = new uint32_t[tests];
		for (int i = 0; i < tests; i++) {
			hashValues[i] = 0;
		}
		this->engine = engine;
		this->tests = tests;
		complete = false;
	}
	~MetaDataReloadState() {
		delete[] hashValues;
	}
	uint32_t* hashValues;
	EngineBase *engine;
	int tests;
	volatile bool complete;
};

void hashMetaData(MetaDataReloadState *state, int index) {
	hash<string> hasher;
	uint32_t i;

	Collection<byte, ComponentMetaData> *components =
		state->engine->getMetaData()->getComponents();
	ComponentMetaData *component;
	for (i = 0; i < components->getSize(); i++) {
		component = components->getByIndex(i);
		state->hashValues[index] ^= hasher(component->getName());
		delete component;
	}
	delete components;

	PropertyMetaData *property;
	Collection<string, PropertyMetaData> *properties =
		state->engine->getMetaData()->getProperties();
	for (i = 0; i < properties->getSize(); i += 10) {
		property = properties->getByIndex(i);
		state->hashValues[index] ^= hasher(property->getName());
		state->hashValues[index] ^= hasher(property->getCategory());
		state->hashValues[index] ^= hasher(property->getDescription());
		state->hashValues[index] ^= hasher(property->getType());
		state->hashValues[index] ^= hasher(property->getUrl());
		delete property;
	}
	delete properties;

	try {
		ValueMetaData *value;
		Collection<ValueMetaDataKey, ValueMetaData> *values =
			state->engine->getMetaData()->getValues();
		for (i = 0; i < values->getSize(); i += 100) {
			value = values->getByIndex(i);
			state->hashValues[index] ^= hasher(value->getName());
			state->hashValues[index] ^= hasher(value->getDescription());
			state->hashValues[index] ^= hasher(value->getUrl());
			delete value;
		}
		delete values;
	}
	catch (NotImplementedException&) {
		// Ignore the fact this metadata type is not implemented.
	}

	try {
	    ProfileMetaData *profile;
	    Collection<uint32_t, ProfileMetaData> *profiles =
	            state->engine->getMetaData()->getProfiles();
	    for (i = 0; i < profiles->getSize(); i += 100) {
	        profile = profiles->getByIndex(i);
            state->hashValues[index] ^= profile->getProfileId();
            delete profile;
	    }
	    delete profiles;
	}
	catch (NotImplementedException&) {
	    // Ignore the fact this metadata type is not implemented.
	}
}

void reloadRepeat(MetaDataReloadState *state) {
	do {
		state->engine->refreshData();
#ifdef _MSC_VER
		Sleep(500); // milliseconds
#else
		usleep(500000); // microseconds
#endif
	} while (state->complete == false);
}

void EngineTests::verifyMetaDataReload(EngineBase *engine) {
	MetaDataReloadState state(
		engine, 
		fiftyoneDegreesThreadingGetIsThreadSafe() ?
			THREAD_COUNT : 
			2);

	// Multi threaded reload test.
	if (fiftyoneDegreesThreadingGetIsThreadSafe()) {
		thread* threads = new thread[THREAD_COUNT];
		thread reloader(reloadRepeat, &state);
		for (int i = 0; i < THREAD_COUNT; i++) {
			threads[i] = thread(hashMetaData, &state, i);
		}
		for (int i = 0; i < THREAD_COUNT; i++) {
			threads[i].join();
		}
		state.complete = true;
		reloader.join();
		delete[] threads;
	}
	
	// Single threaded reload test.
	else {
		hashMetaData(&state, 0);
		state.complete = true;
		reloadRepeat(&state);
		hashMetaData(&state, 1);
	}

	// Verify the hash values.
	for (int i = 0; i < state.tests - 1; i++) {
		ASSERT_EQ(state.hashValues[i], state.hashValues[i + 1]) <<
			L"Hash values were not the same.";
	}
}
