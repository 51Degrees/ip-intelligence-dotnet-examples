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

#ifndef FIFTYONE_DEGREES_ENTITY_META_DATA_BUILDER_HPP
#define FIFTYONE_DEGREES_ENTITY_META_DATA_BUILDER_HPP

#include <string>
#include "Exceptions.hpp"
#include "collection.h"
#include "constants.h"
#include "storedBinaryValue.h"
#include "string.h"
#include "string_pp.hpp"


using namespace FiftyoneDegrees;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Meta data builder class contains static helper methods used when
		 * building meta data instances.
		 */
		class EntityMetaDataBuilder {
		protected:

			/**
			 * @name Static Helpers
			 * @{
			 */

			/**
			 * Get a copy of a string from the strings collection at the offset
			 * provided.
			 * @param stringsCollection pointer to the collection to copy the
			 * string from
			 * @param offset offset in the strings collection of the string to
			 * copy
			 * @return string copy of the string at the offset provided
			 */
			static string getString(
				fiftyoneDegreesCollection *stringsCollection,
				uint32_t offset) {
				return getValue(
					stringsCollection,
					offset,
					FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING); // legacy contract
			}

			/**
			 * Get a copy of a binary value (as string) from the strings collection at the offset
			 * provided.
			 * @param stringsCollection pointer to the collection to copy the
			 * value (as string) from
			 * @param offset offset in the strings collection of the binary value to
			 * copy (as string)
			 * @param storedValueType format of byte array representation.
			 * @return string describing contents of binary value the offset provided
			 */
			static string getValue(
				fiftyoneDegreesCollection *stringsCollection,
				uint32_t offset,
				fiftyoneDegreesPropertyValueType storedValueType) {
				FIFTYONE_DEGREES_EXCEPTION_CREATE;
				string result;
				fiftyoneDegreesCollectionItem item;
				fiftyoneDegreesDataReset(&item.data);
				const fiftyoneDegreesStoredBinaryValue * const binaryValue = fiftyoneDegreesStoredBinaryValueGet(
					stringsCollection,
					offset,
					storedValueType,
					&item,
					exception);
				FIFTYONE_DEGREES_EXCEPTION_THROW;
				if (binaryValue != nullptr) {
					std::stringstream stream;
					writeStoredBinaryValueToStringStream(
						binaryValue,
						storedValueType,
						stream,
						FIFTYONE_DEGREES_MAX_DOUBLE_DECIMAL_PLACES,
						exception);
					FIFTYONE_DEGREES_EXCEPTION_THROW;
					result.append(stream.str());
				}
				FIFTYONE_DEGREES_COLLECTION_RELEASE(
					stringsCollection,
					&item);
				return result;
			}

			/**
			 * @}
			 */
		};
	}
}

#endif