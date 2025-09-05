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

#ifndef FIFTYONE_DEGREES_CONFIG_IPI_HPP
#define FIFTYONE_DEGREES_CONFIG_IPI_HPP

#include <cstddef>
#include "common-cxx/CollectionConfig.hpp"
#include "common-cxx/ConfigBase.hpp"
#include "ipi.h"


namespace FiftyoneDegrees {
	namespace IpIntelligence {
		using FiftyoneDegrees::Common::CollectionConfig;
		using FiftyoneDegrees::Common::ConfigBase;

		/**
		 * C++ class wrapper for the #fiftyoneDegreesConfigIpi
		 * configuration structure. See ipi.h.
		 *
		 * This extends the ConfigBase class to add IP Intelligence
		 * specific configuration options.
		 *
		 * Configuration options are set using setter methods and fetched
		 * using corresponding getter methods. The names are self
		 * explanatory.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * using namespace FiftyoneDegrees::IpIntelligence;
		 * string dataFilePath;
		 * RequiredPropertiesConfig *properties;
		 *
		 * // Construct a new configuration
		 * ConfigIpi *config = new ConfigIpi();
		 *
		 * // Use the configuration when constructing an engine
		 * EngineIpi *engine = new EngineIpi(
		 *     dataFilePath,
		 *     config,
		 *     properties);
		 * ```
		 */
		class ConfigIpi : public ConfigBase {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Construct a new instance using the default configuration
			 * #fiftyoneDegreesIpiDefaultConfig.
			 */
			ConfigIpi();

			/**
			 * Construct a new instance using the configuration provided.
			 * The values are copied and no reference to the provided
			 * parameter is retained.
			 * @param config pointer to the configuration to copy
			 */
			ConfigIpi(fiftyoneDegreesConfigIpi *config);

			/** 
			 * @}
			 * @name Setters
			 * @{
			 */

			/**
			 * Set the collections to use the high performance
			 * configuration.
			 * See #fiftyoneDegreesIpiHighPerformanceConfig
			 */
			void setHighPerformance();

			/**
			 * Set the collections to use the balanced configuration.
			 * See #fiftyoneDegreesIpiBalancedConfig
			 */
			void setBalanced();

			/**
			 * Set the collections to use the balanced temp configuration.
			 * See #fiftyoneDegreesIpiBalancedTempConfig
			 */
			void setBalancedTemp();

			/**
			 * Set the collections to use the low memory configuration.
			 * See #fiftyoneDegreesIpiLowMemoryConfig
			 */
			void setLowMemory();

			/**
			 * Set the collections to use the entirely in memory
			 * configuration.
			 * See #fiftyoneDegreesIpiInMemoryConfig
			 */
			void setMaxPerformance();

			/**
			 * Set the expected concurrent requests for all the data set's
			 * collections. All collections in the data set which use
			 * cached elements will have their caches constructued to allow
			 * for the concurrency value set here.
			 * See CollectionConfig::setConcurrency
			 * @param concurrency expected concurrent requests
			 */
			void setConcurrency(uint16_t concurrency);


			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the configuration for the strings collection.
			 * @return strings collection configuration
			 */
			const CollectionConfig &getStrings() const;

			/**
			 * Get the configuration for the components collection.
			 * @return components collection configuration
			 */
			const CollectionConfig &getComponents() const;

			/**
			 * Get the configuration for the maps collection.
			 * @return maps collection configuration
			 */
			const CollectionConfig &getMaps() const;
			/**
			 * Get the configuration for the properties collection.
			 * @return properties collection configuration
			 */
			const CollectionConfig &getProperties() const;

			/**
			 * Get the configuration for the values collection.
			 * @return values collection configuration
			 */
			const CollectionConfig &getValues() const;

			/**
			 * Get the configuration for the profiles collection.
			 * @return profiles collection configuration
			 */
			const CollectionConfig &getProfiles() const;

			/**
			 * Get the configuration for the graphs collection.
			 * @return graphs collection configuration
			 */
			const CollectionConfig &getGraphs() const;

			/**
			 * Get the configuration for the profile groups collection.
			 * @return profile groups collection configuration
			 */
			const CollectionConfig &getProfileGroups() const;

			/**
			 * Get the configuration for the profile offsets collection.
			 * @return profile offsets collection configuration
			 */
			const CollectionConfig &getProfileOffsets() const;

			/**
			 * Get the configuration for the property types collection.
			 * @return property types collection configuration
			 */
			const CollectionConfig &getPropertyTypes() const;

			/**
			 * Get the configuration for the graph collection.
			 * @return graph collection configuration
			 */
			const CollectionConfig &getGraph() const;

			/**
			 * Get the lowest concurrency value in the list of possible
			 * concurrencies.
			 * @return a 16 bit integer with the minimum concurrency value.
			 */
			uint16_t getConcurrency() const override;

			 /**
			  * Gets the configuration data structure for use in C code.
			  * Used internally.
			  * @return pointer to the underlying configuration data
			  * structure.
			  */
			fiftyoneDegreesConfigIpi &getConfig();

			/** 
			 * @}
			 */
		private:
			/** The underlying configuration structure */
			fiftyoneDegreesConfigIpi config;

			/** The underlying strings configuration structure */
			CollectionConfig strings;

			/** The underlying components configuration structure */
			CollectionConfig components;

			/** The underlying data set maps configuration structure */
			CollectionConfig maps;

			/** The underlying properties configuration structure */
			CollectionConfig properties;

			/** The underlying values configuration structure */
			CollectionConfig values;

			/** The underlying profiles configuration structure */
			CollectionConfig profiles;

			/** The information about each of the available graphs */
			CollectionConfig graphs;

			/** The underlying profile groups configuration structure */
			CollectionConfig profileGroups;

			/** The underlying profile offsets configuration structure */
			CollectionConfig profileOffsets;

			/** The underlying property types configuration structure */
			CollectionConfig propertyTypes;

			/** The records that form an individual graph */
			CollectionConfig graph;

			/**
			 * Initialise the collection configurations by creating
			 * instances from the IP Intelligence configuration structure.
			 */
			void initCollectionConfig();

			/**
			 * Set the performance profile from an existing configuration.
			 * @param existing pointer to a configuration to copy the
			 * performance profile from
			 */
			void setPerformanceFromExistingConfig(
				const fiftyoneDegreesConfigIpi &existing);
		};
	}
}
#endif	