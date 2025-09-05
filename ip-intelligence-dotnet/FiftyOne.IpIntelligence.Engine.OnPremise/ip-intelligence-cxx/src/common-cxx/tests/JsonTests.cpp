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

#include "Base.hpp"
#include "StringCollection.hpp"
#include "FixedSizeCollection.hpp"
#include "../fiftyone.h"

class JsonTests : public Base {
public:
    JsonTests();
    virtual ~JsonTests();
    void CreateObjects();
    
    StringCollection *stringsCollectionHelper;
    FixedSizeCollection<fiftyoneDegreesProperty> *propertiesCollectionHelper;
    FixedSizeCollection<fiftyoneDegreesValue> *valuesCollectionHelper;
    fiftyoneDegreesCollection *stringsCollection;
    fiftyoneDegreesCollection *propertiesCollection;
    fiftyoneDegreesCollection *valuesCollection;
    
    fiftyoneDegreesCollectionItem item;

    static constexpr size_t BUFFER_SIZE = 512;
    static constexpr int N_PROPERTIES = 15;
    static constexpr int N_PER_PROPERTY = 6;
    static const char *strings[N_PROPERTIES * (N_PER_PROPERTY + 1)];
};

const char *JsonTests::strings[] = {
    "Brightness",   "Bright", "Dim", "Dull", "Glowing", "Shiny", //0
    "Color",        "Black", "Blue", "Green", "Red", "Yellow",   //6
    "Condition",    "\"Bro\\ken\"", "New", "Old", "Pristine", "Worn",  //12
    "Flexibility",  "\t\r\f\bBendable\n", "Flexible", "Pliable", "Rigid", "Stiff", //18 - some special characters
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

JsonTests::JsonTests() {
    CreateObjects();
}

JsonTests::~JsonTests() {
    delete propertiesCollectionHelper;
    delete valuesCollectionHelper;
    delete stringsCollectionHelper;
}

void JsonTests::CreateObjects() {
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
        byte isList = i == 1 ? 1 : 0;
        tempProperties.push_back({
            0, // componentIndex
            i, // displayOrder
            1, // isMandatory
            isList, // isList - make the 1st property (after 0th) a list
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
}

TEST_F(JsonTests, basicJsonForming) {
    FIFTYONE_DEGREES_EXCEPTION_CREATE;
    fiftyoneDegreesStringBuilder builder {(char * const)fiftyoneDegreesMalloc(BUFFER_SIZE), BUFFER_SIZE};
    
    fiftyoneDegreesJson json {
        builder, /**< Output buffer */
        stringsCollection, /**< Collection of strings */
        NULL, /**< The property being added */
        NULL, /**< The values for the property */
        exception, /**< Exception */
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING, /**< Stored property type */
    };
    
    fiftyoneDegreesJsonDocumentStart(&json);
        for (uint32_t propIdx = 0; propIdx < 4; ++propIdx) {
            fiftyoneDegreesProperty *property =  fiftyoneDegreesPropertyGet(propertiesCollection, propIdx, &item, exception);
            fiftyoneDegreesList valuesList;
            fiftyoneDegreesListInit(&valuesList, 4);
            
            fiftyoneDegreesCollectionItem valueItem;
            fiftyoneDegreesCollectionItem valueItem2;
            
            const fiftyoneDegreesValue *value = fiftyoneDegreesValueGet(valuesCollection, propIdx * (N_PER_PROPERTY - 1), &valueItem, exception);
            fiftyoneDegreesValueGetName(stringsCollection, value, &valueItem, exception);
            fiftyoneDegreesListAdd(&valuesList, &valueItem);
            
            if (property->isList) {
                const fiftyoneDegreesValue *value2 = fiftyoneDegreesValueGet(valuesCollection, propIdx * (N_PER_PROPERTY - 1) + 1, &valueItem2, exception);
                fiftyoneDegreesValueGetName(stringsCollection, value2, &valueItem2, exception);
                fiftyoneDegreesListAdd(&valuesList, &valueItem2);
            }
            
            json.property = property;
            json.values = &valuesList;
            
            if (propIdx > 0) {
                fiftyoneDegreesJsonPropertySeparator(&json);
            }
            
            fiftyoneDegreesJsonPropertyStart(&json);
                fiftyoneDegreesJsonPropertyValues(&json);
            fiftyoneDegreesJsonPropertyEnd(&json);
            fiftyoneDegreesListFree(&valuesList);
        }

    fiftyoneDegreesJsonDocumentEnd(&json);

    EXPECT_STREQ(json.builder.ptr, "{\"Brightness\":\"Bright\",\"Color\":[\"Black\",\"Blue\"],\"Condition\":\"\\\"Bro\\ken\\\"\",\"Flexibility\":\"\\t\\r\\f\\bBendable\\n\"}");

    fiftyoneDegreesFree(builder.ptr);
}

TEST_F(JsonTests, unhappyPaths) {
    FIFTYONE_DEGREES_EXCEPTION_CREATE;
    fiftyoneDegreesStringBuilder builder {(char * const)fiftyoneDegreesMalloc(BUFFER_SIZE), BUFFER_SIZE};
    
    fiftyoneDegreesJson json {
        builder, /**< Output buffer */
        stringsCollection, /**< Collection of strings */
        NULL, /**< The property being added */
        NULL, /**< The values for the property */
        exception, /**< Exception */
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING, /**< Stored property type */
    };
    {
        EXCEPTION_CLEAR;
        fiftyoneDegreesJsonDocumentStart(&json);
        EXPECT_TRUE(EXCEPTION_OKAY);
        
        EXCEPTION_CLEAR;
        fiftyoneDegreesJsonPropertyStart(&json);
        EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_NULL_POINTER));
            
        EXCEPTION_CLEAR;
        fiftyoneDegreesJsonPropertyValues(&json);
        EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_NULL_POINTER));
    
        EXCEPTION_CLEAR;
        fiftyoneDegreesJsonPropertyEnd(&json);
        EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_NULL_POINTER));
        
        EXCEPTION_CLEAR;
        fiftyoneDegreesJsonDocumentEnd(&json);
        EXPECT_TRUE(EXCEPTION_OKAY);
        
        EXPECT_STREQ(builder.ptr, "{}");
    }
    
    fiftyoneDegreesFree(builder.ptr);
}
