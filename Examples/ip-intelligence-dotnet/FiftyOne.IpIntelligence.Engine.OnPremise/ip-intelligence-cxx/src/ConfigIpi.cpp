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

#include <algorithm>
#include "ConfigIpi.hpp"

using namespace std;
using namespace FiftyoneDegrees::IpIntelligence;

ConfigIpi::ConfigIpi() : ConfigBase(&this->config.b) {
	config = fiftyoneDegreesIpiInMemoryConfig;
	initCollectionConfig();
}

ConfigIpi::ConfigIpi(fiftyoneDegreesConfigIpi *config) :
	ConfigBase(&config->b) {
	this->config = config != nullptr ?
		*config : fiftyoneDegreesIpiInMemoryConfig;
	initCollectionConfig();
}

void ConfigIpi::setPerformanceFromExistingConfig(
	const fiftyoneDegreesConfigIpi &existing) {
	const fiftyoneDegreesConfigBase b = config.b;
	config = existing;
	config.b = b;
	config.b.allInMemory = existing.b.allInMemory;
}

void ConfigIpi::setHighPerformance() {
	setPerformanceFromExistingConfig(fiftyoneDegreesIpiHighPerformanceConfig);
}

void ConfigIpi::setBalanced() {
	setPerformanceFromExistingConfig(fiftyoneDegreesIpiBalancedConfig);
}

void ConfigIpi::setBalancedTemp() {
	setPerformanceFromExistingConfig(fiftyoneDegreesIpiBalancedTempConfig);
}

void ConfigIpi::setLowMemory() {
	setPerformanceFromExistingConfig(fiftyoneDegreesIpiLowMemoryConfig);
}

void ConfigIpi::setMaxPerformance() {
	setPerformanceFromExistingConfig(fiftyoneDegreesIpiInMemoryConfig);
}

const CollectionConfig & ConfigIpi::getStrings() const {
	return strings;
}

const CollectionConfig & ConfigIpi::getComponents() const {
	return components;
}

const CollectionConfig & ConfigIpi::getMaps() const {
	return maps;
}

const CollectionConfig & ConfigIpi::getProperties() const {
	return properties;
}

const CollectionConfig & ConfigIpi::getValues() const {
	return values;
}

const CollectionConfig & ConfigIpi::getProfiles() const {
	return profiles;
}

const CollectionConfig & ConfigIpi::getGraphs() const {
	return graphs;
}

const CollectionConfig & ConfigIpi::getProfileGroups() const {
	return profileGroups;
}

const CollectionConfig & ConfigIpi::getProfileOffsets() const {
	return profileOffsets;
}

const CollectionConfig & ConfigIpi::getPropertyTypes() const {
	return propertyTypes;
}

const CollectionConfig & ConfigIpi::getGraph() const {
	return graph;
}

void ConfigIpi::initCollectionConfig() {
	strings = CollectionConfig(&config.strings);
	components = CollectionConfig(&config.components);
	maps = CollectionConfig(&config.maps);
	properties = CollectionConfig(&config.properties);
	values = CollectionConfig(&config.values);
	profiles = CollectionConfig(&config.profiles);
	graphs = CollectionConfig(&config.graphs);
	profileGroups = CollectionConfig(&config.profileGroups);
	profileOffsets = CollectionConfig(&config.profileOffsets);
	propertyTypes = CollectionConfig(&config.propertyTypes);
	graph = CollectionConfig(&config.graph);
}

/**
 * Gets the configuration data structure for use in C code. Used internally.
 * @return the underlying configuration data structure.
 */
fiftyoneDegreesConfigIpi &ConfigIpi::getConfig() {
	return config;
}

/**
 * Provides the lowest concurrency value in the list of possible concurrencies.
 * @return a 16 bit integer with the minimum concurrency value.
 */
uint16_t ConfigIpi::getConcurrency() const {
	uint16_t concurrencies[] = {
		strings.getConcurrency(),
		components.getConcurrency(),
		maps.getConcurrency(),
		properties.getConcurrency(),
		values.getConcurrency(),
		profiles.getConcurrency(),
		graphs.getConcurrency(),
		profileGroups.getConcurrency(),
		profileOffsets.getConcurrency(),
		propertyTypes.getConcurrency(),
		graph.getConcurrency(),
	};
	return *min_element(concurrencies, 
		concurrencies + (sizeof(concurrencies) / sizeof(uint16_t)));
}

void ConfigIpi::setConcurrency(uint16_t concurrency) {
	strings.setConcurrency(concurrency);
	components.setConcurrency(concurrency);
	maps.setConcurrency(concurrency);
	properties.setConcurrency(concurrency);
	values.setConcurrency(concurrency);
	profiles.setConcurrency(concurrency);
	graphs.setConcurrency(concurrency);
	profileGroups.setConcurrency(concurrency);
	profileOffsets.setConcurrency(concurrency);
	propertyTypes.setConcurrency(concurrency);
	graph.setConcurrency(concurrency);
}