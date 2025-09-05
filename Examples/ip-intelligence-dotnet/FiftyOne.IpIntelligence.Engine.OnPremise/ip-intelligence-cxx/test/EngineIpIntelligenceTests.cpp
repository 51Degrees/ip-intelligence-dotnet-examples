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

#ifdef _MSC_FULL_VER
#include <string.h>
#else
#include <strings.h>
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#endif
#include <regex>

#include "Constants.hpp"
#include "EngineIpIntelligenceTests.hpp"
#include "../src/EngineIpi.hpp"
#include "../src/common-cxx/file.h"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

EngineIpIntelligenceTests::EngineIpIntelligenceTests(
	ConfigIpi *config,
	RequiredPropertiesConfig *requiredProperties,
	const char *directory,
	const char **fileNames,
	int fileNamesLength,
	const char *ipAddressesFileName)
	: EngineTests(requiredProperties, directory, fileNames, fileNamesLength) {
	char ipAddressesFullName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char ipAddress[500] = "";
	fiftyoneDegreesFileGetPath(
		directory,
		ipAddressesFileName,
		ipAddressesFullName,
		sizeof(ipAddressesFullName));
	fiftyoneDegreesTextFileIterate(
		ipAddressesFullName, 
		ipAddress, 
		sizeof(ipAddress), 
		this, 
		EngineIpIntelligenceTests::ipAddressRead);
	this->config = config;
	engine = nullptr;
}

EngineIpIntelligenceTests::~EngineIpIntelligenceTests() {
	while (ipAddresses.empty() == false) {
		ipAddresses.pop_back();
	}
}

EngineBase* EngineIpIntelligenceTests::getEngine() { return (EngineBase*)engine; }

void EngineIpIntelligenceTests::SetUp() {
	EngineTests::SetUp();
}

void EngineIpIntelligenceTests::TearDown() {
	if (engine != nullptr) {
		delete engine;
	}
	if (data.current != nullptr) {
		fiftyoneDegreesFree(data.current);
		data.current = nullptr;
		data.length = 0;
	}
	EngineTests::TearDown();
}

void EngineIpIntelligenceTests::verifyComponentMetaDataDefaultProfile(
	MetaData *metaData,
	ComponentMetaData *component) {
	if (component->getDefaultProfileId() != 0) {
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
}

/*
 * This test check whether IP address string can be parsed
 * and searched for in values collection where the IP address
 * is in byte format.
 */
void EngineIpIntelligenceTests::verifyValueMetaDataIpAddress() {
	auto const engineIpi = (EngineIpi *)getEngine();
	auto const values = std::unique_ptr<Collection<ValueMetaDataKey, ValueMetaData>>(
		engineIpi->getMetaData()->getValues());
	EXPECT_TRUE(values->getSize() > 0) << "There is no values "
		"meta data.";

	auto const properties = std::unique_ptr<Collection<string, PropertyMetaData>>(
		engineIpi->getMetaData()->getProperties());
	EXPECT_TRUE(properties->getSize() > 0) << "There is no properties "
		"meta data.";

	auto const property = std::unique_ptr<PropertyMetaData>(
		properties->getByKey("IpRangeStart"));
	if (!property) {
		return;
	}
	auto const rangeStartValues = std::unique_ptr<Collection<ValueMetaDataKey, ValueMetaData>>(
		engineIpi->getMetaData()->getValuesForProperty(property.get()));
	EXPECT_TRUE(rangeStartValues->getSize() > 0) << "There is no values "
		"meta data for property " << property->getName();


	// Pick the first value to be used for testing.
	auto const value1 = std::unique_ptr<ValueMetaData>(
		rangeStartValues->getByIndex(1));
	EXPECT_TRUE(value1 != nullptr) << "There is no value at index 0.";
	
	// See if a valid IP address value string can be correctly parsed
	// and searched from the values collection.
	auto const valueName = value1->getName();
	auto const key = ValueMetaDataKey("IpRangeStart", valueName);
	auto const value2 = std::unique_ptr<ValueMetaData>(
		values->getByKey(key));
	EXPECT_TRUE(value2 != nullptr) << "Value meta data is not found "
		"where it should be at IP address: " << value1->getName();
	if (value2) {
		EXPECT_EQ(0, value1->getName().compare(value2->getName()))
			<< "Value meta data is not correct where it should be at IP address: "
			<< value1->getName();
	}
}

void EngineIpIntelligenceTests::metaData() {
	// FIXME: Too long (15+ min)
	// EngineTests::verifyMetaData(getEngine());
	// // Verify additional cases where the value
	// // is not string
	// verifyValueMetaDataIpAddress();
}
void EngineIpIntelligenceTests::availableProperties() {
	// TODO: This mainly check the evidence property which is 
	// not yet applicable to IP intelligence
}

string EngineIpIntelligenceTests::getExpectedFileType() {
	int i;
	for (i = 0; i < _IpiFileNamesLength; i++) {
		if (strcmp(fileName, _IpiFileNames[i]) == 0) {
			return _fileTypes[i];
		}
	}
	return "";
}

void EngineIpIntelligenceTests::ipAddressRead(
	const char *ipAddress,
	void *state) {
	((EngineIpIntelligenceTests*)state)->ipAddresses.push_back(
		string(ipAddress));
}

void EngineIpIntelligenceTests::verifyPropertyValue(
	ResultsBase *results, 
	string property, 
	string value) {
	vector<string> props = results->getProperties();
	if (find(props.begin(), props.end(), property) != props.end()) {
		EXPECT_EQ(*results->getValueAsString(property), value);
	}
}

void EngineIpIntelligenceTests::validateIndex(
	ResultsBase *results,
	int index) {
	ResultsIpi *resultsIpi = (ResultsIpi *)results;
	Common::Value<vector<string>> values = results->getValues(index);
	if (values.hasValue()) {
		EXPECT_NO_THROW(*resultsIpi->getValueAsIpAddress(index)) << "IP address value "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedBoolList(index)) << "Boolean list "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedIntegerList(index)) << "Integer list "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedDoubleList(index)) << "Double list "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedStringList(index)) << "String list "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValueAsString(index)) << "String value "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_THROW(*resultsIpi->getValueAsBool(index), TooManyValuesException);
		EXPECT_THROW(*resultsIpi->getValueAsInteger(index), TooManyValuesException);
		EXPECT_THROW(*resultsIpi->getValueAsDouble(index), TooManyValuesException);
	}
}

