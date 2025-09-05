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

#ifndef FIFTYONE_DEGREES_EVIDENCE_BASE_HPP
#define FIFTYONE_DEGREES_EVIDENCE_BASE_HPP

#include <string>
#include <map>
#include "Exceptions.hpp"
#include "evidence.h"

using std::map;
using std::make_shared;
using std::invalid_argument;
using std::stringstream;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Base evidence class containing evidence to be processed by an engine.
		 * This wraps a dynamically generated C evidence structure.
		 *
		 * The class extends the map<string, string> template to add a method
		 * of constructing a C evidence structure from the key value pairs.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * EngineBase *engine;
		 *
		 * // Construct a new evidence instance
		 * EvidenceBase *evidence = new EvidenceBase();
		 *
		 * // Add an item of evidence
		 * evidence->operator[]("evidence key") = "evidence value";
		 *
		 * // Give the evidence to an engine for processing
		 * ResultsBase *results = engine->processBase(evidence);
		 *
		 * // Do something with the results (and delete them once finished)
		 * // ...
		 *
		 * // Delete the evidence
		 * delete evidence;
		 * ```
		 */
		class EvidenceBase : public map<string, string> {
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			 /**
			  * Construct a new instance containing no evidence.
			  */
			EvidenceBase();

			/**
			 * Free all the underlying memory containing the evidence.
			 */
			virtual ~EvidenceBase();

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			 /**
			  * Get the underlying C structure containing the evidence. This
			  * only includes evidence which is relevant to the engine. Any
			  * evidence which is irrelevant will not be included in the result.
			  * @return pointer to a populated C evidence structure
			  */
			fiftyoneDegreesEvidenceKeyValuePairArray* get();

			/**
			 * @}
			 * @name Overrides
			 * @{
			 */

			 /**
			  * Clear all evidence items from the instance.
			  */
			void clear();

			/**
			 * Remove the evidence item at the position indicated.
			 * @param position of the item to remove
			 */
			void erase(iterator position);

			/**
			 * Remove the evidence items between the two position indicated.
			 * @param first item to remove
			 * @param last item to remove
			 */
			void erase(iterator first, iterator last);

			/**
			 * @}
			 */
		protected:
			/**
			 * Get whether or not the evidence key prefix is relevant or not.
			 * If the prefix is not relevant or not known then it is of no use
			 * to the engine processing it.
			 * @param prefix extracted from the evidence key
			 * @return true if the key prefix relevant and should be used
			 */
			virtual bool isRelevant(fiftyoneDegreesEvidencePrefix prefix);
		private:
			/** The underlying evidence structure. */
			fiftyoneDegreesEvidenceKeyValuePairArray* evidence;
		};
	}
}

#endif