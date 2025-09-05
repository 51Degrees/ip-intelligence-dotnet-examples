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

#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_HPP

#include <string>
#include <vector>
#include "EntityMetaData.hpp"
#include "value.h"


namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Key used to store ValueMetaData in a Collection. A value name is not
		 * unique, however it is unique within the collection of values for a
		 * single property. For this reason, the key consists of the property
		 * name and the value name.
		 */
		class ValueMetaDataKey {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Default constructor. This should not be used externally as it
			 * returns an invalid instance.
			 */
			ValueMetaDataKey();

			/**
			 * Construct a new instance of ValueMetaDataKey from the unique
			 * combination of property and value names.
			 * @param propertyName the name of the property the value relates
			 * to
			 * @param valueName the name of the value
			 */
			ValueMetaDataKey(string propertyName, string valueName);

			/**
			 * Construct a new instance of ValueMetaDataKey from the unique
			 * combination of property and value names.
			 * @param propertyName the name of the property the value relates
			 * to
			 * @param valueName the name of the value
			 * @param valueNameLength the length of the name of the value
			 */
			ValueMetaDataKey(
				const string &propertyName,
				const uint8_t *valueName,
				long valueNameLength);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the name of the property that the value keyed relates to.
			 * @return name of the property
			 */
			const string getPropertyName() const;

			/**
			 * Get the name of the value which is being keyed.
			 * @return name of the value
			 */
			const string getValueName() const;

			/**
			 * @}
			 * @name Operators
			 * @{
			 */

			/**
			 * Override the less than operator so the unique key can be used to
			 * order lists.
			 * @param other the other key to compare
			 * @return true if this key comes before the other key
			 */
			const bool operator< (ValueMetaDataKey other) const;

			/**
			 * Override the equality operator so the unique key can be found in
			 * a generic collection.
			 * @param other the other key to compare
			 * @return true if both keys are equal
			 */
			const bool operator== (ValueMetaDataKey other) const;

			/**
			 * @}
			 */
		private:
			/** The name of the property the value relates to */
			string propertyName;

			/** The value as a string */
			string valueName;
		};

		/**
		 * Meta data relating to a value populated by an engine implementation.
		 */
		class ValueMetaData : public EntityMetaData<ValueMetaDataKey> {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Default constructor, should not be used externally as it
			 * produces an invalid instance
			 */
			ValueMetaData();

			/**
			 * Construct a new instance of ValueMetaData from an existing
			 * instance. This copies the existing instance and does not hold a
			 * reference to it.
			 * @param value to copy
			 */
			ValueMetaData(ValueMetaData *value);

			/**
			 * Create a new instance of ValueMetaData. This should only be used
			 * internally by the Collection class.
			 * @param key containing the value and property names
			 * @param description full description of the value
			 * @param url relating to the value
			 */
			ValueMetaData(
				ValueMetaDataKey key,
				string description,
				string url);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the name of the value. This is a string representation of
			 * the value itself.
			 * @return value name
			 */
			string getName() const;

			/**
			 * Get the full description of the value.
			 * @return description string
			 */
			string getDescription() const;

			/**
			 * Get the URL containing any extra information about the value.
			 * @return URL string
			 */
			string getUrl() const;

			/**
			 * Get the name of the value as a UTF-8 byte vector.
			 * @return value name as UTF-8 bytes
			 */
			std::vector<uint8_t> getUtf8ValueName() const;

			/**
			 * @}
			 */
		private:
			/** Full description of the value */
			string description;

			/** URL containing more information on the value */
			string url;
		};
	}
}

#endif
