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
#include "Base.hpp"
#include "limits.h"
#include "StringCollection.hpp"
#include "FixedSizeCollection.hpp"

class PropertyTests : public Base {
public:
    virtual void SetUp();
    virtual void TearDown();
    void CreateObjects();
    void assessProperty(const fiftyoneDegreesProperty *property, int i);
    
    fiftyoneDegreesProperty *property;
    StringCollection *stringsCollectionHelper;
    fiftyoneDegreesCollection *stringsCollection;
    
    fiftyoneDegreesCollectionItem item;
    
    FixedSizeCollection<fiftyoneDegreesProperty> *propertiesCollectionHelper;
    fiftyoneDegreesCollection *propertiesCollection;
    
    static constexpr int N_PROPERTIES = 3;
    static constexpr int N_PER_PROPERTY = 5;
    static const char *strings[N_PROPERTIES * N_PER_PROPERTY];
    static const fiftyoneDegreesPropertyValueType types[N_PROPERTIES];

};

const fiftyoneDegreesPropertyValueType PropertyTests::types[N_PROPERTIES] = {
   FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_JAVASCRIPT,
   FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
   FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DOUBLE,
};

const char *PropertyTests::strings[] = {
    "prop1", "name", "descr", "cat", "url",
    "prop2", "name2", "descr2", "cat2", "url2",
    "prop3", "name3", "descr3", "cat3", "url3"
};

void PropertyTests::CreateObjects() {
    stringsCollectionHelper = new StringCollection(strings, sizeof(strings)/sizeof(strings[0]));
    stringsCollection = stringsCollectionHelper->getState()->collection;
    uint32_t *offsets = stringsCollectionHelper->getState()->offsets;
    std::vector<fiftyoneDegreesProperty> tempProperties;
    for (byte i=0;i<N_PROPERTIES;++i) {
        int propIdx = i * N_PER_PROPERTY;
        tempProperties.push_back({
            0, // componentIndex
            i, // displayOrder
            1, // isMandatory
            0, // isList
            1, // showValues
            0, // isObsolete
            1, // show
            (byte) types[i], // valueType
            20, // defaultValueIndex;
            offsets[propIdx + 1], // nameOffset
            offsets[propIdx + 2], // descriptionOffset
            offsets[propIdx + 3], // categoryOffset
            offsets[propIdx + 4], // urlOffset
            5, // firstValueIndex
            15, // lastValueIndex
            4, // mapCount
            0 // firstMapIndex
        });
    }
    propertiesCollectionHelper = new FixedSizeCollection<fiftyoneDegreesProperty>(tempProperties);
    propertiesCollection = propertiesCollectionHelper->getState()->collection;
}

void PropertyTests::SetUp() {
    CreateObjects();
    Base::SetUp();
}

void PropertyTests::TearDown() {
    Base::TearDown();
    delete stringsCollectionHelper;
    delete propertiesCollectionHelper;
}

void PropertyTests::assessProperty(const fiftyoneDegreesProperty *p, int i) {
    EXCEPTION_CREATE
    
    const String *name = fiftyoneDegreesPropertyGetName(stringsCollection, p, &item, exception);
    EXPECT_STREQ(strings[i * N_PER_PROPERTY + 1], &name->value);
    
    const String *descr = fiftyoneDegreesPropertyGetDescription(stringsCollection, p, &item, exception);
    EXPECT_STREQ(strings[i * N_PER_PROPERTY + 2], &descr->value);
    
    const String *cat = fiftyoneDegreesPropertyGetCategory(stringsCollection, p, &item, exception);
    EXPECT_STREQ(strings[i * N_PER_PROPERTY + 3], &cat->value);
    
    const String *url = fiftyoneDegreesPropertyGetUrl(stringsCollection, p, &item, exception);
    EXPECT_STREQ(strings[i*N_PER_PROPERTY + 4], &url->value);
}

TEST_F(PropertyTests, RetrievePropertyFromCollection) {
    
    for (int i=0;i<N_PROPERTIES;i++) {
        EXCEPTION_CREATE
        EXPECT_EQ(types[i], fiftyoneDegreesPropertyGetValueType(propertiesCollection, i, exception));
        {
            fiftyoneDegreesProperty *p = fiftyoneDegreesPropertyGet(propertiesCollection, i, &item, exception);
            assessProperty(p, i);
        }
        {
            const Property *p = fiftyoneDegreesPropertyGetByName(propertiesCollection, stringsCollection, strings[i * N_PER_PROPERTY + 1], &item, exception);
            assessProperty(p, i);
        }
    }
}
