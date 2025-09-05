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

#ifndef FIFTYONE_DEGREES_COLLECTION_CONFIG_HPP
#define FIFTYONE_DEGREES_COLLECTION_CONFIG_HPP

#include "Exceptions.hpp"
#include "collection.h"

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * C++ class wrapper for the #fiftyoneDegreesCollectionConfig structure.
		 * See collection.h.
		 *
		 * Configuration options are set using setter methods and fetched using
		 * corresponding getter methods. The names are self explanatory.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * FiftyoneDegrees::Common::CollectionConfig *config;
		 * 
		 * // Configure a collection with a capacity of 100, with 20 preloaded
		 * // items, which can be used by 4 threads concurrently
		 * config->setCapacity(100);
		 * config->setLoaded(20);
		 * config->setConcurrency(4);
		 * ```
		 */
		class CollectionConfig {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Construct a new instance of CollectionConfig with the default
			 * configuration. This method does not set the internal config
			 * structure, so an extending class must do this if calling this
			 * constructor.
			 */
			CollectionConfig();

			/**
			 * Construct a new instance of CollectionConfig which references an
			 * existing instance of the C structure.
			 * @param config pointer to existing collection configuration
			 * structure
			 */
			CollectionConfig(fiftyoneDegreesCollectionConfig *config);

			/**
			 * @}
			 * @name Setters
			 * @{
			 */

			/**
			 * Set the number of items the cache should store, 0 for no cache.
			 * @param capacity to set
			 */
			void setCapacity(uint32_t capacity);

			/**
			 * Set the expected number of concurrent requests.
			 * @param concurrency to set
			 */
			void setConcurrency(uint16_t concurrency);

			/**
			 * Set the number of items to load into memory from the start of
			 *  the collection.
			 * @param loaded to set
			 */
			void setLoaded(uint32_t loaded);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the number of items the cache should store, 0 for no cache.
			 * @return capacity value
			 */
			uint32_t getCapacity() const;

			/**
			 * Get the expected number of concurrent requests.
			 * @return concurrency value
			 */
			uint16_t getConcurrency() const;

			/**
			 * Get the number of items to load into memory from the start of
			 * the collection.
			 * @return loaded value
			 */
			uint32_t getLoaded() const;

			/**
			 * Get a pointer to the underlying configuration structure.
			 * @return C structure pointer
			 */
			fiftyoneDegreesCollectionConfig* getConfig() const;

			/** 
			 * @}
			 */

		private:
			/** Pointer to the underlying C configuration structure. */
			fiftyoneDegreesCollectionConfig *config;
		};
	}
}

#endif