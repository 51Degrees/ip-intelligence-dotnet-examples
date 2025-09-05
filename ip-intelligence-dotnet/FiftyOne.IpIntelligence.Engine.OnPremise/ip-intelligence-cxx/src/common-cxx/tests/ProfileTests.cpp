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

#include "../fiftyone.h"
#include "../string.h"
#include "../profile.h"
#include "../value.h"

#include "Base.hpp"
#include "limits.h"
#include "StringCollection.hpp"
#include "FixedSizeCollection.hpp"
#include "VariableSizeCollection.hpp"
#include <algorithm>

constexpr int N_PROPERTIES = 15;

//in fact because each our profile has a fixed size and has a fixed number of properties - we can construct
//a fixed width element collection - in the generic case this would not be possible, for some profiles
//may miss values - so the arrays of valueIndices would be of variadic length
#pragma pack(push, 1)
typedef struct {
    fiftyoneDegreesProfile profile;
    uint32_t valueIndices[N_PROPERTIES];
} ProfileContainer;
#pragma pack(pop)

class ProfileTests : public Base {
public:
    ProfileTests();
    virtual ~ProfileTests();
    virtual void SetUp();
    virtual void TearDown();
    void CreateObjects();
    void assessProperty(const fiftyoneDegreesProperty *property, int i);
    void assessValue(const fiftyoneDegreesValue *value, int i);
    
    int valueNameStringIndexFromValueIndex(int i);
    uint32_t profileIdFromProfileIndex(int i);
    uint32_t profileIndexFromProfileId(int id);
    fiftyoneDegreesPropertyAvailableArray *createAvailableProperties(std::vector<std::string> &propertyNames);
    int propertyIndexFromPropertyName(string &propertyName);
    void indicesLookup(std::vector<std::string> &propertyNames);
    void iterateValueIndicesForAvailableProperties(std::vector<std::string> &propertyNames);
    void iterateValueIndicesForEachAvailableProperty(std::vector<std::string> &propertyNames);
 
    fiftyoneDegreesCollectionItem item;
    
    StringCollection *stringsCollectionHelper;
    FixedSizeCollection<fiftyoneDegreesProperty> *propertiesCollectionHelper;
    VariableSizeCollection<ProfileContainer> *profilesCollectionHelper;
    FixedSizeCollection<fiftyoneDegreesProfileOffset> *profileOffsetsCollectionHelper;
    FixedSizeCollection<fiftyoneDegreesValue> *valuesCollectionHelper;
    
    fiftyoneDegreesCollection *stringsCollection;
    fiftyoneDegreesCollection *propertiesCollection;
    fiftyoneDegreesCollection *profilesCollection;
    fiftyoneDegreesCollection *valuesCollection;
    fiftyoneDegreesCollection *profileOffsetsCollection;
    
    
    static constexpr int N_PER_PROPERTY = 6;
    static constexpr int N_PROFILES = 5;
    static const char *strings[N_PROPERTIES * (N_PER_PROPERTY + 1)];
    static const char *profileValueNames[][N_PROPERTIES];
};

//NOTE: each property name is in the beginning of the line, followed by values
//NOTE: both properties and values within a property must be alphabetically sorted 
// for the binary search is able to search within the values collection between the property first and last values