void EngineIpIntelligenceTests::validateName(
	ResultsBase *results,
	string *name) {

	if (isNameAvailable(results, name) == false) {
		// If the name is not available in the properties then when it's 
		// requested an exception should be thrown. 
		EXPECT_THROW(*results->getValues(name),
			InvalidPropertyException) << "Property '" << *name << "' is "
			"missing and should throw exception";
	}
	else {
		ResultsIpi *resultsIpi = (ResultsIpi *)results;
		// If the name is available so the values should be retrieved with
		// out an exception.
		Common::Value<vector<string>> values = results->getValues(name);

		if (values.hasValue() && values.getValue().size() > 0) {
			EXPECT_NO_THROW(*resultsIpi->getValueAsIpAddress(name)) << "IP address value "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedBoolList(name)) << "Boolean list "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedIntegerList(name)) << "Integer list "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedDoubleList(name)) << "Double list "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedStringList(name)) << "String list "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValueAsString(name)) << "String value "
				"for property '" << *name << "' can't throw exception";
			EXPECT_THROW(*resultsIpi->ResultsBase::getValueAsBool(name), TooManyValuesException);
			EXPECT_THROW(*resultsIpi->ResultsBase::getValueAsInteger(name), TooManyValuesException);
			EXPECT_THROW(*resultsIpi->ResultsBase::getValueAsDouble(name), TooManyValuesException);
		}
		else {
			// There are no values returned. This is only allowed when:
			// 1. If there was no evidence provided. This means the results can
			//    not be determined as there was nothing to process.
			// 2. We don't have enough data for required property.
			EXPECT_TRUE(values.getNoValueReason() ==
				FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS ||
				values.getNoValueReason() == 
				FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE) <<
				L"Must get values for available property '" << *name << "'";
		}
	}
}

void EngineIpIntelligenceTests::validateQuick(ResultsBase *results) {
	ResultsIpi* resultsIpi = (ResultsIpi*)results;
	for (int i = 0; i < results->getAvailableProperties(); i++) {
		vector<string> values;
		Common::Value<vector<string>> value = results->getValues(i);
		if (value.hasValue()) {
			EXPECT_NO_THROW(values = *value) << "Should not throw "
			"exception for property '" << results->getPropertyName(i) << "'";
			vector<WeightedValue<string>> weightedStrings = 
				resultsIpi->getValuesAsWeightedStringList(i).getValue();

			float curWeight = 0.0f;
			for (vector<WeightedValue<string>>::iterator iter = weightedStrings.begin();
				iter != weightedStrings.end();
				iter++) {
				if (iter != weightedStrings.begin()) {
					EXPECT_TRUE(curWeight >= iter->getWeight()) << "Weights of returned results "
						"are not in the descending order: " << results->getPropertyName(i);
				}
				curWeight = iter->getWeight();
			}
		}
		else {
			// This could only happen only because we don't have the details
			// in our data.
			EXPECT_THROW(values = *value, NoValuesAvailableException);
		}
	}
}

