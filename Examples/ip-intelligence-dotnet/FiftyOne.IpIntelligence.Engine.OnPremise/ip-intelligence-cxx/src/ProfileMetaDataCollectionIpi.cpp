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

#include "ProfileMetaDataCollectionIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"
#include "common-cxx/collectionKeyTypes.h"

using namespace std;
using namespace FiftyoneDegrees::IpIntelligence;

ProfileMetaDataCollectionIpi::ProfileMetaDataCollectionIpi(
	fiftyoneDegreesResourceManager *manager)
	: Collection<uint32_t, ProfileMetaData>() {
	dataSet = DataSetIpiGet(manager);
	if (dataSet == nullptr) {
		throw runtime_error("Data set pointer can not be null");
	}
	profiles = dataSet->profiles;
	profileOffsets = dataSet->profileOffsets;
}

ProfileMetaDataCollectionIpi::~ProfileMetaDataCollectionIpi() {
	DataSetIpiRelease(dataSet);
}

ProfileMetaData* ProfileMetaDataCollectionIpi::getByIndex(
	uint32_t index) const {
	EXCEPTION_CREATE;
	Item item;
	ProfileMetaData *result = nullptr;
	const Profile *profile;
	DataReset(&item.data);
	auto const profileOffsetValue = CollectionGetInteger32(
		profileOffsets, index, exception);
	EXCEPTION_THROW
	const CollectionKey profileKey = {
		(uint32_t)profileOffsetValue,
		CollectionKeyType_Profile,
	};
	profile = (const Profile *)profiles->get(
		profiles,
		&profileKey,
		&item,
		exception);
	EXCEPTION_THROW
	if (profile != nullptr) {
		result = ProfileMetaDataBuilderIpi::build(dataSet, profile);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

ProfileMetaData* ProfileMetaDataCollectionIpi::getByKey(uint32_t key) const {
	EXCEPTION_CREATE;
	Item item;
	ProfileMetaData *result = nullptr;
	Profile *profile;
	DataReset(&item.data);
	profile = ProfileGetByProfileIdIndirect(
		profileOffsets,
		profiles,
		key,
		&item,
		exception);
	EXCEPTION_THROW;
	if (profile != nullptr) {
		result = ProfileMetaDataBuilderIpi::build(dataSet, profile);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

uint32_t ProfileMetaDataCollectionIpi::getSize() const {
	return CollectionGetCount(profileOffsets);
}