const char *ProfileTests::strings[] = {
    "Brightness",   "Bright", "Dim", "Dull", "Glowing", "Shiny", //0
    "Color",        "Black", "Blue", "Green", "Red", "Yellow",   //6
    "Condition",    "Broken", "New", "Old", "Pristine", "Worn",  //12
    "Flexibility",  "Bendable", "Flexible", "Pliable", "Rigid", "Stiff", //18
    "Material",     "Fabric", "Glass", "Metal", "Plastic", "Wood", //24
    "Opacity",      "Opaque", "Semi-opaque", "Semi-transparent", "Translucent", "Transparent", //30
    "Pattern",      "Checkered", "Dotted", "Floral", "Plain", "Striped", //36
    "Position",     "Diagonal", "Horizontal", "Tilted", "Upside-down", "Vertical", //42
    "Shape",        "Oval", "Rectangular", "Round", "Square", "Triangular",  //48
    "Size",         "Enormous", "Large", "Medium", "Small", "Tiny",  //54
    "Taste",        "Bitter", "Salty", "Savory", "Sour", "Sweet", //60
    "Temperature",  "Cold", "Cool", "Freezing", "Hot", "Warm",  //66
    "Texture",      "Hard", "Rough", "Smooth", "Soft", "Sticky", //72
    "Volume",       "Loud", "Moderate", "Mute", "Quiet", "Silent", //78
    "Weight",       "Dense", "Heavy", "Light", "Lightweight", "Moderate" //84
};

//each profile is actually a set of indices of the property values in the valuesCollection array,
//but to make it more human readable - we compute those indices from value names
//for each profile value indices need to be
//strictly increasing for the binary search to work, this is achieved, by the fact that
//properties are sorted alphabetically and index of each value increases in the valuesCollection

const char *ProfileTests::profileValueNames[][N_PROPERTIES] = {
    {
        "Bright", "Black", "Broken", "Bendable", "Fabric", "Opaque",
        "Checkered", "Diagonal", "Oval", "Enormous", "Bitter",
        "Cold", "Hard", "Loud", "Dense"
    },
    {
        "Dim", "Blue", "New", "Flexible", "Glass", "Semi-opaque",
        "Dotted", "Horizontal", "Rectangular", "Large", "Salty",
        "Cool", "Rough", "Moderate", "Heavy"
    },
    {
        "Dull", "Green", "Old", "Pliable", "Metal", "Semi-transparent",
        "Floral", "Tilted", "Round", "Medium", "Savory",
        "Freezing", "Smooth", "Mute", "Light"
    },
    {
        "Glowing", "Red", "Pristine", "Rigid", "Plastic", "Translucent",
        "Plain", "Upside-down", "Square", "Small", "Sour",
        "Hot", "Soft", "Quiet", "Lightweight"
    },
    {
        "Shiny", "Yellow", "Worn", "Stiff", "Wood", "Transparent",
        "Striped", "Vertical", "Triangular", "Tiny", "Sweet",
        "Warm", "Sticky", "Silent", "Moderate"
    }
};

ProfileTests::ProfileTests() {
    CreateObjects();
}

ProfileTests::~ProfileTests() {
    delete propertiesCollectionHelper;
    delete valuesCollectionHelper;
    delete stringsCollectionHelper;
    delete profilesCollectionHelper;
    delete profileOffsetsCollectionHelper;
}

