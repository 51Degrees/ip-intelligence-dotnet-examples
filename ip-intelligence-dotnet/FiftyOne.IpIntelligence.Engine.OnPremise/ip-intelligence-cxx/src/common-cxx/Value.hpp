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

#ifndef FIFTYONE_DEGREES_VALUE_HPP
#define FIFTYONE_DEGREES_VALUE_HPP

#include "Exceptions.hpp"
#include "results.h"


namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Encapsulates a value returned an instance of ResultsBase for a
		 * specified property. An instance contains either the value, or the
		 * reason which there is no value.
		 *
		 * Before retrieving the value itself, its presence should first be
		 * checked using the hasValue() method. The typed value can be
		 * returned either using the getValue() method, or more simply, by
		 * dereferencing (similarly to an iterator). If there is no value, then
		 * an exception will be thrown.
		 *
		 * ## Usage Example
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * Value<string> value;
		 *
		 * // Check that there is a valid value
		 * if (value.hasValue()) {
		 *     // Get the value
		 *     string stringValue = *value;
		 *     // Do something with the value
		 *     // ...
		 * }
		 * else {
		 *     // Get the reason for the value being invalid
		 *     string message = value.getNoValueMessage();
		 *     // Do something with the message
		 *     // ...
		 * }
		 *
		 * // Or just try getting the value and catch the exception. Note that
		 * // this is not the recommended method.
		 * try {
		 *     string stringValue = *value;
		 * }
		 * catch (exception e) {
		 *     const char *message = e.what();
		 * }
		 * ```
		 *
		 * @tparam T the type of the value e.g. string
		 */
		template <class T> class Value {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Default constructor.
			 */
			Value() {
				this->value = T();
				this->hasValueInternal = false;
			}

			/**
			 * @}
			 * @name Value Methods
			 * @{
			 */

			/**
			 * Indicates whether or not a valid value has been returned by the
			 * ResultsBase instance.
			 * @return true if there is a valid value
			 */
			bool hasValue() {
				return hasValueInternal;
			}

            /**
             * Indicates the reason why valid values are not available. This
             * can be called if hasValue() returns false. For a more detailed
             * message, call getNoValueMessage().
             * @return enum indicating the reason for no valid values
             */
			fiftyoneDegreesResultsNoValueReason getNoValueReason() {
				return noValueReason;
			}

			/**
			 * Gets a message explaining why there is no value. Calling this if
			 * hasValue() returned true will result in undefined behavior.
			 * @return message explaining the reason for the missing value
			 */
			const char* getNoValueMessage() {
				return noValueMessage;
			}

			/**
			 * Gets the value contained in the Value instance. If there is no
			 * valid value, then an exception will be thrown. To prevent an
			 * exception, the hasValue() method should be checked first.
			 * @return value
			 */
			T getValue() {
				if (hasValueInternal) {
					return value;
				}
				else {
					switch (noValueReason) {
					case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY:
						if (noValueMessage != nullptr) {
							throw InvalidPropertyException(noValueMessage);
						}
						else {
							throw InvalidPropertyException();
						}
					case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES:
						throw TooManyValuesException();
					case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_DIFFERENCE:
					case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULT_FOR_PROPERTY:
					case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS:
					case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN:
						throw NoValuesAvailableException(noValueMessage);
					default:
						throw NoValuesAvailableException();
					}
				}
			}

			/**
			 * Set the value to be contained in the Value instance.
			 * @param targetValue the value to set
			 */
			void setValue(T targetValue) {
				this->value = targetValue;
				hasValueInternal = true;
			}

			/**
			 * Set the reason there is no value available. This will be used
			 * if an exception is thrown by the setValue method.
			 * @param reason the reason enum indicating why the value is
			 * missing
			 * @param message the message containing a fuller description on
			 * why the value is missing
			 */
			void setNoValueReason(
				fiftyoneDegreesResultsNoValueReason reason,
				const char *message) {
				this->noValueReason = reason;
				this->noValueMessage = message;
			}

			/**
			 * @}
			 * @name Operators
			 * @{
			 */

			/**
			 * Gets the value contained in the Value instance. If there is no
			 * valid value, then an exception will be thrown. To prevent an
			 * exception, the hasValue() method should be checked first.
			 * @return value
			 */
			T operator*() {
				return getValue();
			}

			/**
			 * @}
			 */

		private:

			bool hasValueInternal;
			T value;
			const char *noValueMessage = nullptr;
			fiftyoneDegreesResultsNoValueReason noValueReason =
				FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN;
		};
	}
}

#endif
