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

#ifndef FIFTYONE_DEGREES_CONFIG_BASE_HPP
#define FIFTYONE_DEGREES_CONFIG_BASE_HPP

#include <vector>
#include <string>
#include "Exceptions.hpp"
#include "config.h"

using std::vector;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * C++ class wrapper for the #fiftyoneDegreesConfigBase configuration
		 * structure. See config.h.
		 *
		 * Configuration options are set using setter methods and fetched using
		 * corresponding getter methods. The names are self explanatory.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * RequiredPropertiesConfig *properties;
		 *
		 * // Construct a new configuration
		 * ConfigBase *config = new ConfigBase();
		 *
		 * // Configure the engine to create a temporary data file, or reuse
		 * // an existing temporary file if one exists
		 * config->setUseTempFile(true);
		 * config->setReuseTempFile(true);
		 *
		 * // Use the configuration when constructing an engine
		 * EngineBase *engine = new EngineBase(config, properties);
		 * ```
		 */
		class ConfigBase {
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			/**
			 * Constructs a new instance of the configuration with a reference
			 * to the C configuration provided.
			 * @param config pointer to the underlying configuration structure
			 */
			ConfigBase(fiftyoneDegreesConfigBase *config);

			/**
			 * Free any memory associated with temporary directories.
			 */
			virtual ~ConfigBase();
			
			/**
			 * @}
			 * @name Setters
			 * @{
			 */

			/**
			 * Set whether or not the HTTP header field might be prefixed with
			 * 'HTTP_'.
			 * @param use whether or not prefixed upper headers should be used
			 */
			void setUseUpperPrefixHeaders(bool use);

			/**
			 * Set whether or not a temporary file should be created from the
			 * original data file and used to initialise the data set.
			 * @param use should create a temp file
			 */
			void setUseTempFile(bool use);

			/**
			 * Set whether or not a temporary file that already exists for a
			 * master file should be reused by another process.
			 * @param reuse should create a temp files be reused
			 */
			void setReuseTempFile(bool reuse);

			/**
			 * Sets a collection of temporary directories to use if temporary
			 * file operation is required in the order in which the directories
			 * should be used. If no temporary directories are provided and
			 * temporary files should be used the temporary files will be
			 * placed in the same directory as the master file.
			 * @param tempDirs collection of temporary directories.
			 */
			void setTempDirectories(vector<string> tempDirs);
			
			/**
			 * Set whether or not an index to speed up the retrieval of values
			 * for profiles and properties is created.
			 * @param index should create an index
			 */
			void setPropertyValueIndex(bool index);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get whether or not an HTTP_ upper case prefixes should be
			 * considered when evaluating HTTP headers.
			 * @return true if upper case HTTP_ prefixed header keys should be
			 * considered.
			 */
			bool getUseUpperPrefixHeaders() const;

			/**
			 * Get whether or not a temporary file should be created from the
			 * original data file and used to initialise the data set.
			 * @return true if temporary files should be used, otherwise false.
			 */
			bool getUseTempFile() const;

			/**
			 * Get whether temporary files can be reused across multiple
			 * processes.
			 * @return true if temporary files can be reused, otherwise false.
			 */
			bool getReuseTempFile() const;

			/**
			 * Gets a vector of temporary directory strings which should be
			 * used to store temporary files.
			 * @return a vector of temporary directories, or NULL if no
			 * temporary directories are to be used.
			 */
			vector<string> getTempDirectories() const;

			/**
			 * Gets a flag indicating if an index of values for properties and
			 * profile is created.
			 * @return true if an index should be created, or false if not.
			 */
			bool getPropertyValueIndex() const;

			/**
			 * Get the expected number of concurrent accessors of the data set.
			 * @return concurrency
			 */
			virtual uint16_t getConcurrency() const;

			/**
			 * @}
			 */
		private:
			/** Pointer to the underlying C configuration structure. */
			fiftyoneDegreesConfigBase *config;

			/** Paths to directories which should be used when attempting to
			   create temp files. */
			vector<string> tempDirs;

			/**
			 * Frees any memory associated with temporary directories.
			 */
			void freeTempDirectories();

			/**
			 * Maps the strings in the temporary directories vector to string
			 * pointers in the config data structure.
			 */
			void mapTempDirectories();
		};
	}
}

#endif