void ProfileTests::CreateObjects() {
    stringsCollectionHelper = new StringCollection(strings, sizeof(strings)/sizeof(strings[0]));
    stringsCollection = stringsCollectionHelper->getState()->collection;
    uint32_t *offsets = stringsCollectionHelper->getState()->offsets;
    
    std::vector<fiftyoneDegreesValue> values;
    for (int i=0;i<N_PROPERTIES;i++) {
        for (int j=1;j<N_PER_PROPERTY;j++) {
            int valueNameIdx = i * N_PER_PROPERTY + j;
            values.push_back({
                (int16_t) i, // propertyIndex
                (int32_t) offsets[valueNameIdx], // nameOffset
                (int32_t) offsets[valueNameIdx], // descriptionOffset - pointing to the same location as name for now
                (int32_t) offsets[valueNameIdx], // urlOffset - pointing to the same location as name for now
            });
        }
    }
    
    valuesCollectionHelper = new FixedSizeCollection<fiftyoneDegreesValue>(values);
    valuesCollection = valuesCollectionHelper->getState()->collection;
    
    std::vector<fiftyoneDegreesProperty> tempProperties;
    for (byte i=0;i<N_PROPERTIES;++i) {
        int propIdx = i * N_PER_PROPERTY;
        uint32_t valueIdx = i * (N_PER_PROPERTY - 1); //there are 1 less values than properties in the valuesCollection
        tempProperties.push_back({
            0, // componentIndex
            i, // displayOrder
            1, // isMandatory
            0, // isList
            1, // showValues
            0, // isObsolete
            1, // show
            FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING, // valueType
            valueIdx, // defaultValueIndex;
            offsets[propIdx + 0], // nameOffset
            offsets[propIdx + 1], // descriptionOffset
            offsets[propIdx + 2], // categoryOffset
            offsets[propIdx + 3], // urlOffset
            valueIdx , // firstValueIndex
            valueIdx + N_PER_PROPERTY - 2, // lastValueIndex
            4, // mapCount
            0 // firstMapIndex
        });
    }
    propertiesCollectionHelper = new FixedSizeCollection<fiftyoneDegreesProperty>(tempProperties);
    propertiesCollection = propertiesCollectionHelper->getState()->collection;
    
    std::vector<ProfileContainer> tempProfiles;
    for (int i=0; i<N_PROFILES;++i) {
        ProfileContainer cont = {{
            0, //componentIndex
            profileIdFromProfileIndex(i), //profileId
            N_PROPERTIES
        }, {}} ;
        for (int j=0;j<N_PROPERTIES;++j) {
            EXCEPTION_CREATE
            fiftyoneDegreesProperty *property = fiftyoneDegreesPropertyGet(propertiesCollection, j, &item, exception);
            COLLECTION_RELEASE(item.collection, &item);
            uint32_t valueIndex = (uint32_t) fiftyoneDegreesValueGetIndexByName(valuesCollection, stringsCollection, property, profileValueNames[i][j], exception);
            EXCEPTION_THROW
            cont.valueIndices[j] = valueIndex;
        }
        tempProfiles.push_back(cont);
    }
    profilesCollectionHelper = new VariableSizeCollection<ProfileContainer>(tempProfiles);
    profilesCollection = profilesCollectionHelper->getState()->collection;
    uint32_t *profileOffsets = profilesCollectionHelper->getState()->offsets;
    
    std::vector<fiftyoneDegreesProfileOffset> tempProfileOffsets;
    for (int i=0;i<N_PROFILES;++i) {
        tempProfileOffsets.push_back({
            tempProfiles[i].profile.profileId,
            profileOffsets[i]
        });
    }

    profileOffsetsCollectionHelper = new FixedSizeCollection<fiftyoneDegreesProfileOffset>(tempProfileOffsets);
    profileOffsetsCollection = profileOffsetsCollectionHelper->getState()->collection;
}

void ProfileTests::SetUp() {
    Base::SetUp();
}

void ProfileTests::TearDown() {
    Base::TearDown();
}

// projects value collection index onto the strings array index
int ProfileTests::valueNameStringIndexFromValueIndex(int i) {
    int propertyIdx = i / (N_PER_PROPERTY - 1);
    int propertyNameIdx = propertyIdx * N_PER_PROPERTY;
    int valueNameIdx = propertyNameIdx + i % (N_PER_PROPERTY - 1) + 1;
    return valueNameIdx;
}

// compute unique profile Id from profile index in the collection
uint32_t ProfileTests::profileIdFromProfileIndex(int i) {
    return 101 * i + 1;
}

uint32_t ProfileTests::profileIndexFromProfileId(int id) {
    return (id - 1) / 101;
}

void ProfileTests::assessValue(const fiftyoneDegreesValue *value, int i) {
    EXCEPTION_CREATE
    int valueNameIdx = valueNameStringIndexFromValueIndex(i);
    
    EXPECT_STREQ(&fiftyoneDegreesValueGetName(stringsCollection, value, &item, exception)->value, strings[valueNameIdx]);
    COLLECTION_RELEASE(item.collection, &item);

    EXPECT_STREQ(&fiftyoneDegreesValueGetDescription(stringsCollection, value, &item, exception)->value, strings[valueNameIdx]);
    COLLECTION_RELEASE(item.collection, &item);
    
    EXPECT_STREQ(&fiftyoneDegreesValueGetUrl(stringsCollection, value, &item, exception)->value, strings[valueNameIdx]);
    COLLECTION_RELEASE(item.collection, &item);
    
    
}

