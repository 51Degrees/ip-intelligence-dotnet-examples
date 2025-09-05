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

#ifndef FIFTYONE_DEGREES_RESULTS_IPI_HPP
#define FIFTYONE_DEGREES_RESULTS_IPI_HPP

#include <sstream>
#include <vector>
#include "common-cxx/ResultsBase.hpp"
#include "WeightedValue.hpp"
#include "common-cxx/IpAddress.hpp"
#include "ipi.h"
#include <functional>


class EngineIpIntelligenceTests;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		using std::shared_ptr;
		using std::string;
		using std::vector;
		using FiftyoneDegrees::Common::ResultsBase;

		/**
		 * Encapsulates the results of an IP Intelligence engine's
		 * processing. The class is constructed using an instance of a C
		 * #fiftyoneDegreesResultsIpi structure which are then
		 * referenced to return associated values and metrics.
		 *
		 * The key used to get the value for a property can be either the
		 * name of the property, or the index of the property in the
		 * required properties structure.
		 *
		 * Results instances should only be created by an Engine.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::IpIntelligence;
		 * ResultsIpi *results;
		 *
		 * // Get property value as a coordinate for property "AverageLocation"
		 * Value<fiftyoneDegreesCoordinate> coordinate =
		 *	getValueAsCoordinate("AverageLocation");
		 *
		 * // Get the network id for the IP range
		 * string networkId = results->getNetworkId();
		 *
		 * // Delete the results
		 * delete results;
		 * ```
		 */
		class ResultsIpi : public ResultsBase {
			friend class ::EngineIpIntelligenceTests;
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			 /**
			  * @copydoc Common::ResultsBase::ResultsBase
			  */
			ResultsIpi(
				fiftyoneDegreesResultsIpi *results,
				shared_ptr<fiftyoneDegreesResourceManager> manager);

			/**
			 * Release the reference to the underlying results
			 * and associated data set.
			 */
			~ResultsIpi() override;

			/**
			 * @}
			 * @name Value Getters
			 * @{
			 */

			/**
			 * Get a vector with all weighted boolean representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted boolean values for the property
			 */
			Common::Value<vector<WeightedValue<bool>>>
				getValuesAsWeightedBoolList(const char *propertyName);

			/**
			 * Get a vector with all weighted boolean representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted boolean values for the property
			 */
			Common::Value<vector<WeightedValue<bool>>>
				getValuesAsWeightedBoolList(const string *propertyName);
			
			/**
			 * Get a vector with all weighted boolean representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted boolean values for the property
			 */
			Common::Value<vector<WeightedValue<bool>>>
				getValuesAsWeightedBoolList(const string &propertyName);

			/**
			 * Get a vector with all weighted boolean representations of the 
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted boolean values for the property
			 */
			Common::Value<vector<WeightedValue<bool>>>
				getValuesAsWeightedBoolList(int requiredPropertyIndex);
			
			/**
			 * Get a vector with all weighted string representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
				getValuesAsWeightedStringList(const char *propertyName);

			/**
			 * Get a vector with all weighted string representations of the
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<std::vector<uint8_t>>>>
				getValuesAsWeightedUTF8StringList(int requiredPropertyIndex);

			/**
			 * Get a vector with all weighted byte vector representations of the
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted byte vector values for the property
			 */
			Common::Value<vector<WeightedValue<std::vector<uint8_t>>>>
				getValuesAsWeightedUTF8StringList(const char *propertyName);

			/**
			 * Get a vector with all weighted byte vector representations of the
			 * values associated with the required property name. If the name
			* is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted byte vector values for the property
			 */
			Common::Value<vector<WeightedValue<std::vector<uint8_t>>>>
				getValuesAsWeightedUTF8StringList(const string &propertyName);

			/**
			 * Get a vector with all weighted string representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
				getValuesAsWeightedStringList(const string *propertyName);
			
			/**
			 * Get a vector with all weighted string representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
				getValuesAsWeightedStringList(const string &propertyName);

			/**
			 * Get a vector with all weighted string representations of the 
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
				getValuesAsWeightedStringList(int requiredPropertyIndex);

			/**
			 * Get a vector with all weighted string representations of the
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @param decimalPlaces precision (places after decimal dot)
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
				getValuesAsWeightedWKTStringList(
					const char *propertyName, ::byte decimalPlaces);

			/**
			 * Get a vector with all weighted string representations of the
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @param decimalPlaces precision (places after decimal dot)
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
				getValuesAsWeightedWKTStringList(
					const string *propertyName, ::byte decimalPlaces);

			/**
			 * Get a vector with all weighted string representations of the
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @param decimalPlaces precision (places after decimal dot)
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
				getValuesAsWeightedWKTStringList(
					const string &propertyName, ::byte decimalPlaces);

			/**
			 * Get a vector with all weighted string representations of the
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @param decimalPlaces precision (places after decimal dot)
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
				getValuesAsWeightedWKTStringList(
				int requiredPropertyIndex, ::byte decimalPlaces);
			
			/**
			 * Get a vector with all weighted integer representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted integer values for the property
			 */
			Common::Value<vector<WeightedValue<int>>>
				getValuesAsWeightedIntegerList(const char *propertyName);

			/**
			 * Get a vector with all weighted integer representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted integer values for the property
			 */
			Common::Value<vector<WeightedValue<int>>>
				getValuesAsWeightedIntegerList(const string *propertyName);
			
			/**
			 * Get a vector with all weighted integer representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted integer values for the property
			 */
			Common::Value<vector<WeightedValue<int>>>
				getValuesAsWeightedIntegerList(const string &propertyName);

			/**
			 * Get a vector with all weighted integer representations of the 
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted integer values for the property
			 */
			Common::Value<vector<WeightedValue<int>>>
				getValuesAsWeightedIntegerList(int requiredPropertyIndex);

			/**
			 * Get a vector with all weighted double representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted double values for the property
			 */
			Common::Value<vector<WeightedValue<double>>>
				getValuesAsWeightedDoubleList(const char *propertyName);

			/**
			 * Get a vector with all weighted double representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted double values for the property
			 */
			Common::Value<vector<WeightedValue<double>>>
				getValuesAsWeightedDoubleList(const string *propertyName);
			
			/**
			 * Get a vector with all weighted double representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted double values for the property
			 */
			Common::Value<vector<WeightedValue<double>>>
				getValuesAsWeightedDoubleList(const string &propertyName);

			/**
			 * Get a vector with all weighted double representations of the 
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted double values for the property
			 */
			Common::Value<vector<WeightedValue<double>>>
			getValuesAsWeightedDoubleList(int requiredPropertyIndex);

			/**
			 * Get an IpAddress instance representation of the value associated 
			 * with the required property name. If the property name is not valid
			 * then hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return an IpAddress representation of the value for the property
			 */
			Common::Value<IpIntelligence::IpAddress> getValueAsIpAddress(
				const char *propertyName);

			/**
			 * Get an IpAddress instance representation of the value associated 
			 * with the required property name. If the property name is not valid
			 * then hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return an IpAddress representation of the value for the property
			 */
			Common::Value<IpIntelligence::IpAddress> getValueAsIpAddress(
				const string &propertyName);

			/**
			 * Get an IpAddress instance representation of the value associated 
			 * with the required property name. If the property name is not valid
			 * then hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return an IpAddress representation of the value for the property
			 */
			Common::Value<IpIntelligence::IpAddress> getValueAsIpAddress(
				const string *propertyName);
			
			/**
			 * Get an IpAddress instance representation of the value associated 
			 * with the required property index. If the index is not valid
			 * then hasValue returns false with NoValueReason and its message.
			 * @param requiredPropertyIndex index in the required
			 * properties list
			 * @return an IpAddress representation of the value for the property
			 */
			Common::Value<IpIntelligence::IpAddress> getValueAsIpAddress(
				int requiredPropertyIndex);

			/**
			 * Override the get boolean representation of the value associated
			 * with the required property name.
			 * This now always returns a no value. The reason is always too many results.
			 */
			Common::Value<bool> getValueAsBool(int requiredPropertyIndex) override;

			/**
			 * Override the get integer representation of the value associated
			 * with the required property name.
			 * This now always returns a no value. The reason is always too many results.
			 */
			Common::Value<int> getValueAsInteger(int requiredPropertyIndex) override;

			/**
			 * Override the get double representation of the value associated
			 * with the required property name.
			 * This now always returns a no value. The reason is always too many results.
			 */
			Common::Value<double> getValueAsDouble(int requiredPropertyIndex) override;

		protected:
			void getValuesInternal(
				int requiredPropertyIndex,
				vector<string> &values) override;

			bool hasValuesInternal(int requiredPropertyIndex) override;

			const char* getNoValueMessageInternal(
				fiftyoneDegreesResultsNoValueReason reason) override;

			fiftyoneDegreesResultsNoValueReason getNoValueReasonInternal(
				int requiredPropertyIndex) override;

		private:
			/**
			 * Utility function to check the property value type
			 * Consumers of this function should always check for
			 * exception status before using the returned value.
			 * @param requiredPropertyIndex index in the required
			 * properties list
			 * @param exception object which is used when an exception
			 * occurs
			 * @return the value type of the property
			 */
			fiftyoneDegreesPropertyValueType getPropertyValueType(
				int requiredPropertyIndex,
				fiftyoneDegreesException *exception);

			/**
			 * Iterates over available values for the caller to populate results.
			 * @param requiredPropertyIndex index in the required
			 * properties list
			 * @param onNoValue called if no values are available for property
			 * @param onValuesCount called when values count is known and positive
			 * @param onEachValue called for each value available
			 * @param onAfterValues called after all values have been iterated successfully
			 */
			void iterateWeightedValues(
				int requiredPropertyIndex,
				const std::function<void(
					fiftyoneDegreesResultsNoValueReason reason,
					const char *reasonStr)>& onNoValue,
				const std::function<void(uint32_t count)>& onValuesCount,
				const std::function<void(
					const fiftyoneDegreesStoredBinaryValue *binaryValue,
					fiftyoneDegreesPropertyValueType storedValueType,
					uint16_t rawWeighting,
					fiftyoneDegreesException *exception)>& onEachValue,
				const std::function<void()>& onAfterValues);

			fiftyoneDegreesResultsIpi *results;
		};
	}
}

#endif