void EngineIpIntelligenceTests::verifyWithEvidence(EvidenceIpi *evidence) {
	ResultsIpi *results = ((EngineIpi*)getEngine())->process(evidence);
	validate(results);
	delete results;
}

void EngineIpIntelligenceTests::verifyWithEmptyEvidence() {
	EvidenceIpi evidence;
	verifyWithEvidence(&evidence);
}

void EngineIpIntelligenceTests::verifyMixedPrefixesEvidence() {
	EvidenceIpi mixedEvidence;
	// To make sure query.client-ip-51d is pick up first
	mixedEvidence["query.client-ip-51d"] = lowerBoundIpv4Address;
	mixedEvidence["server.true-client-ip-51d"] = upperBoundIpv4Address;
	auto results = std::unique_ptr<ResultsIpi>(
		((EngineIpi*)getEngine())->process(&mixedEvidence));
	if (!results) {
		return;
	}
	Common::Value<IpIntelligence::IpAddress> rangeStart = results->getValueAsIpAddress("IpRangeStart");
	if (!rangeStart.hasValue()) {
		return;
	}
	unsigned char lowerBoundIpAddress[FIFTYONE_DEGREES_IPV4_LENGTH];
	memset(lowerBoundIpAddress, 0, FIFTYONE_DEGREES_IPV4_LENGTH);
	EXPECT_EQ(0,
		memcmp(rangeStart.getValue().getIpAddress(),
			lowerBoundIpAddress,
			FIFTYONE_DEGREES_IPV4_LENGTH)) << "The IpRangeStart IP address is not "
		"at the lower bound where it should be.";

	// Check if true-client-ip-51d is taking priority
	mixedEvidence["query.client-ip-51d"] = "";
	mixedEvidence["query.true-client-ip-51d"] = lowerBoundIpv4Address;
	results = std::unique_ptr<ResultsIpi>(
		((EngineIpi*)getEngine())->process(&mixedEvidence));
	rangeStart = results->getValueAsIpAddress("IpRangeStart");
	EXPECT_EQ(0,
		memcmp(rangeStart.getValue().getIpAddress(),
			lowerBoundIpAddress,
			FIFTYONE_DEGREES_IPV4_LENGTH)) << "The IpRangeStart IP address is not "
		"at the lower bound where it should be.";
}

void EngineIpIntelligenceTests::verifyWithEvidence() {
	EvidenceIpi queryEvidence, serverEvidence;
	queryEvidence["query.client-ip-51d"] = ipv4Address;
	verifyWithEvidence(&queryEvidence);

	serverEvidence["server.client-ip-51d"] = ipv6Address;
	verifyWithEvidence(&serverEvidence);
}

void EngineIpIntelligenceTests::verifyWithIpAddressString(const char *ipAddress) {
	EngineIpi *engineIpi = (EngineIpi*)getEngine();
	ResultsIpi *results = engineIpi->process(
		ipAddress);
	validate(results);
	delete results;
}

void EngineIpIntelligenceTests::verifyWithIpv4Address() {
	verifyWithIpAddressString(ipv4Address);
}

void EngineIpIntelligenceTests::verifyWithIpv6Address() {
	verifyWithIpAddressString(ipv6Address);
}

void EngineIpIntelligenceTests::verifyWithBadIpv4Address() {
	verifyWithIpAddressString(badIpv4Address);
}

void EngineIpIntelligenceTests::verifyWithBadIpv6Address() {
	verifyWithIpAddressString(badIpv6Address);
}

void EngineIpIntelligenceTests::verifyWithBoundIpv4Address() {
	verifyWithIpAddressString(lowerBoundIpv4Address);
	verifyWithIpAddressString(upperBoundIpv4Address);
}

void EngineIpIntelligenceTests::verifyWithBoundIpv6Address() {
	verifyWithIpAddressString(lowerBoundIpv6Address);
	verifyWithIpAddressString(upperBoundIpv6Address);
}

