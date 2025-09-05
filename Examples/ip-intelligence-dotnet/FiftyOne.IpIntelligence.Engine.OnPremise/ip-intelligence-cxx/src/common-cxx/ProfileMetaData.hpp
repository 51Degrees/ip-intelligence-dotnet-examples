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

#ifndef FIFTYONE_DEGREES_PROFILE_META_DATA_HPP
#define FIFTYONE_DEGREES_PROFILE_META_DATA_HPP

#include <string>
#include "EntityMetaData.hpp"

using std::vector;

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * Meta data relating to a profile populated by an engine
		 * implementation.
		 */
		class ProfileMetaData : public EntityMetaData<uint32_t> {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Default constructor, should not be used externally as it
			 * produces an invalid instance
			 */
			ProfileMetaData();

			/**
			 * Construct a new instance of ProfileMetaData from an existing
			 * instance. This copies the existing instance and does not hold a
			 * reference to it.
			 * @param profile to copy
			 */
			ProfileMetaData(ProfileMetaData *profile);

			/**
			 * Create a new instance of ProfileMetaData. This should only be
			 * used internally by the Collection class.
			 * @param profileId the unique id of the profile
			 * @param componentId the unique id of the component the profile
			 * relates to
			 */
			ProfileMetaData(uint32_t profileId, byte componentId);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the unique id of the profile.
			 * @return unique id
			 */
			uint32_t getProfileId() const;

			/**
			 * Get the unique id of the component the profile relates to.
			 * @return unique component id
			 */
			byte getComponentId() const;

			/**
			 * @}
			 */
		private:
			
			/** The unique id of the component which the profile relates to */
			byte componentId;
		};
	}
}

#endif