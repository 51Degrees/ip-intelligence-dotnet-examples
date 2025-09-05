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

#ifndef FIFTYONE_DEGREES_COLLECTION_HPP
#define FIFTYONE_DEGREES_COLLECTION_HPP

#include "Exceptions.hpp"
#include "collection.h"


namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * A group of items accessible by index or key.
		 *
		 * A Collection instance hands out new instances of any item requested
		 * so once they are finished with, they must be deleted. A static
		 * object is not returned, as this would require any type V to declare
		 * a default constructor in the case that the caller declares a
		 * variable before calling a get method.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * FiftyoneDegrees::Common::Collection<string, string> *collection;
		 *
		 * // Iterate over all indexes
		 * for (int i = 0; i < collection->getSize(); i++) {
		 *
		 *     // Get the value for the current index
		 *     string *value = collection->getByIndex(i);
		 *
		 *     // Do something with the value
		 *     // ...
		 *
		 *     // Delete the value instance
		 *     delete value;
		 * }
		 *
		 * // Or get a value using it's key of type <T>
		 * string *value = collection->getByKey("string type key");
		 *
		 * // Do something with the value
		 * // ...
		 *
		 * // Delete the value instance
		 * delete value;
		 * ```
		 *
		 * @tparam K key type for the items in the collection which must be
		 * unique e.g. string is used for property meta data where the unique
		 * key is the name of the property. The type K must implement the '<'
		 * and '==' operators
		 * @tparam V value type for the items stored in the collection. These
		 * must be instantiatable by the collection, and should implement a
		 * public destructor as their lifetimes are not handled by the
		 * collection instance
		 */
		template <class K, class V> class Collection {
		public:

			/**
			 * @name Destructor
			 * @{
			 */

			/**
			 * Dispose of any internal data.
			 */
			virtual ~Collection() {};

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the item from the collection at the index provided.
			 * @param index of the item required in the collection. Not the
			 * same as the key
			 * @return a new instance of the item at the index
			 */
			virtual V* getByIndex(uint32_t index) const = 0;

			/**
			 * Get the item for the key from the collection.
			 * @param key unique key of the item required in the collection
			 * @return a new instance of the item with the key
			 */
			virtual V* getByKey(K key) const = 0;

			/**
			* Number of items in the underlying collection.
			* @return the number of items in the collection
			*/
			virtual uint32_t getSize() const = 0;

			/**
			 * @}
			 */
		protected:

			/**
			 * A collection can't be constructed without an inheriting class.
			 */
			Collection() { }
		};
	}
}

#endif