void EngineIpIntelligenceTests::verifyWithOutOfRangeIpv4Address() {
	verifyWithIpAddressString(outOfRangeIpv4Address);
}

void EngineIpIntelligenceTests::verifyWithOutOfRangeIpv6Address() {
	verifyWithIpAddressString(outOfRangeIpv6Address);
}


void EngineIpIntelligenceTests::verifyWithInvalidInput() {
	int character;
	char ipAddress[2];
	ipAddress[1] = '\0';
	EngineIpi *engineIpi = (EngineIpi*)getEngine();
	for (character = CHAR_MIN; character <= CHAR_MAX; character++) {
		ipAddress[0] = (char)character;
		ResultsIpi *results = engineIpi->process(
		ipAddress);
		validate(results);
		delete results;
	}
}

void EngineIpIntelligenceTests::verifyWithNullEvidence() {
	EngineTests::verifyWithEvidence(nullptr);
}
 
void EngineIpIntelligenceTests::verifyWithNullIpAddress() {
	EngineIpi*engineIpi = (EngineIpi*)getEngine();
	ResultsIpi *results = engineIpi->process(
		(const char *)nullptr);
	validate(results);
	delete results;
}
 
void EngineIpIntelligenceTests::verifyWithEmptyIpAddress() {
	EngineIpi *engineIpi = (EngineIpi*)getEngine();
	ResultsIpi *results = engineIpi->process(
		"");
	validate(results);
	delete results;
}


void EngineIpIntelligenceTests::verify() {
	EngineTests::verify();
	verifyWithEvidence();
	verifyWithIpv4Address();
	verifyWithIpv6Address();
	verifyWithBadIpv4Address();
	verifyWithBadIpv6Address();
	verifyWithBoundIpv4Address();
	verifyWithBoundIpv6Address();
	verifyWithOutOfRangeIpv4Address();
	verifyWithOutOfRangeIpv6Address();
	verifyWithEmptyEvidence();
	verifyWithEmptyIpAddress();
	verifyWithNullIpAddress();
	verifyWithNullEvidence();
	verifyWithInvalidInput();
}

bool EngineIpIntelligenceTests::validateIpAddressInternal(
	IpIntelligence::IpAddress ipAddress, int length) {
	const unsigned char *ip = ipAddress.getIpAddress();
	int octet;
	for (int i = 0; i < length; i++) {
		octet = (int)ip[i];
		if (octet < 0 || octet > 255) {
			return false;
		}
	}
	return true;
}

bool EngineIpIntelligenceTests::validateIpAddress(
	IpIntelligence::IpAddress ipAddress) {
	bool result = false;

	switch(ipAddress.getType()) {
	case FIFTYONE_DEGREES_IP_TYPE_IPV4:
		result = validateIpAddressInternal(ipAddress, FIFTYONE_DEGREES_IPV4_LENGTH);
		break;
	case FIFTYONE_DEGREES_IP_TYPE_IPV6:
		result = validateIpAddressInternal(ipAddress, FIFTYONE_DEGREES_IPV6_LENGTH);
		break;
	default:
		break;
	}
	return result;
}

void EngineIpIntelligenceTests::verifyIpAddressValue(
	const char *ipAddress, 
	Common::Value<IpIntelligence::IpAddress> value) {
	if (value.hasValue()) {
		EXPECT_EQ(true, validateIpAddress(value.getValue())) << "An invalid IP address has been "
			"returned, where it should be for IP address: " << ipAddress;
	}
}

void EngineIpIntelligenceTests::ipAddressPresent(const char *ipAddress) {
	EngineIpi *engineIpi = (EngineIpi*)getEngine();
	ResultsIpi *results = engineIpi->process(ipAddress);
	Common::Value<IpIntelligence::IpAddress> rangeStart = results->getValueAsIpAddress("IpRangeStart");
	Common::Value<IpIntelligence::IpAddress> rangeEnd = results->getValueAsIpAddress("IpRangeEnd");

	verifyIpAddressValue(ipAddress, rangeStart);
	verifyIpAddressValue(ipAddress, rangeEnd);

	delete results;
}

