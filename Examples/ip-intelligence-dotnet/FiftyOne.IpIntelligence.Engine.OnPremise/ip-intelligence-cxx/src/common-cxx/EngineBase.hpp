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

#ifndef FIFTYONE_DEGREES_ENGINE_BASE_HPP
#define FIFTYONE_DEGREES_ENGINE_BASE_HPP

#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm>
#include "Exceptions.hpp"
#include "RequiredPropertiesConfig.hpp"
#include "Date.hpp"
#include "ConfigBase.hpp"
#include "MetaData.hpp"
#include "EvidenceBase.hpp"
#include "ResultsBase.hpp"
#include "dataset.h"
#include "property.h"
#include "collection.h"


namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Encapsulates the engine class to be extended by engine
		 * implementations. Common logic is contained in this base class to be
		 * used by any extending classes.
		 *
		 * An engine is constructed with a configuration, then used to process
		 * evidence in order to return a set of results. It also exposes
		 * methods to refresh the data using a new data set, and get properties
		 * relating to the data set being used by the engine.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * ConfigBase *config;
		 * RequiredPropertiesConfig *properties;
		 * EvidenceBase *evidence;
		 * 
		 * // Construct the engine
		 * EngineBase *engine = new EngineBase(config, properties);
		 *
		 * // Process some evidence
		 * ResultsBase *results = engine->processBase(evidence);
		 *
		 * // Do something with the results
		 * // ...
		 *
		 * // Delete the results and the engine
		 * delete results;
		 * delete engine;
		 * ```
		 */
		class EngineBase {
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			/**
			 * Construct a new instance of the engine class with the
			 * configuration and properties provided.
			 * @param config used to build the engine
			 * @param properties the properties expected to be fetched
			 * from results
			 */
			EngineBase(
				ConfigBase *config,
				RequiredPropertiesConfig *properties);

			/**
			 * Frees the meta data class and the data set resource created by
			 * the extending class.
			 */
			virtual ~EngineBase();

			/**
			 * @}
			 * @name Engine Methods
			 * @{
			 */

			/**
			 * Processes the evidence provided and returns the result.
			 * @param evidence to process. The keys in getKeys() will be the
			 * only ones considered by the engine.
			 * @return a new results instance with the values for all requested
			 * properties
			 */
			virtual ResultsBase* processBase(EvidenceBase *evidence) const = 0;

			/**
			 * Refresh the data set from the original file location. This
			 * should be implemented by the extending class.
			 */
			virtual void refreshData() const = 0;

			/**
			 * Refresh the data set from the file location provided.
			 * @param fileName of the new data file
			 */
			virtual void refreshData(const char *fileName) const = 0;

			/**
			 * Refresh the data set from the memory location provided.
			 * @param data pointer to the data in memory
			 * @param length length of the data in memory
			 */
			virtual void refreshData(void *data, fiftyoneDegreesFileOffset length) const = 0;

			/**
			 * Refresh the data set from the memory location provided.
			 * @param data pointer to the data in memory
			 * @param length of the data in memory
			 */
			virtual void refreshData(unsigned char data[], fiftyoneDegreesFileOffset length) const = 0;

			/**
			 * @}
			 * @name Setters
			 * @{
			 */

			/**
			 * Sets the license key to be used when updating the data set.
			 * @param licenseKey to set
			 */
			void setLicenseKey(const string &licenseKey);

			/**
			 * Sets the URL to be used when updating the data set.
			 * @param updateUrl to set
			 */
			void setDataUpdateUrl(const string &updateUrl);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Return the a pointer to the meta data class which contains meta
			 * data for the properties, values, profiles and components that
			 * exist within the engine.
			 * @return pointer to the meta data class
			 */
			MetaData* getMetaData() const;

			/**
			 * Gets whether or not automatic updates are enabled. If they are,
			 * then this tells an external service that it should update the
			 * data set.
			 * @return true if updates are enabled
			 */
			bool getAutomaticUpdatesEnabled() const;

			/**
			 * Get the path to the data file the current data set was
			 * initialised from.
			 * @return data file path
			 */
			virtual string getDataFilePath() const = 0;

			/**
			 * Get the path to the temporary data file created by the engine,
			 * or an empty string if one was not created.
			 * @return temp data file path or empty string
			 */
			virtual string getDataFileTempPath() const = 0;

			/**
			 * Get the URL to be used when updating the data file, or an empty
			 * string if this is not set.
			 * @return URL to download new data file from
			 */
			virtual string getDataUpdateUrl() const;

			/**
			 * Get the date at which the current data set was published.
			 * @return data set published date
			 */
			virtual Date getPublishedTime() const = 0;

			/**
			 * Get the date at which a new data file will be available to
			 * download from the URL returned by #getDataUpdateUrl().
			 * @return new data set available date
			 */
			virtual Date getUpdateAvailableTime() const = 0;

			/**
			 * Get the name of the data set being used e.g. Pattern or Hash.
			 * @return product name
			 */
			virtual string getProduct() const = 0;

			/**
			 * Get the tier of the data set being used e.g. Premium or
			 * Enterprise.
			 * @return data set type
			 */
			virtual string getType() const = 0;

			/**
			 * Get the list of keys which the engine accepts as evidence. This
			 * is a pointer to an internal vector should not (and cannot) be
			 * freed.
			 * @return pointer to the list of evidence keys
			 */
			const vector<string>* getKeys() const;

			/**
			 * Get whether or not the engine was compiled with thread-safe
			 * support. If it was not then certain precautions should be taken.
			 * @return true if the engine is thread-safe
			 */
			bool getIsThreadSafe() const;

			/**
			 * @}
			 */
		protected:
			/** Pointer to the configuration used to build the engine. */
			ConfigBase *config;

			/** A shared pointer to the manager is passed around and referenced
			by all instances that hold open a resource handle. This acts as a
			counter to ensure that the pointer to the manager remains valid
			until the last handle is freed. The shared pointer also handles
			freeing the pointer once no references remain. See resource.h for
			more information. */
			shared_ptr<fiftyoneDegreesResourceManager> manager;

			/** Pointer to the properties which are going to be used. */
			RequiredPropertiesConfig *requiredProperties;

			/** Pointer to the meta data relating to the contents of the
			engine's data set. */
			MetaData *metaData;

			/** License key used to update the data set. May be empty. */
			string licenceKey;

			/** URL used to update the data set. */
			string updateUrl;

			/** The default key to use when adding the results to a dictionary. */
			string defaultDataKey;

			/** Keys which should be added to evidence. */
			vector<string> keys;

			/**
			 * Gets a string from a strings collection, and appends to a stream.
			 * @param stream to append the string to
			 * @param strings collection to get the string from
			 * @param offset of the string in the collection
			 * @param storedValueType format of byte array representation.
			 */
			virtual void appendValue(
				stringstream &stream,
				fiftyoneDegreesCollection *strings,
				uint32_t offset,
				fiftyoneDegreesPropertyValueType storedValueType) const;

			/**
			 * Gets a string from a strings collection, and appends to a stream.
			 * @param stream to append the string to
			 * @param strings collection to get the string from
			 * @param offset of the string in the collection
			 */
			virtual void appendString(
				stringstream &stream,
				fiftyoneDegreesCollection *strings,
				uint32_t offset) const;

			/**
			 * Initialise the HTTP header keys which are used by this engine.
			 * These are the pieces of evidence which should be passed in if
			 * available.
			 * @param uniqueHeaders to get the keys from
			 */
			virtual void initHttpHeaderKeys(fiftyoneDegreesHeaders *uniqueHeaders);

			/**
			 * Initialise the override keys which are used by this engine.
			 * These are additional pieces of evidence which should be passed
			 * in if available.
			 * @param overrideProperties to get the keys from
			 */
			void initOverrideKeys(
				fiftyoneDegreesOverridePropertyArray *overrideProperties);

			/**
			 * Adds a key to the list of keys which should be added as evidence.
			 * This is called by the key init methods.
			 * @param key to add
			 */
			void addKey(string key);
		};
	}
}

#endif