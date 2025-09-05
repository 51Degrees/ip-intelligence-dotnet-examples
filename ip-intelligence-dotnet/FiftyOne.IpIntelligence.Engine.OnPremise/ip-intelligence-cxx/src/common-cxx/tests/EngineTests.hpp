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

#ifndef FIFTYONE_DEGREES_ENGINE_BASE_TEST_HPP
#define FIFTYONE_DEGREES_ENGINE_BASE_TEST_HPP

#include "Base.hpp"
#include "../EngineBase.hpp"
#include <unordered_map>
#include <thread>

#define ENGINE_CLASS_NAME(e,t,c,p) Engine##e##Tests##t##c##p

#define ENGINE_CLASS_NAME_BASE(e,t) Engine##e##Tests##t

#define ENGINE_CLASS_NAME_CONFIG_POINTER(e,c) fiftyoneDegrees##e##c##ConfigPtr

#define ENGINE_CLASS_NAME_CONFIG_SOURCE(e,c) fiftyoneDegrees##e##c##Config

#define ENGINE_CONFIG(e,c) fiftyoneDegreesConfig##e \
*ENGINE_CLASS_NAME_CONFIG_POINTER(e,c) = &ENGINE_CLASS_NAME_CONFIG_SOURCE(e,c);

#define EXTERN_ENGINE_CONFIG(e,c) extern fiftyoneDegreesConfig##e \
*ENGINE_CLASS_NAME_CONFIG_POINTER(e,c);

#define PROPERTY_SAMPLE_SIZE 10
#define PROFILE_SAMPLE_SIZE 10
#define VALUE_SAMPLE_SIZE 10

using namespace FiftyoneDegrees::Common;

class EngineTests : public Base {
public:
	EngineTests(
		RequiredPropertiesConfig *requiredProperties, 
		const char *directory, 
		const char **fileNames,
		int fileNamesLength);
	virtual ~EngineTests();
	virtual void SetUp();
	virtual void TearDown();
	virtual void testProduct(string expected);
	virtual void testType(string expected);
	virtual void testPublishedDate();
	virtual void testUpdateDate();
	virtual void verify();
	void properties();
	virtual void validate(ResultsBase *results);
	virtual void validateQuick(ResultsBase *results);
protected:
	virtual EngineBase* getEngine();
	RequiredPropertiesConfig *requiredProperties;
	const char *directory;
	const char *fileName;
	const char** fileNames;
	int fileNamesLength;
	char fullName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	virtual void validateIndex(ResultsBase *results, int index);
	virtual void validateName(ResultsBase *results, string *name);
	void validateByBoth(ResultsBase *results);
	void validateByIndex(ResultsBase *results);
	virtual void validateByName(ResultsBase *results);
	void validateAll(ResultsBase *results);
	void verifyWithEvidence(EvidenceBase *evidence);
	bool isNameAvailable(ResultsBase *results, string *name);
	bool mustHaveValue(string *name);
	void verifyMetaData(EngineBase *engine);
	virtual void verifyComponentMetaDataDefaultProfile(
		MetaData *metaData,
		ComponentMetaData *component);
	virtual void verifyComponentMetaData(MetaData *metaData);
	virtual void verifyPropertyMetaData(MetaData *metaData);
	virtual void verifyProfileMetaData(MetaData *metaData);
	virtual void verifyValueMetaData(MetaData *metaData);
	void verifyMetaDataReload(EngineBase *engine);
};

#endif