void EngineIpIntelligenceTests::boundIpAddressPresent(const char *ipAddress) {
	unsigned char lowerBoundIpAddress[FIFTYONE_DEGREES_IPV6_LENGTH];
	unsigned char upperBoundIpAddress[FIFTYONE_DEGREES_IPV6_LENGTH];
	memset(lowerBoundIpAddress, 0, FIFTYONE_DEGREES_IPV6_LENGTH);
	memset(upperBoundIpAddress, 0xff, FIFTYONE_DEGREES_IPV6_LENGTH);

	auto const engineIpi = (EngineIpi*)getEngine();
	auto const results = std::unique_ptr<ResultsIpi>(
		engineIpi->process(ipAddress));
	Common::Value<IpIntelligence::IpAddress> rangeStart = results->getValueAsIpAddress("IpRangeStart");
	Common::Value<IpIntelligence::IpAddress> rangeEnd = results->getValueAsIpAddress("IpRangeEnd");

	if (!rangeStart.hasValue()) {
		return;
	}

	verifyIpAddressValue(ipAddress, rangeStart);
	verifyIpAddressValue(ipAddress, rangeEnd);

	EXPECT_EQ(rangeStart.getValue().getType(), rangeEnd.getValue().getType())
			<< "IpRangeStart and IpRangeEnd types are not the same, where it should "
			"be at IP address: " << ipAddress;

	if (rangeStart.getValue().getType() == FIFTYONE_DEGREES_IP_TYPE_IPV4) {
		EXPECT_TRUE(
			memcmp(
				lowerBoundIpAddress,
				rangeStart.getValue().getIpAddress(),
				FIFTYONE_DEGREES_IPV4_LENGTH) == 0 ||
			memcmp(
				upperBoundIpAddress,
				rangeEnd.getValue().getIpAddress(),
				FIFTYONE_DEGREES_IPV4_LENGTH) == 0) << "IpRangeStart or IpRangeEnd are not "
			"at the bound where it should be at IP address: " << ipAddress;
	}
	else {
		EXPECT_TRUE(
			memcmp(
				lowerBoundIpAddress,
				rangeStart.getValue().getIpAddress(),
				FIFTYONE_DEGREES_IPV6_LENGTH) == 0 ||
			memcmp(
				upperBoundIpAddress,
				rangeEnd.getValue().getIpAddress(),
				FIFTYONE_DEGREES_IPV6_LENGTH) == 0) << "IpRangeStart or IpRangeEnd are not "
			"at the bound where it should be at IP address: " << ipAddress;
	}
}

void EngineIpIntelligenceTests::randomIpAddressPresent(int count) {
	auto const engineIpi = (EngineIpi*)getEngine();

	for (int i = 0; i < count; i++) {
		string ipAddress = ipAddresses[rand() % ipAddresses.size()];
		auto const results = std::unique_ptr<ResultsIpi>(
			engineIpi->process(ipAddress.c_str()));

		Common::Value<IpIntelligence::IpAddress> rangeStart = results->getValueAsIpAddress("IpRangeStart");
		Common::Value<IpIntelligence::IpAddress> rangeEnd = results->getValueAsIpAddress("IpRangeEnd");

		if (!rangeStart.hasValue()) {
			continue;
		}

		verifyIpAddressValue(ipAddress.c_str(), rangeStart);
		verifyIpAddressValue(ipAddress.c_str(), rangeEnd);

		EXPECT_EQ(rangeStart.getValue().getType(), rangeEnd.getValue().getType())
			<< "IpRangeStart and IpRangeEnd types are not the same, where it should "
			"be at IP address: " << ipAddress;
		EXPECT_TRUE(fiftyoneDegreesIpAddressesCompare(
			rangeStart.getValue().getIpAddress(),
			rangeEnd.getValue().getIpAddress(),
			rangeStart.getValue().getType()) < 0) << "Range start IP address should "
			"be smaller than Range end IP address, where it shoud for IP address: " <<
			ipAddress;
	}
}

void EngineIpIntelligenceTests::randomWithIpAddress(int count) {
#	ifdef _MSC_VER
	UNREFERENCED_PARAMETER(count);
#	endif
	// FIXME: Unstable on CI
	// EngineIpi *engineIpi = (EngineIpi*)getEngine();
	// for (int i = 0; i < count; i++) {
	// 	string ipAddress = ipAddresses[rand() % ipAddresses.size()];
	// 	ResultsIpi *results = engineIpi->process(
	// 		ipAddress.c_str());
	// 	validateQuick(results);
	// 	delete results;
	// }
}

