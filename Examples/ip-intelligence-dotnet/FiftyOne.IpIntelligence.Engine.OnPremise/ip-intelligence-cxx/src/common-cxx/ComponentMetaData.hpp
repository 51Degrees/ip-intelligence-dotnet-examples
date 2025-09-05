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

#ifndef FIFTYONE_DEGREES_COMPONENT_META_DATA_HPP
#define FIFTYONE_DEGREES_COMPONENT_META_DATA_HPP

#include <string>
#include "EntityMetaData.hpp"


namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * EntityMetaData relating to a component populated by an engine
		 * implementation.
		 */
		class ComponentMetaData : public EntityMetaData<byte> {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Default constructor, should not be used externally as it
			 * produces an invalid instance
			 */
			ComponentMetaData();

			/**
			 * Construct a new instance of ComponentMetaData from an existing
			 * instance. This copies the existing instance and does not hold a
			 * reference to it.
			 * @param component to copy
			 */
			ComponentMetaData(ComponentMetaData *component);

			/**
			 * Construct a new instance of ComponentMetaData. This should only
			 * be used internally by the collection class.
			 * @param componentId unique id of the component
			 * @param name the name of the component
			 * @param defaultProfileId unique id of the default profile for the
			 * component
			 */
			ComponentMetaData(
				byte componentId,
				string name,
				uint32_t defaultProfileId);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the unique if of the component as an integer.
			 * @return unique id as int
			 */
			int getComponentIdAsInt() const;

			/**
			 * Get the unique id of the component.
			 * @return unique id
			 */
			byte getComponentId() const;

			/**
			 * Get the name of the component.
			 * @return name of the component
			 */
			string getName() const;

			/**
			 * Get the unique id for the default profile for this component.
			 * @return default profile id
			 */
			uint32_t getDefaultProfileId() const;

			/**
			 * @}
			 */
		private:
			/** The name of the component */
			string name;

			/** The unique if of the default profile for this component */
			uint32_t defaultProfileId;
		};
	}
}

#endif