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

#ifndef FIFTYONE_DEGREES_EXCEPTIONS_HPP
#define FIFTYONE_DEGREES_EXCEPTIONS_HPP

#include <string>
#include <exception>
#include <stdexcept>
#include <memory>
#include "exceptions.h"
#include "status.h"
#include "memory.h"

using std::runtime_error;
using std::string;
using std::exception;
using std::shared_ptr;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Exception containing the status code which caused the exception.
		 */
		class StatusCodeException : public exception {
		public:
			/**
			 * New exception from the status code provided.
			 * @param code indicating the error
			 */
			StatusCodeException(fiftyoneDegreesStatusCode code);

			/**
			 * New exception from the status code provided and the file name
			 * relating to the operation which failed.
			 * @param code indicating the error
			 * @param fileName relating the failed operation
			 */
			StatusCodeException(
				fiftyoneDegreesStatusCode code,
				const char *fileName);

			/**
			 * Dispose of the exception instance.
			 */
			~StatusCodeException();

			/**
			 * Get the message associated with the exception.
			 * @return error message
			 */
			const char* what() const noexcept;

			/**
			 * Get the status code
			 */
			fiftyoneDegreesStatusCode getCode() const noexcept;

		protected:

			/**
			 * Default constructor.
			 */
			StatusCodeException();

			/** Status code */
			fiftyoneDegreesStatusCode statusCode;

			/** Error message from the status code. */
			string message;
		};

		/**
		 * Fatal exception meaning the process should be halted.
		 */
		class FatalException : public StatusCodeException {
		public:
			/**
			 * New exception from the C exception structure provided.
			 * @param exception to convert to a C++ exception
			 */
			FatalException(fiftyoneDegreesException *exception);
		};

		/**
		 * Exception indicating that the method has not been implemented by the
		 * class. This is the case when abstract methods have not been properly
		 * implemented by their inheriting classes.
		 */
		class NotImplementedException : public runtime_error {
		public:
			/**
			 * Default constructor.
			 */
			NotImplementedException();
		};

		/**
		 * Exception thrown when a property does not exist in the data set.
		 * Usually when attempting to fetch a value from a set of results for a
		 * property which does not exist.
		 */
		class InvalidPropertyException : public runtime_error {
		public:
			/**
			 * Default constructor.
			 */
			InvalidPropertyException();

			/**
			 * Constructor with error message.
			 * @param message the error message
			 */
			InvalidPropertyException(const char *message);
		};

		/**
		 * Exception thrown when there is a problem with the evidence provided.
		 */
		class EvidenceException : public runtime_error {
		public:
			/**
			 * Default constructor.
			 */
			EvidenceException();
		};

		/**
		 * Exception indicating that an attempt was made to return a single
		 * value type (e.g. string or int) when there are multiple values for
		 * the requested property. Instead the values should be fetched as a
		 * vector.
		 */
		class TooManyValuesException : public runtime_error {
		public:
			/**
			 * Default constructor.
			 */
			TooManyValuesException();
		};

		/**
		 * Exception indicating that there were no values in the results for
		 * the requested property.
		 */
		class NoValuesAvailableException : public runtime_error {
		public:
			/**
			 * Default constructor
			 */
			NoValuesAvailableException();

			/**
			 * Constructor with error message.
			 * @param message the error message
			 */
			NoValuesAvailableException(const char *message);
		};
	}
}

#endif