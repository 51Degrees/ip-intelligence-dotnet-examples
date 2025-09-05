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

#ifndef FIFTYONE_DEGREES_WKBTOT_HPP_INCLUDED
#define FIFTYONE_DEGREES_WKBTOT_HPP_INCLUDED

#include "wkbtot.h"
#include "string.h"
#include <sstream>

namespace FiftyoneDegrees::Common {
    /**
     * Converts WKB "string" to WKT string and pushes into a string stream.
     * @param wkbString "string" containing WKB geometry.
     * @param reductionMode type/value reduction applied to decrease WKB size.
     * @param stream string stream to push WKT into.
     * @param decimalPlaces precision for numbers (places after the decimal dot).
     * @param exception pointer to the exception struct.
     * @return How many bytes were written to the buffer and if it was too small.
     */
    fiftyoneDegreesWkbtotResult writeWkbStringToStringStream(
        const fiftyoneDegreesVarLengthByteArray *wkbString,
        fiftyoneDegreesWkbtotReductionMode reductionMode,
        std::stringstream &stream,
        uint8_t decimalPlaces,
        fiftyoneDegreesException *exception);
}

#endif //FIFTYONE_DEGREES_WKBTOT_HPP_INCLUDED
