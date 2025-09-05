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

#ifndef FIFTYONE_DEGREES_RESULTS_BASE_HPP
#define FIFTYONE_DEGREES_RESULTS_BASE_HPP

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "Exceptions.hpp"
#include "Value.hpp"
#include "RequiredPropertiesConfig.hpp"
#include "results.h"
#include "resource.h"

using std::shared_ptr;
using std::stringstream;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Encapsulates the results of an engine's processing. The class is
		 * constructed using an instance of a C #fiftyoneDegreesResultsBase
		 * structure which is then referenced to return associated values and
		 * metrics. Any memory used by the results is freed by the extending
		 * class.
		 *
		 * Values contained in a results instance can be returned as a string,
		 * or as a type specified by the method used to fetch the value. For
		 * example, the #getValueAsBool(int) method returns a value as a bool
		 * instead of a string representation.
		 *
		 * The key used to get the value for a property can be either the name
		 * of the property, or the index of the property in the required
		 * properties structure.
		 *
		 * Results instances should only be created by a Engine.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * ResultsBase *results;
		 *
		 * // Iterate over all property indexes
		 * for (int i = 0; i < results->getAvailableProperties(); i++) {
		 *
		 *     // Get the value for the property as a string
		 *     string value = *results->getValueAsString(i);
		 *
		 *     // Do something with the value
		 *     // ...
		 * }
		 *
		 * // Or get a value using the name of the property
		 * string value = *results->getValueAsString("name of a property");
		 *
		 * // Delete the results
		 * delete results;
		 * ```
		 */
		class ResultsBase {
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			/**
			 * Create a new instance of Results from the results structure
			 * provided.
			 *
			 * This method should only be called from inside an engine's
			 * process method.
			 * @param results pointer to the underlying results structure
			 * @param manager shared pointer to the manager which manages the
			 * data set used to create the results. This is needed for
			 * thread-safe operation, see local variable description for more
			 * info.
			 */
			ResultsBase(
				fiftyoneDegreesResultsBase *results,
				shared_ptr<fiftyoneDegreesResourceManager> manager);

			/**
			 * Free any memory associated with the results and release any
			 * resource handles.
			 */
			virtual ~ResultsBase();

			/**
			 * @}
			 * @name Available Properties
			 * @{
			 */

			/**
			 * Get the number of available properties contained in the Results
			 * instance.
			 * @return the number of available properties
			 */
			int getAvailableProperties() const;

			/**
			 * Get whether or not this results instance contains a value for
			 * the requested property.
			 * @param propertyName name of the property to check for
			 * @return true if there is a value for the requested property
			 */
			bool containsProperty(const string &propertyName) const;

			/**
			 * Get the names of the properties which are available in the
			 * Results instance. The index of the property in the vector
			 * indicates its index in the Results instance, so a name's index
			 * can be used to fetch its corresponding value via a get method.
			 * @return vector containing the names of all available properties
			 */
			vector<string> getProperties() const;

			/**
			 * Get the name of the property at the require property index, or
			 * an empty string if the required property index is invalid.
			 * @param requiredPropertyIndex of the property name required
			 * @return the name of the property, or an empty string if not
			 * valid
			 */
			string getPropertyName(int requiredPropertyIndex) const;

			/**
			 * @}
			 * @name Value Getters
			 * @{
			 */

			/**
			 * Get a vector with all values associated with the required
			 * property name. If the name is not valid an empty vector is
			 * returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of values for the property
			 */
			Value<vector<string>> getValues(const char *propertyName);

			/**
			 * Get a vector with all values associated with the required
			 * property name. If the name is not valid an empty vector is
			 * returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of values for the property
			 */
			Value<vector<string>> getValues(const string &propertyName);

			/**
			 * Get a vector with all values associated with the required
			 * property name. If the name is not valid an empty vector is
			 * returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of values for the property
			 */
			Value<vector<string>> getValues(const string *propertyName);

			/**
			 * Get a vector with all values associated with the required
			 * property index. If the index is not valid an empty vector is
			 * returned.
			 * @param requiredPropertyIndex of the property required
			 * @return a vector of values for the property
			 */
			Value<vector<string>> getValues(int requiredPropertyIndex);

			/**
			 * Get a string representation of the value associated with the
			 * required property name. If the property name is not valid an
			 * empty string is returned. If the property relates to a list with
			 * more than one value then values are separated by '|' characters.
			 * @param propertyName string containing the property name
			 * @return a string representation of the value for the property
			 */
			Value<string> getValueAsString(const char *propertyName);

			/**
			 * Get a string representation of the value associated with the
			 * required property name. If the property name is not valid an
			 * empty string is returned. If the property relates to a list with
			 * more than one value then values are separated by '|' characters.
			 * @param propertyName string containing the property name
			 * @return a string representation of the value for the property
			 */
			Value<string> getValueAsString(const string &propertyName);

			/**
			 * Get a string representation of the value associated with the
			 * required property name. If the property name is not valid an
			 * empty string is returned. If the property relates to a list with
			 * more than one value then values are separated by '|' characters.
			 * @param propertyName string containing the property name
			 * @return a string representation of the value for the property
			 */
			Value<string> getValueAsString(const string *propertyName);

			/**
			 * Get a string representation of the value associated with the
			 * required property index. If the index is not valid an empty
			 * string is returned. If the property relates to a list with more
			 * than one value then values are separated by '|' characters.
			 * @param requiredPropertyIndex of the property required
			 * @return a string representation of the value for the property or
			 * an empty string
			 */
			virtual Value<string> getValueAsString(int requiredPropertyIndex);

			/**
			 * Get a boolean representation of the value associated with the
			 * required property name. If the property name is not valid then
			 * false is returned.
			 * @param propertyName string containing the property name
			 * @return a boolean representation of the value for the property
			 */
			Value<bool> getValueAsBool(const char *propertyName);

			/**
			 * Get a boolean representation of the value associated with the
			 * required property name. If the property name is not valid then
			 * false is returned.
			 * @param propertyName string containing the property name
			 * @return a boolean representation of the value for the property
			 */
			Value<bool> getValueAsBool(const string &propertyName);

			/**
			 * Get a boolean representation of the value associated with the
			 * required property name. If the property name is not valid then
			 * false is returned.
			 * @param propertyName string containing the property name
			 * @return a boolean representation of the value for the property
			 */
			Value<bool> getValueAsBool(const string *propertyName);

			/**
			 * Get a boolean representation of the value associated with the
			 * required property index. If the property index is not valid then
			 * false is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a boolean representation of the value for the property
			 */
			virtual Value<bool> getValueAsBool(int requiredPropertyIndex);

			/**
			 * Get an integer representation of the value associated with the
			 * required property name. If the property name is not valid then 0
			 * is returned. Using a property which returns non-numeric
			 * characters will result in unexpected behavior.
			 * @param propertyName string containing the property name
			 * @return an integer representation of the value for the property
			 */
			Value<int> getValueAsInteger(const char *propertyName);

			/**
			 * Get an integer representation of the value associated with the
			 * required property name. If the property name is not valid then 0
			 * is returned. Using a property which returns non-numeric
			 * characters will result in unexpected behavior.
			 * @param propertyName string containing the property name
			 * @return an integer representation of the value for the property
			 */
			Value<int> getValueAsInteger(const string &propertyName);

			/**
			 * Get an integer representation of the value associated with the
			 * required property name. If the property name is not valid then 0
			 * is returned. Using a property which returns non-numeric
			 * characters will result in unexpected behavior.
			 * @param propertyName string containing the property name
			 * @return an integer representation of the value for the property
			 */
			Value<int> getValueAsInteger(const string *propertyName);

			/**
			 * Get an integer representation of the value associated with the
			 * required property index. If the property index is not valid then
			 * 0 is returned. Using a property which returns non-numeric
			 * characters will result in unexpected behavior.
			 * @param requiredPropertyIndex in the required properties
			 * @return an integer representation of the value for the property
			 */
			virtual Value<int> getValueAsInteger(int requiredPropertyIndex);

			/**
			 * Get a double representation of the value associated with the
			 * required property name. If the property name is not valid then 0
			 * is returned. Using a property which returns non-numeric
			 * characters will result in unexpected behavior.
			 * @param propertyName string containing the property name
			 * @return a double representation of the value for the property
			 */
			Value<double> getValueAsDouble(const char *propertyName);

			/**
			 * Get a double representation of the value associated with the
			 * required property name. If the property name is not valid then 0
			 * is returned. Using a property which returns non-numeric
			 * characters will result in unexpected behavior.
			 * @param propertyName string containing the property name
			 * @return a double representation of the value for the property
			 */
			Value<double> getValueAsDouble(const string &propertyName);

			/**
			 * Get a double representation of the value associated with the
			 * required property name. If the property name is not valid then 0
			 * is returned. Using a property which returns non-numeric
			 * characters will result in unexpected behavior.
			 * @param propertyName string containing the property name
			 * @return a double representation of the value for the property
			 */
			Value<double> getValueAsDouble(const string *propertyName);

			/**
			 * Get a double representation of the value associated with the
			 * required property index. If the property index is not valid then
			 * 0 is returned. Using a property which returns non-numeric
			 * characters will result in unexpected behavior.
			 * @param requiredPropertyIndex in the required properties
			 * @return a double representation of the value for the property
			 */
			virtual Value<double> getValueAsDouble(int requiredPropertyIndex);

			/**
			 * @}
			 */

		protected:
			/** Pointer to the underlying available properties structure. */
			fiftyoneDegreesPropertiesAvailable *available;

			/**
			 * Get the index in the available properties for the property name
			 * provided.
			 * @return 0 based index or -1 if not found
			 */
			int getRequiredPropertyIndex(const char *propertyName);

			/**
			 * Get the values for the index in required properties and add them
			 * to the values vector supplied. This is implemented by extending
			 * classes.
			 * @param requiredPropertyIndex index in the available properties
			 * @param values vector to populate with the values for the
			 * property
			 */
			virtual void getValuesInternal(
				int requiredPropertyIndex,
				vector<string> &values) = 0;

            /**
             * Get whether or not there are valid values available for the
             * property identified by its index in the required properties.
             * This is implemented by extending classes, and used when
             * populating a Value instance to return.
             * @param requiredPropertyIndex index in the available properties
             * @return true if there are values available for the property
             */
			virtual bool hasValuesInternal(int requiredPropertyIndex) = 0;

            /**
             * Get the message explaining the reason for missing values. This
             * can differ slightly between APIs, so the implementation of this
             * is left up to the extending class. This is used when populating
             * a Value instance to return.
             * @param reason the enum indicating the reason no values are
             * available
             * @return string explaining the reason in more detail
             */
			virtual const char* getNoValueMessageInternal(
				fiftyoneDegreesResultsNoValueReason reason) = 0;

            /**
             * Get the reason for values not being available. This is
             * implemented by the extending class and is called when the
             * hasValuesInternal method returns false.
             * @param requiredPropertyIndex index in the available properties
             * @return enum indicating the reason for values not being
             * available
             */
			virtual fiftyoneDegreesResultsNoValueReason getNoValueReasonInternal(
				int requiredPropertyIndex) = 0;

		private:
			/** A shared pointer to the manager is passed around and referenced
			by all instances that hold open a resource handle. This acts as a
			counter to ensure that the pointer to the manager remains valid
			until the last handle is freed. The shared pointer also handles
			freeing the pointer once no references remain.
			**IMPORTANT** : Although this pointer is not used, the shared
			pointer reference is required by the resource manager logic. See
			resource.h for more information. */
			shared_ptr<fiftyoneDegreesResourceManager> manager;
		};
	}
}

#endif