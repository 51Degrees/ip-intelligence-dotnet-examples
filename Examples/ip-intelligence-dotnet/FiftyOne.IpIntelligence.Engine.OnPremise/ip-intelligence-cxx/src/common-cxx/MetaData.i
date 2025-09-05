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

%include std_vector.i
%include std_string.i

%include "Collection.i"
%include "PropertyMetaData.i"
%include "ComponentMetaData.i"
%include "ProfileMetaData.i"
%include "ValueMetaData.i"

%template(ComponentMetaDataCollectionSwig) Collection<byte,ComponentMetaData>;
%template(PropertyMetaDataCollectionSwig) Collection<std::string,PropertyMetaData>;
%template(ProfileMetaDataCollectionSwig) Collection<uint32_t,ProfileMetaData>;
%template(ValueMetaDataCollectionSwig) Collection<ValueMetaDataKey,ValueMetaData>;

%rename (MetaDataSwig) MetaData;

%nodefaultctor MetaData;

%newobject getComponents;
%newobject getProperties;
%newobject getProfiles;
%newobject getValues;

%newobject getValuesForProperty;
%newobject getValuesForProfile;
%newobject getComponentForProfile;
%newobject getComponentForProperty;
%newobject getDefaultProfileForComponent;
%newobject getDefaultValueForProperty;
%newobject getPropertiesForComponent;
%newobject getPropertyForValue;

class MetaData
{
public:
	~MetaData();

	Collection<byte, ComponentMetaData>* getComponents();
	Collection<std::string, PropertyMetaData>* getProperties();
	Collection<uint32_t, ProfileMetaData>* getProfiles();
	Collection<ValueMetaDataKey, ValueMetaData>* getValues();

	Collection<ValueMetaDataKey, ValueMetaData>* getValuesForProperty(
		PropertyMetaData *property);
	Collection<ValueMetaDataKey, ValueMetaData>* getValuesForProfile(
		ProfileMetaData *profile);
	ComponentMetaData* getComponentForProfile(
		ProfileMetaData *profile);
	ComponentMetaData* getComponentForProperty(
		PropertyMetaData *property);
	ProfileMetaData* getDefaultProfileForComponent(
		ComponentMetaData *component);
	ValueMetaData* getDefaultValueForProperty(
		PropertyMetaData *property);
	Collection<std::string, PropertyMetaData>* getPropertiesForComponent(
		ComponentMetaData *component);
	PropertyMetaData* getPropertyForValue(
		ValueMetaData *value);
};