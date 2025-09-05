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

#include "ConfigBase.hpp"

#include "fiftyone.h"

using namespace FiftyoneDegrees::Common;

ConfigBase::ConfigBase(fiftyoneDegreesConfigBase *config) {
	this->config = config;
}

ConfigBase::~ConfigBase() {
	freeTempDirectories();
}

void ConfigBase::setUseUpperPrefixHeaders(bool use) {
	this->config->usesUpperPrefixedHeaders = use;
}

void ConfigBase::setUseTempFile(bool use) {
	this->config->useTempFile = use;
}

void ConfigBase::setReuseTempFile(bool reuse) {
	this->config->reuseTempFile = reuse;
}

void ConfigBase::setTempDirectories(
	vector<string> newTempDirs) {
	this->tempDirs = newTempDirs;
	mapTempDirectories();
}

void ConfigBase::setPropertyValueIndex(bool index) {
	this->config->propertyValueIndex = index;
}

bool ConfigBase::getUseUpperPrefixHeaders() const {
	return config->usesUpperPrefixedHeaders;
}

bool ConfigBase::getUseTempFile() const {
	return config->useTempFile;
}

bool ConfigBase::getReuseTempFile() const {
	return config->reuseTempFile;
}

vector<string> ConfigBase::getTempDirectories() const {
	return tempDirs;
}

bool ConfigBase::getPropertyValueIndex() const {
	return config->propertyValueIndex;
}

uint16_t ConfigBase::getConcurrency() const {
	return 0;
}

void ConfigBase::freeTempDirectories() {
	if (config->tempDirs != NULL) {
		Free(config->tempDirs);
		config->tempDirs = NULL;
	}
	config->tempDirCount = 0;
}

void ConfigBase::mapTempDirectories() {
	config->tempDirCount = (int)tempDirs.size();
	config->tempDirs = (const char**)Malloc(
		config->tempDirCount * sizeof(const char*));
	for (int i = 0; i < config->tempDirCount; i++) {
		config->tempDirs[i] = tempDirs.at(i).c_str();
	}
}