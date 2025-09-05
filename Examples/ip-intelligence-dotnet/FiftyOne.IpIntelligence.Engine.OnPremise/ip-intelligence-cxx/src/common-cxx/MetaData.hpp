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

#ifndef FIFTYONE_DEGREES_METADATA_HPP
#define FIFTYONE_DEGREES_METADATA_HPP

#include "Collection.hpp"
#include "ComponentMetaData.hpp"
#include "PropertyMetaData.hpp"
#include "ProfileMetaData.hpp"
#include "ValueMetaData.hpp"
#include "resource.h"
#include <memory>

using std::shared_ptr;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Contains meta data for the properties, values, profiles and
		 * components that exist within the engine instance. All Collections
		 * returned are new instances which hold a reference to the underlying
		 * data structures, so must be disposed of. All single meta data
		 * instances returned either by this class or the Collections it
		 * returns hold no reference and are safe to hold on to.
		 *
		 * ## Example Usage
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * EngineBase *engine;
		 *
		 * // Get the meta data from an engine
		 * MetaData *metaData = engine->getMetaData();
		 *
		 * // Get the meta data for the engine's properties
		 * Collection<string, PropertyMetaData> *properties =
		 *     metaData->getProperties();
		 *
		 * // Do something with the properties (see Collection.hpp for an
		 * // example).
		 * // ...
		 *
		 * // Delete the properties collection
		 * delete properties;
		 *
		 * // Delete the meta data instance
		 * delete metaData;
		 * ```
		 */
		class MetaData {
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			/**
			 * Construct a new instance of MetaData. This should only be used
			 * internally by engines.
			 * @param manager shared pointer to the manager instance to get
			 * data from
			 */
			MetaData(shared_ptr<fiftyoneDegreesResourceManager> manager);

			/**
			 * Free any data and handles used for the meta data.
			 */
			virtual ~MetaData();

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get a new Collection instance of all component meta data keyed
			 * on the unique id of the component.
			 * @return all components
			 */
			virtual Collection<byte, ComponentMetaData>* getComponents() const = 0;

			/**
			 * Get a new Collection instance of all property meta data keyed on
			 * the name of the property.
			 * @return all properties
			 */
			virtual Collection<string, PropertyMetaData>* getProperties() const = 0;

			/**
			 * Get a new Collection instance of all profile meta data keyed on
			 * the unique id of the profile.
			 * @return all profiles
			 */
			virtual Collection<uint32_t, ProfileMetaData>* getProfiles() const = 0;

			/**
			 * Get a new Collection instance of all value meta data keyed on
			 * the name of the value and the property it relates to.
			 * @return all values
			 */
			virtual Collection<ValueMetaDataKey, ValueMetaData>* getValues() const = 0;

			/**
			 * @}
			 * @name Filtered Getters
			 * @{
			 */

			/**
			 * Get a new Collection instance of value meta data relating to the
			 * specified property, keyed on the name of the value and the
			 * property.
			 * @param property to get the values for
			 * @return values for the property
			 */
			virtual Collection<ValueMetaDataKey, ValueMetaData>*  
				getValuesForProperty(PropertyMetaData *property) const = 0;

			/**
			 * Get a new Collection instance of value meta data for the
			 * specified profile, keyed on the name of the value and the
			 * property it relates to.
			 * @param profile to get the values for
			 * @return values for the profile
			 */
			virtual Collection<ValueMetaDataKey, ValueMetaData>* 
				getValuesForProfile(ProfileMetaData *profile) const = 0;

			/**
			 * Get the Component which the specified profile relates to.
			 * @param profile to get the component for
			 * @return component the profile relates to
			 */
			virtual ComponentMetaData* getComponentForProfile(
				ProfileMetaData *profile) const = 0;

			/**
			 * Get the Component which the specified property relates to.
			 * @param property to get the component for
			 * @return component the property relates to
			 */
			virtual ComponentMetaData* getComponentForProperty(
				PropertyMetaData *property) const = 0;

			/**
			 * Get the default profile for the specified component. This is the
			 * profile that will be used if there is no match.
			 * @param component to get the default profile for
			 * @return default profile for the component
			 */
			virtual ProfileMetaData* getDefaultProfileForComponent(
				ComponentMetaData *component) const = 0;

			/**
			 * Get the default value for the specified component. This is the
			 * value that will be used if a profile does not contain a value
			 * for the property.
			 * @param property to get the default value for
			 * @return default value for the property
			 */
			virtual ValueMetaData* getDefaultValueForProperty(
				PropertyMetaData *property) const = 0;

			/**
			 * Get a new Collection instance of the property meta data relating
			 * to the specified component, keyed on the name of the property.
			 * These are the properties which a profile relating to the
			 * component will contain.
			 * @param component to get the properties for
			 * @return properties for the component
			 */
			virtual Collection<string, PropertyMetaData>* 
				getPropertiesForComponent(ComponentMetaData *component) const = 0;

			/**
			 * Get the property which the value relates to.
			 * @param value to get the property for
			 * @return property the value relates to
			 */
			virtual PropertyMetaData* getPropertyForValue(
				ValueMetaData *value) const = 0;

			/**
			 * Get the properties which are required to fetch extra evidence for
			 * a specified property. If a property is not available (i.e. not set
			 * as a required property on engine construction) this will always be
			 * empty.
			 * @param property to get the evidence properties for
			 * @return evidence properties for the property
			 */
			virtual Collection<string, PropertyMetaData>*
				getEvidencePropertiesForProperty(PropertyMetaData *property) const = 0;

			/**
			 * @}
			 */
		protected:
			/** A shared pointer to the manager is passed around and referenced
			by all instances that hold open a resource handle. This acts as a
			counter to ensure that the pointer to the manager remains valid
			until the last handle is freed. The shared pointer also handles
			freeing the pointer once no references remain. See resource.h for
			more information. */
			shared_ptr<fiftyoneDegreesResourceManager> manager;

			/**
			 * Get a value from the collection and copy it as string to a C++ string
			 * instance. This method releases the collection item before
			 * returning.
			 * @param strings pointer to the collection containing the string
			 * @param offset of the string in the collection
			 * @param storedValueType format of byte array representation.
			 * @return copy of the requested value from the collection as string
			 */
			string getValue(
				fiftyoneDegreesCollection *strings,
				uint32_t offset,
				fiftyoneDegreesPropertyValueType storedValueType);
		};
	}
}

#endif