void EngineIpIntelligenceTests::randomWithEvidence(int count) {
#	ifdef _MSC_VER
	UNREFERENCED_PARAMETER(count);
#	endif
	// FIXME: Unstable on CI
	// string ipKey = "query.client-ip-51d";
	// EngineIpi *engineIpi = (EngineIpi*)getEngine();
	// for (int i = 0; i < count; i++) {
	// 	EvidenceIpi evidence;
	// 	evidence[ipKey] = ipAddresses[rand() % ipAddresses.size()].c_str();
	// 	ResultsIpi *results = engineIpi->process(&evidence);
	// 	validateQuick(results);
	// 	delete results;
	// }
}

void EngineIpIntelligenceTests::multiThreadRandomRunThread(void* state) {
	((EngineIpIntelligenceTests*)state)->randomWithIpAddress(200);
	((EngineIpIntelligenceTests*)state)->randomWithEvidence(200);
	FIFTYONE_DEGREES_THREAD_EXIT;
}

/**
 * Check that multiple threads can fetch items from the cache safely.
 * NOTE: it is important that 'number of threads' <=
 * 'number of values' / 'number of threads'. This prevents null from being
 * returned by the cache.
 * @param concurrency number of threads to run the test with
 */
void EngineIpIntelligenceTests::multiThreadRandom(uint16_t concurrency) {
	if (fiftyoneDegreesThreadingGetIsThreadSafe() == false) {
		return;
	}
	ASSERT_NE(nullptr, getEngine());
	runThreads(
		concurrency,
		(FIFTYONE_DEGREES_THREAD_ROUTINE)multiThreadRandomRunThread);
}

 void EngineIpIntelligenceTests::compareResults(
 	ResultsIpi *a, 
 	ResultsIpi *b) {
 	EXPECT_NE(a->results->b.dataSet, b->results->b.dataSet) <<
 		"The data set was not reloaded.";
 	EXPECT_EQ(a->getAvailableProperties(), b->getAvailableProperties()) <<
 		"Number of properties available does not match.";
 	for (size_t i = 0; i < a->getProperties().size(); i++) {
		Common::Value<vector<string>> av = a->getValues((int)i);
		Common::Value<vector<string>> bv = b->getValues((int)i);
		if (av.hasValue()) {
			EXPECT_TRUE(bv.hasValue()) << "Expected both has values.";
			vector<string> avs = *a->getValues((int)i);
 			vector<string> bvs = *b->getValues((int)i);
 			EXPECT_EQ(avs.size(), bvs.size()) << "Expected same number of values.";
 			for (size_t v = 0; v < avs.size(); v++) {
 				EXPECT_STREQ(avs[v].c_str(), bvs[v].c_str()) <<
 					"Values for the new data set should be the same.";
 			}
		}
		else {
			// One does not have values, the other should not have value either.
			EXPECT_FALSE(bv.hasValue()) << "Expected both dot not have values.";
		}
 	}
 }
 
bool EngineIpIntelligenceTests::fileReadToByteArray() {
	fiftyoneDegreesStatusCode status = fiftyoneDegreesFileReadToByteArray(
		fullName,
		&data);
	return status == FIFTYONE_DEGREES_STATUS_SUCCESS;
}
 
void EngineIpIntelligenceTests::reloadFile() {
	EngineIpi *engineIpi = (EngineIpi*)getEngine();
	ResultsIpi *results1 = engineIpi->process(
		ipv4Address);
	engineIpi->refreshData();
	ResultsIpi *results2 = engineIpi->process(
		ipv4Address);
	compareResults(results1, results2);
	delete results1;
	delete results2;
}
 
void EngineIpIntelligenceTests::reloadMemory() {
	EngineIpi *engineIpi = (EngineIpi*)getEngine();
	ResultsIpi *results1 = engineIpi->process(
		ipv4Address);
	fiftyoneDegreesMemoryReader newData;
	fiftyoneDegreesStatusCode status = fiftyoneDegreesFileReadToByteArray(
		fullName,
		&newData);
	EXPECT_EQ(status, FIFTYONE_DEGREES_STATUS_SUCCESS) << "New data could "
		"not be loaded into memory from '" << fullName << "'";
	EXPECT_NE(newData.current, nullptr) << "New data could "
		"not be loaded into memory from '" << fullName << "'";
	engineIpi->refreshData(newData.current, newData.length);
	ResultsIpi *results2 = engineIpi->process(
		ipv4Address);
	compareResults(results1, results2);
	delete results1;
	delete results2;

	// Now that the results1 has been deleted free the memory used by the 
	// now replaced original dataset. Set the data that will be freed 
	// during tear down to that which is now active in memory.
	fiftyoneDegreesFree(data.current);
	data = newData;
}

ENGINE_TEST_CONFIGS(Ipi)
