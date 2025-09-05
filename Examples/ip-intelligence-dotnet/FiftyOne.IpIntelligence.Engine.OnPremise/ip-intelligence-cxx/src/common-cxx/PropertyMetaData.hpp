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

#ifndef FIFTYONE_DEGREES_PROPERTY_META_DATA_HPP
#define FIFTYONE_DEGREES_PROPERTY_META_DATA_HPP

#include <string>
#include <vector>
#include "EntityMetaData.hpp"


namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Meta data relating to a property populated by an engine
		 * implementation.
		 */
		class PropertyMetaData : public EntityMetaData<string> {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Default constructor, should not be used externally as it
			 * produces an invalid instance
			 */
			PropertyMetaData();

			/**
			 * Construct a new instance of PropertyMetaData from an existing
			 * instance. This copies the existing instance and does not hold a
			 * reference to it.
			 * @param property to copy
			 */
			PropertyMetaData(PropertyMetaData *property);

			/**
			 * Construct a new instance of PropertyMetaData. This should only
			 * be used internally by the Collection class.
			 * @param name of the property
			 * @param dataFilesWherePresent names of data files where the
			 * property is available
			 * @param type string indicating the variable type e.g. `int`
			 * @param category name of the category which the property belongs
			 * to
			 * @param url relating to the property
			 * @param available bool indicating whether the property is
			 * available in the dataset
			 * @param displayOrder the order of importance to use when
			 * displaying the property
			 * @param isMandatory true if the property is mandatory
			 * @param isList true if the value of the property is a list
			 * @param isObsolete true if the property is obsolete
			 * @param show true if the property should be displayed
			 * @param showValues true if the values for the property should be
			 * displayed
			 * @param description the full description of the property
			 * @param defaultValue string representation of the default value
			 * in the dataset for this property. Used when loading the default
			 * value from the value meta data collection.
			 * @param componentId the unique id of the component which the
			 * property belongs to
			 * @param evidenceProperties list of indexes in the properties
			 * collection for this property's evidence properties
			 */
			PropertyMetaData(
				string name,
				vector<string> dataFilesWherePresent,
				string type,
				string category,
				string url,
				bool available,
				byte displayOrder,
				bool isMandatory,
				bool isList,
				bool isObsolete,
				bool show,
				bool showValues,
				string description,
				string defaultValue,
				byte componentId,
				vector<uint32_t> evidenceProperties);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the name of the property.
			 * @return string representing the property
			 */
			string getName() const;

			/**
			 * Get the list of data file tiers where the property is available
			 * e.g. Premium or Enterprise.
			 * @return vector containing data file names
			 */
			vector<string> getDataFilesWherePresent() const;

			/**
			 * Get the type of data which the property refers to e.g. `int`
			 * @return string indicating the variable type
			 */
			string getType() const;

			/**
			 * Get the category which the property belongs to e.g. `device`.
			 * @return string indicating the category
			 */
			string getCategory() const;

			/**
			 * Get the URL related to the property.
			 * @return URL string
			 */
			string getUrl() const;

			/**
			 * Get whether or not the property is available in the active data
			 * set.
			 * @return true if the property is available, otherwise false.
			 */
			bool getAvailable() const;

			/**
			 * Get the order in which the property should be displayed.
			 * @return display order
			 */
			int getDisplayOrder() const;

			/**
			 * Get whether or not the property is mandatory. This means that it
			 * must be populated in the data set.
			 * @return true if the property is mandatory
			 */
			bool getIsMandatory() const;

			/**
			 * Get whether or not the value of the property is a list. If so,
			 * the a result can contain multiple values for this property.
			 * @return true if the value of the property is a list
			 */
			bool getIsList() const;

			/**
			 * Get whether or not the property is obsolete. If so, the values
			 * may not be exported in the data set.
			 * @return true if the property is obsolete
			 */
			bool getIsObsolete() const;

			/**
			 * Get whether or not the property should be displayed.
			 * @return true if the property should be displayed
			 */
			bool getShow() const;

			/**
			 * Get whether or not the values of the property should be
			 * displayed.
			 * @return true if the values of the property should be displayed
			 */
			bool getShowValues() const;

			/**
			 * Get the full description of the property.
			 * @return description string
			 */
			string getDescription() const;

			/**
			 * Gets the default value as a string for the property. This can be
			 * used along with the property itself to get the value meta data
			 * from the value meta data collection.
			 * @return default value string
			 */
			string getDefaultValue() const;

			/**
			 * Get the component id of the property. This can be used to get
			 * the component meta data from the component meta data collection.
			 * @return component id
			 */
			byte getComponentId() const;

			/**
			 * Get the evidence property indexes for the property.
			 * @return indexes for evidence properties
			 */
			vector<uint32_t> getEvidenceProperties() const;

			/**
			 * @}
			 */
		private:
			/** Names of the data file containing the property */
			vector<string> dataFilesWherePresent;

			/** Name of the property type e.g. `int` */
			string type;

			/** Category the property belongs to */
			string category;

			/** URL containing more information on the property */
			string url;

			/** True if the property is available */
			bool available;

			/** Order in which the property should be displayed */
			byte displayOrder;

			/** True if the property is mandatory in the data set */
			bool isMandatory;

			/** True if the values for this property are a list */
			bool isList;

			/** True if the property is obsolete */
			bool isObsolete;

			/** True if the property should be displayed */
			bool show;

			/** True if the property values should be shown */
			bool showValues;

			/** Full description for the property */
			string description;

			/** Default value for the property as a string */
			string defaultValue;

			/** Unique id of the component the property relates to */
			byte componentId;

			/** Indexes in the properties collection for this property's evidence
			properties */
			vector<uint32_t> evidenceProperties;
		};
	}
}

#endif