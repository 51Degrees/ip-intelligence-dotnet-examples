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

#ifndef FIFTYONE_DEGREES_ENTITY_META_DATA_HPP
#define FIFTYONE_DEGREES_ENTITY_META_DATA_HPP

#include <string>
#include <vector>
#include "data.h"

using std::vector;
using std::string;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Base class for any entity meta data. All meta data classes should
		 * extend this class.
		 *
		 * A meta data instance is immutable once it has been instantiated. An
		 * instance is also comparable to another via the `==` and `!=`
		 * operators, provided the type K is properly implemented.
		 * @tparam K key type for the entity which must be unique e.g. string
		 * is used for property meta data where the unique key is the name of
		 * the property. The type K must implement the `<` and `==` operators.
		 */
		template <class K> class EntityMetaData {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Construct a new instance with the key provided.
			 * @param key of type K
			 */
			EntityMetaData(K key) { this->key = key; }

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the unique key for this entity instance.
			 * @return key of type K
			 */
			K getKey() const { return key; }

			/**
			 * @}
			 * @name Operators
			 * @{
			 */

			/**
			 * Returns true is the two entities share the same unique key.
			 * @param other the other instance to compare
			 * @return true if they are equal
			 */
			const bool operator== (const EntityMetaData<K> other) const {
				return other.key == key;
			}

			/**
			 * Returns true is the two entities do not share the same unique
			 * key.
			 * @param other the other instance to compare
			 * @return true if they are not equal
			 */
			const bool operator!= (const EntityMetaData<K> other) const {
				return !(other.key == key);
			}

			/**
			 * @}
			 */
		private:
			/** The unique key for this instance. */
			K key;
		};
	}
}

#endif