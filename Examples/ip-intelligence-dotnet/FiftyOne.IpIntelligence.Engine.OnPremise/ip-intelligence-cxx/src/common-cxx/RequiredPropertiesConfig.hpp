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

#ifndef FIFTYONE_DEGREES_REQUIRED_PROPERTIES_HPP
#define FIFTYONE_DEGREES_REQUIRED_PROPERTIES_HPP

#include <string>
#include <vector>
#include "properties.h"

using std::string;
using std::vector;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * C++ class wrapper for the #fiftyoneDegreesPropertiesRequired
		 * structure. It defines a set of properties which are required by a
		 * caller, usually to a data set constructor.
		 *
		 * An instance is immutable once instantiated.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * ConfigBase *config;
		 *
		 * // Construct a required properties configuration with a list of
		 * // required properties
		 * RequiredPropertiesConfig *properties =
		 *     new RequiredPropertiesConfig("a property, another property");
		 *
		 * // Use the required properties configuration when constructing an
		 * // engine
		 * EngineBase *engine = new EngineBase(config, properties);
		 * ```
		 */
		class RequiredPropertiesConfig {
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			/**
			 * Constructs a new instance of the RequiredPropertiesConfig so
			 * that all possible properties contained in the data are available
			 * for inspection.
			 */
			RequiredPropertiesConfig();

			/**
			 * Construct a new instance of required RequiredPropertiesConfig
			 * using the list of property names supplied. Once created,
			 * properties cannot be added.
			 * @param properties to enable
			 */
			RequiredPropertiesConfig(const vector<string> *properties);

			/**
			 * Construct a new instance of required RequiredPropertiesConfig
			 * using the list of property names supplied. Once created,
			 * properties cannot be added.
			 * @param properties to enable
			 */
			RequiredPropertiesConfig(vector<string> *properties);

			/**
			 * Construct a new instance of required RequiredPropertiesConfig
			 * using the list of property names supplied. Once created,
			 * properties cannot be added.
			 * @param properties to enable
			 */
			RequiredPropertiesConfig(const char *properties);

			/**
			 * Construct a new instance of required RequiredPropertiesConfig
			 * using the list of property names supplied. Once created,
			 * properties cannot be added.
			 * @param properties to enable
			 */
			RequiredPropertiesConfig(const string &properties);

			/**
			 * Construct a new instance of required RequiredPropertiesConfig
			 * using the list of property names supplied. Once created,
			 * properties cannot be added.
			 * @param properties to enable
			 */
			RequiredPropertiesConfig(const string *properties);

			/**
			 * Destroy the configuration, freeing all the memory allocated
			 * within it.
			 */
			virtual ~RequiredPropertiesConfig();

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the list of properties which are required.
			 * @return vector containing the property names
			 */
			vector<string> getProperties() const;

			/**
			 * Get a pointer to the underlying C required properties structure.
			 * This is a pointer memory internal to this class, so should not
			 * be freed.
			 * @return required properties config
			 */
			fiftyoneDegreesPropertiesRequired* getConfig();

			/**
			 * Get the number of properties contained within the required
			 * properties configuration.
			 * @return number of properties
			 */
			int getCount() const;

			/**
			 * @}
			 */

			/**
			 * Get whether or not the property name supplied is contained in
			 * the required properties configuration.
			 * @param property name of the property to find
			 * @return true if the property name is present
			 */
			bool containsProperty(const char *property) const;

			/**
			 * Get whether or not the property name supplied is contained in
			 * the required properties configuration.
			 * @param property name of the property to find
			 * @return true if the property name is present
			 */
			bool containsProperty(const string property) const;
		private:
			/** The underlying C configuration structure. */
			fiftyoneDegreesPropertiesRequired conf;
		};
	}
}

#endif