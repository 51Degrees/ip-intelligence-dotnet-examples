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

#include <iostream>
#include <string>
#include <thread>

#include "../../../src/common-cxx/textfile.h"
#include "../../../src/EngineIpi.hpp"

#define THREAD_COUNT 4

static const char *dataDir = "ip-intelligence-data";

static const char *dataFileName = "51Degrees-LiteV41.ipi";

static const char *ipAddressFileName = "evidence.yml";

using std::thread;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

namespace FiftyoneDegrees {
	namespace Examples {
		/**
		 * C++ IP Intelligence engine examples.
		 */
		namespace IpIntelligence {
			/**
			 * Base class extended by all IP Intelligence examples.
			 */
			class ExampleBase {
			public:
				/**
				 * Construct a new instance of the example to be run using the
				 * data pointer provided.
				 * @param data pointer to the data set in memory
				 * @param length of the data in bytes
				 * @param config to configure the engine with
				 */
				ExampleBase(
					::byte *data,
					fiftyoneDegreesFileOffset length,
					const std::shared_ptr<ConfigIpi> &config);
				
				/**
				 * Construct a new instance of the example to be run using the
				 * data file provided.
				 * @param dataFilePath path to the data file to use
				 */
				ExampleBase(const string& dataFilePath);
				
				/**
				 * Construct a new instance of the example to be run using the
				 * data file and configuration provided.
				 * @param dataFilePath path to the data file to use
				 * @param config to configure the engine with
				 */
				ExampleBase(
					const string& dataFilePath,
					const std::shared_ptr<ConfigIpi> &config);
				
				/**
				 * Dispose of anything created with the example.
				 */
				virtual ~ExampleBase();
				
				/**
				 * Run the example.
				 */
				virtual void run() = 0;
				
				/** Example mobile Ipv4 Address string */
				static const char *ipv4Address;
				
				/** Example desktop Ipv6 Address string */
				static const char *ipv6Address;
				
				/**
				 * Reports the status of the data file initialization.
				 * @param status associated with the initialisation
				 * @param fileName used for initialisation
				 */
				static void reportStatus(fiftyoneDegreesStatusCode status,
										const char *fileName);
			
			protected:
				/**
				 * State containing the states for all threads running in a
				 * multi-threaded example .
				 */
				class SharedState {
				public:
					/**
					 * Construct a new shared state instance.
					 * @param engine pointer to the engine the threads should
					 * use
					 * @param ipAddressFilePath path to the IP Addresses CSV
					 */
					SharedState(EngineIpi *engine, const string &ipAddressFilePath);
				
					/**
					 * Starts threads that run the #processIpAddressesMulti
					 * method.
					 */
					void startThreads();
				
					/**
					 * Joins the threads and frees the memory occupied by the
					 * threads.
					 */
					void joinThreads();
				
					/**
					 * Processes all the IP Addresses in the file named in the
					 * shared state using the engine in the state using a single
					 * thread, and outputs the hash of the results.
					 */
					void processIpAddressesSingle();
				
					/**
					 * Calls the #processIpAddressesSingle method with the state,
					 * then increments the number of threads finished counter.
					 * @param state pointer to a ExampleBase::SharedState to use
					 */
					static void processIpAddressesMulti(void *state);
				
					EngineIpi *engine;            /**< Pointer to the engine */
					volatile long threadsFinished; /**< Number of threads that
																			have finished
													their processing */
					string ipAddressFilePath;      /**< Path to the IP Addresses to
																		process */
					thread threads[THREAD_COUNT];  /**< Pointers to the running
																			threads */
				};
				
				/**
				 * State for a single thread carrying out processing in order
				 * to store a hash of the results.
				 */
				class ThreadState {
				public:
					/**
					 * Construct a new thread state instance.
					 * @param engine pointer to the engine the thread should
					 * use
					 */
					ThreadState(EngineIpi *engine);
					EngineIpi *engine; /**< Pointer to the engine */
					int hashCode;       /**< Running hash code for the processing
													being carried out. This is used to verify the
													work carried out */
				};
				
				/**
				 * Get the hash code for all the values stored in the results
				 * instance.
				 * @param results instance to hash
				 * @return hash code for the results values
				 */
				static unsigned long getHashCode(ResultsIpi *results);
				
				/**
				 * Processes a IP Addresses string and hashes the results, adding
				 * to the hash in the thread state provided.
				 * @param ipAddress the IP Address to hash
				 * @param state pointer to a ExampleBase::ThreadState
				 */
				static void processIpAddress(const char *ipAddress, void *state);
				
				/** Configuration for the Engine */
				std::shared_ptr<ConfigIpi> config;
				/** Properties to initialise the Engine with */
				std::unique_ptr<RequiredPropertiesConfig> properties;
				/** IP Intelligence Engine used for the example */
				std::unique_ptr<EngineIpi> engine;
			
			private:
				/**
				 * Get the hash code for a string of characters.
				 * @param value the string to hash
				 * @return hash code for the string
				 */
				static unsigned long generateHash(unsigned char *value);
			};
		}  // namespace IpIntelligence
	}  // namespace Examples
}  // namespace FiftyoneDegrees