TEST_F(ProfileTests, Values) {
    for (int i=0;i<(N_PER_PROPERTY-1) * N_PROPERTIES;i++) {
        EXCEPTION_CREATE

        const fiftyoneDegreesValue *value = fiftyoneDegreesValueGet(valuesCollection, i, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        assessValue(value, i);
        
        int propertyIdx = i / (N_PER_PROPERTY - 1);
        int valueNameIdx = valueNameStringIndexFromValueIndex(i);

        fiftyoneDegreesProperty *property = fiftyoneDegreesPropertyGet(propertiesCollection, propertyIdx, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        value = fiftyoneDegreesValueGetByName(valuesCollection, stringsCollection, property, strings[valueNameIdx], &item, exception);
        COLLECTION_RELEASE(item.collection, &item);

        assessValue(value, i);
    
        EXPECT_EQ(i, fiftyoneDegreesValueGetIndexByName(valuesCollection, stringsCollection, property, strings[valueNameIdx], exception));
        COLLECTION_RELEASE(item.collection, &item);

        EXPECT_EQ(-1, fiftyoneDegreesValueGetIndexByName(valuesCollection, stringsCollection, property, "NonExistantName", exception));
        COLLECTION_RELEASE(item.collection, &item);
    }
}

TEST_F(ProfileTests, ProfileGetBy) {
    EXCEPTION_CREATE
    for (int i=0; i<N_PROFILES;++i) {
        uint32_t id = profileIdFromProfileIndex(i);
        fiftyoneDegreesProfile *profile = fiftyoneDegreesProfileGetByProfileId(profileOffsetsCollection, profilesCollection, id, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        EXPECT_EQ(profile->profileId, id);
        
        profile = fiftyoneDegreesProfileGetByIndex(profileOffsetsCollection, profilesCollection, i, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        EXPECT_EQ(profile->profileId, id);
    }
}

bool iterateValuesCallback(void *state, fiftyoneDegreesCollectionItem *item) {
    std::vector<fiftyoneDegreesValue *> *values = (std::vector<fiftyoneDegreesValue *> *)state;
    values->push_back((fiftyoneDegreesValue *)item->data.ptr);
    return true;
}

bool iterateValueIndices(void *state, uint32_t valueIndex) {
    std::vector<uint32_t> *indices = (std::vector<uint32_t> *)state;
    indices->push_back(valueIndex);
    return true;
}

TEST_F(ProfileTests, ProfileIterateValues) {
    EXCEPTION_CREATE
    for (int i=0;i<N_PROFILES;++i) {
        fiftyoneDegreesProfile *profile = fiftyoneDegreesProfileGetByIndex(profileOffsetsCollection, profilesCollection, i, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        for (int j=0;j<N_PROPERTIES;++j) {
            fiftyoneDegreesProperty *property = fiftyoneDegreesPropertyGet(propertiesCollection, j, &item, exception);
            COLLECTION_RELEASE(item.collection, &item);
            std::vector<fiftyoneDegreesValue *> values;
            fiftyoneDegreesProfileIterateValuesForProperty(valuesCollection, profile, property, &values, iterateValuesCallback, exception);
            EXPECT_GT(values.size(), 0);
            for(auto v: values) {
                EXPECT_EQ(v->propertyIndex, j);
            }
        }
    }
    EXPECT_TRUE(EXCEPTION_OKAY);
}

int ProfileTests::propertyIndexFromPropertyName(std::string &propertyName) {
    EXCEPTION_CREATE
    for (int i=0;i<N_PROPERTIES;++i) {
        const fiftyoneDegreesProperty *property = fiftyoneDegreesPropertyGet(propertiesCollection, i, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        const fiftyoneDegreesString *name = fiftyoneDegreesPropertyGetName(stringsCollection, property, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        if (0 == strcmp(propertyName.c_str(), &name->value)) {
            return i;
        }
    }
    return -1;
}

fiftyoneDegreesPropertyAvailableArray *ProfileTests::createAvailableProperties(std::vector<std::string> &propertyNames) {
    //random set of 5 available properties
    fiftyoneDegreesPropertyAvailableArray * FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesPropertyAvailable, propertiesAvailable, N_PROPERTIES);
    EXCEPTION_CREATE
    for (size_t j=0;j<propertyNames.size();++j) {
        string &propertyName = propertyNames[j];
        int propIdx = propertyIndexFromPropertyName(propertyName);
        getStringValue(stringsCollectionHelper->getState(), N_PER_PROPERTY * propIdx, &item);
        COLLECTION_RELEASE(item.collection, &item);
        propertiesAvailable->items[j].propertyIndex = propIdx;
        propertiesAvailable->items[j].name = item;
        propertiesAvailable->items[j].evidenceProperties = NULL;
        propertiesAvailable->items[j].delayExecution = false;
        propertiesAvailable->count++;
    }
    return propertiesAvailable;
}

void ProfileTests::iterateValueIndicesForAvailableProperties(std::vector<std::string> &propertyNames) {
    fiftyoneDegreesPropertiesAvailable *propertiesAvailable = createAvailableProperties(propertyNames);
    
    for (int i=0;i<N_PROFILES;++i) {
        EXCEPTION_CREATE
        fiftyoneDegreesProfile *profile = fiftyoneDegreesProfileGetByIndex(profileOffsetsCollection, profilesCollection, i, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        EXPECT_TRUE(EXCEPTION_OKAY);
        
        std::vector<uint32_t> valueIndices;
        EXCEPTION_CLEAR
        fiftyoneDegreesProfileIterateValueIndexes(profile, propertiesAvailable, valuesCollection, &valueIndices, iterateValueIndices, exception);
        EXPECT_EQ(valueIndices.size(), propertiesAvailable->count);
        for (auto index: valueIndices) {
            const fiftyoneDegreesValue *value = fiftyoneDegreesValueGet(valuesCollection, index, &item, exception);
            COLLECTION_RELEASE(item.collection, &item);
            const fiftyoneDegreesString *s = fiftyoneDegreesStringGet(stringsCollection, value->nameOffset, &item, exception);
            COLLECTION_RELEASE(item.collection, &item);
            const char **found = std::find_if(&profileValueNames[i][0], &profileValueNames[i][0] + N_PROPERTIES, [s](const char *p){ return 0 == strcmp(&s->value, p); });
            EXPECT_NE(&profileValueNames[i][0] + N_PROPERTIES, found);  //belongs to current profile
        }
        EXPECT_TRUE(EXCEPTION_OKAY);
    }
    
    fiftyoneDegreesFree(propertiesAvailable);
}

TEST_F(ProfileTests, ProfileIterateValueIndices) {
    std::vector<std::string> propertyNamesNonRepetitive {"Material","Position","Opacity","Shape","Weight"};
    std::vector<std::string> propertyNamesRepetitive {"Material","Material","Opacity","Shape","Weight"};
    iterateValueIndicesForAvailableProperties(propertyNamesNonRepetitive);
    //repetitive are not supported:
    //iterateValueIndicesForAvailableProperties(propertyNamesRepetitive);
}

bool iterateProfiles(void *state, fiftyoneDegreesCollectionItem *item) {
    std::vector<fiftyoneDegreesProfile *> *profiles = (std::vector<fiftyoneDegreesProfile *> *)state;
    profiles->push_back((fiftyoneDegreesProfile *) item->data.ptr);
    return true;
}

TEST_F(ProfileTests, profileIterateForPropertyAndValue) {
    std::vector<fiftyoneDegreesProfile *> profiles;
    EXCEPTION_CREATE
    fiftyoneDegreesProfileIterateProfilesForPropertyAndValue(stringsCollection, propertiesCollection, valuesCollection, profilesCollection, profileOffsetsCollection, "Size", "Enormous", &profiles, iterateProfiles, exception);
    EXPECT_EQ(profileIndexFromProfileId(profiles[0]->profileId), 0);
    EXPECT_TRUE(EXCEPTION_OKAY);
    
    profiles.clear();
    fiftyoneDegreesProfileIterateProfilesForPropertyAndValue(stringsCollection, propertiesCollection, valuesCollection, profilesCollection, profileOffsetsCollection, "Position", "Horizontal", &profiles, iterateProfiles, exception);
    EXPECT_EQ(profileIndexFromProfileId(profiles[0]->profileId), 1);
    
    profiles.clear();
    fiftyoneDegreesProfileIterateProfilesForPropertyAndValue(stringsCollection, propertiesCollection, valuesCollection, profilesCollection, profileOffsetsCollection, "Material", "Metal", &profiles, iterateProfiles, exception);
    EXPECT_EQ(profileIndexFromProfileId(profiles[0]->profileId), 2);
    
    profiles.clear();
    fiftyoneDegreesProfileIterateProfilesForPropertyAndValue(stringsCollection, propertiesCollection, valuesCollection, profilesCollection, profileOffsetsCollection, "Brightness", "Glowing", &profiles, iterateProfiles, exception);
    EXPECT_EQ(profileIndexFromProfileId(profiles[0]->profileId), 3);
    
    profiles.clear();
    fiftyoneDegreesProfileIterateProfilesForPropertyAndValue(stringsCollection, propertiesCollection, valuesCollection, profilesCollection, profileOffsetsCollection, "Weight", "Moderate", &profiles, iterateProfiles, exception);
    EXPECT_EQ(profileIndexFromProfileId(profiles[0]->profileId), 4);
}

void ProfileTests::indicesLookup(std::vector<std::string> &propertyNames) {
    EXCEPTION_CREATE
    fiftyoneDegreesPropertiesAvailable *availableProperties = createAvailableProperties(propertyNames);
    fiftyoneDegreesIndicesPropertyProfile *index = fiftyoneDegreesIndicesPropertyProfileCreate(profilesCollection, profileOffsetsCollection, availableProperties, valuesCollection, exception);
    
    for (int i=0;i<N_PROFILES;++i) {
        uint32_t profileId = profileIdFromProfileIndex(i);
        fiftyoneDegreesProfile *profile = fiftyoneDegreesProfileGetByProfileId(profileOffsetsCollection, profilesCollection, profileId, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        for (uint32_t j=0;j<availableProperties->count;j++) {
            fiftyoneDegreesPropertyAvailable property = availableProperties->items[j];
            //lookup property value Index within the profile with profileId for the available property indexed at j in the array of availableProperties
            uint32_t valueIdxIdxWithinProfile = fiftyoneDegreesIndicesPropertyProfileLookup(index, profileId, j);
            //convert valueIdxIdxWithinProfile to the valueIndex in the actual values collection by looking it up with the profile collection
            uint32_t *first = (uint32_t*)(profile + 1);
            uint32_t valueIdx = *(first + valueIdxIdxWithinProfile);
            //now retrieve value and check that it belongs to the property above
            const fiftyoneDegreesValue *value = fiftyoneDegreesValueGet(valuesCollection, valueIdx, &item, exception);
            COLLECTION_RELEASE(item.collection, &item);
            EXPECT_EQ(value->propertyIndex, property.propertyIndex);
        }
    }

    fiftyoneDegreesIndicesPropertyProfileFree(index);
    fiftyoneDegreesFree(availableProperties);
}

TEST_F(ProfileTests, indicesLookup) {
    std::vector<std::string> propertyNamesNonRepetitive {"Volume","Position","Texture","Flexibility","Weight", "Brightness"};
    std::vector<std::string> propertyNamesRepetitive {"Material","Position","Shape","Shape","Weight"};
    
    indicesLookup(propertyNamesNonRepetitive);
    //repetitive are not supported:
    //indicesLookup(propertyNamesRepetitive);
}

bool collectValues(void *state, fiftyoneDegreesCollectionItem *item) {
    std::vector<fiftyoneDegreesValue *> *values = (std::vector<fiftyoneDegreesValue *> *)state;
    values->push_back((fiftyoneDegreesValue *)item->data.ptr);
    return true;
}

void ProfileTests::iterateValueIndicesForEachAvailableProperty(std::vector<std::string> &propertyNames) {
    EXCEPTION_CREATE
    fiftyoneDegreesPropertiesAvailable *availableProperties = createAvailableProperties(propertyNames);
    fiftyoneDegreesIndicesPropertyProfile *index = fiftyoneDegreesIndicesPropertyProfileCreate(profilesCollection, profileOffsetsCollection, availableProperties, valuesCollection, exception);

    for (int i=0;i<N_PROFILES;++i) {
        fiftyoneDegreesProfile *profile = fiftyoneDegreesProfileGetByIndex(profileOffsetsCollection, profilesCollection, i, &item, exception);
        COLLECTION_RELEASE(item.collection, &item);
        
        for (size_t j=0;j<propertyNames.size();++j) {
            const fiftyoneDegreesProperty *property = fiftyoneDegreesPropertyGetByName(propertiesCollection, stringsCollection, propertyNames[j].c_str(), &item, exception);
            COLLECTION_RELEASE(item.collection, &item);
            
            std::vector<fiftyoneDegreesValue *> values;
            fiftyoneDegreesProfileIterateValuesForPropertyWithIndex(valuesCollection, index, (uint32_t) j, profile, property, &values, collectValues, exception);
            EXPECT_EQ(values.size(), 1);
            const fiftyoneDegreesValue *expectedValue = NULL;
            const uint32_t *first = (uint32_t *) (profile + 1);
            for (uint32_t k=0;k<profile->valueCount;++k) {
                uint32_t valueIdx = *(first + k);
                const fiftyoneDegreesValue *candidate = fiftyoneDegreesValueGet(valuesCollection, valueIdx, &item, exception);
                COLLECTION_RELEASE(item.collection, &item);
                if ((uint32_t) candidate->propertyIndex == availableProperties->items[j].propertyIndex) {
                    expectedValue = candidate;
                    break;
                }
            }
            EXPECT_NE(expectedValue, (fiftyoneDegreesValue *) NULL);
            EXPECT_EQ(values[0], expectedValue);
            EXPECT_EQ(values[0]->propertyIndex, expectedValue->propertyIndex);
            EXPECT_TRUE(EXCEPTION_OKAY);
        }
    }

    fiftyoneDegreesIndicesPropertyProfileFree(index);
    fiftyoneDegreesFree(availableProperties);
}

TEST_F(ProfileTests, iterateValueForPropertyWithIndex) {
    std::vector<std::string> propertyNamesNonRepetitive {"Taste","Position","Material","Size","Color", "Temperature"};
    iterateValueIndicesForEachAvailableProperty(propertyNamesNonRepetitive);
}

TEST_F(ProfileTests, unhappyPaths) {
    //NULL profileId
    uint32_t profileOffset;
    EXCEPTION_CREATE
    uint32_t *profileOffsetPtr = fiftyoneDegreesProfileGetOffsetForProfileId(profileOffsetsCollection, 0, &profileOffset, exception);
    EXPECT_FALSE(EXCEPTION_OKAY);
    EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_PROFILE_EMPTY));

    // non-existent profileId
    EXCEPTION_CLEAR
    profileOffsetPtr = fiftyoneDegreesProfileGetOffsetForProfileId(profileOffsetsCollection, 20003, &profileOffset, exception);
    EXPECT_EQ(profileOffsetPtr, (uint32_t *) NULL);
}
