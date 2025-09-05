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

#include "MetaDataIpi.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

// Define the default profile id of a dynamic component
#define DYNAMIC_COMPONENT_DEFAULT_PROFILE_ID 0

MetaDataIpi::MetaDataIpi(
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: MetaData(manager) {
}

MetaDataIpi::~MetaDataIpi() {
}

Collection<::byte, ComponentMetaData>* MetaDataIpi::getComponents() const
{
	return new ComponentMetaDataCollectionIpi(manager.get());
}

Collection<string, PropertyMetaData>* MetaDataIpi::getProperties() const
{
	return new PropertyMetaDataCollectionIpi(manager.get());
}

Collection<ValueMetaDataKey, ValueMetaData>* MetaDataIpi::getValues() const
{
	return new ValueMetaDataCollectionIpi(manager.get());
}

Collection<uint32_t, ProfileMetaData>* MetaDataIpi::getProfiles() const
{
	return new ProfileMetaDataCollectionIpi(manager.get());
}

Collection<ValueMetaDataKey, ValueMetaData>*
MetaDataIpi::getValuesForProperty(
	PropertyMetaData *property) const {
	return new ValueMetaDataCollectionForPropertyIpi(
		manager.get(),
		property);
}

Collection<ValueMetaDataKey, ValueMetaData>*
MetaDataIpi::getValuesForProfile(
	ProfileMetaData *profile) const {
	const shared_ptr<fiftyoneDegreesResourceManager> managerRef = manager;
	fiftyoneDegreesResourceManager* const managerPtr = managerRef.get();
	return new ValueMetaDataCollectionForProfileIpi(
		managerPtr,
		profile);
}

ComponentMetaData* MetaDataIpi::getComponentForProfile(
	ProfileMetaData *profile) const {
	ComponentMetaData *result = nullptr;
	Collection<::byte, ComponentMetaData> *components = getComponents();
	if (components != nullptr) {
		result = components->getByKey(profile->getComponentId());
		delete components;
	}
	return result;
}

ComponentMetaData* MetaDataIpi::getComponentForProperty(
	PropertyMetaData *property) const {
	ComponentMetaData *result = nullptr;
	Collection<::byte, ComponentMetaData> *components = getComponents();
	if (components != nullptr) {
		result = components->getByKey(property->getComponentId());
		delete components;
	}
	return result;
}

ProfileMetaData* MetaDataIpi::getDefaultProfileForComponent(
	ComponentMetaData *component) const {
	ProfileMetaData *result = nullptr;
	Collection<uint32_t, ProfileMetaData> *profiles = getProfiles();
	if (profiles != nullptr) {
		// Make sure that it is not a dynamic component
		if (component->getDefaultProfileId() != 0) {
			result = profiles->getByKey(component->getDefaultProfileId());
		}
		delete profiles;
	}
	return result;
}

ValueMetaData* MetaDataIpi::getDefaultValueForProperty(
	PropertyMetaData *property) const {
	ValueMetaData *result = nullptr;
	Collection<ValueMetaDataKey, ValueMetaData> *values = getValues();
	if (values != nullptr) {
		result = values->getByKey(ValueMetaDataKey(
			property->getName(), 
			property->getDefaultValue()));
		delete values;
	}
	return result;
}

Collection<string, PropertyMetaData>*
MetaDataIpi::getPropertiesForComponent(
	ComponentMetaData *component) const {
	return new PropertyMetaDataCollectionForComponentIpi(
		manager.get(),
		component);
}

Collection<string, PropertyMetaData>*
MetaDataIpi::getEvidencePropertiesForProperty(
	PropertyMetaData *property) const {
	return new PropertyMetaDataCollectionForPropertyIpi(
		manager.get(),
		property);
}

PropertyMetaData* MetaDataIpi::getPropertyForValue(
	ValueMetaData *value) const {
	PropertyMetaData *result = nullptr;
	Collection<string, PropertyMetaData> *properties = getProperties();
	if (properties != nullptr) {
		result = properties->getByKey(value->getKey().getPropertyName());
		delete properties;
	}
	return result;
}