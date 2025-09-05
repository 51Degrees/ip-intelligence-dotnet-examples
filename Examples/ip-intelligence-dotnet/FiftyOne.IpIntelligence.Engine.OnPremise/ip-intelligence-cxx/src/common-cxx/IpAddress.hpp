/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
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

#ifndef FIFTYONE_DEGREES_IP_ADDRESS_HPP
#define FIFTYONE_DEGREES_IP_ADDRESS_HPP

#include "ip.h"

namespace FiftyoneDegrees {
    namespace IpIntelligence {
        /**
         * A class which represents an IP address.
         *
         * This class is to give the IP address byte array
         * a more concrete and 'easy to work with' form so
         * that it can be passed between managed and unmanaged
         * layers
         */
    	class IpAddress {
        public:
            /**
             * @name Constructors
             * @{
             */

            /**
             * Construct a default instance with an
             * invalid IP address
             */
            IpAddress();
    
            /**
             * Construct an instance with a given
             * combination of IP address byte array
             * and its type
             * @param ipAddressData the IP address byte array
             * @param addressType the type of the IP address
             */
            IpAddress(const unsigned char ipAddressData[],
                      fiftyoneDegreesIpType addressType);

            /**
             * Construct an instance with a given
             * IP address string. The type of the IP
             * address is determined by parsing the string
             * @param ipAddressString the IP address string
             */
            IpAddress(const char *ipAddressString);

            /**
             * @}
             * @name Getters
             * @{
             */

            /**
             * Get the IP address byte array
             * @return a constant pointer to the internal
             * byte array
             */
            const unsigned char *getIpAddress() const {
                return (const unsigned char *)ipAddress;
            };

            /**
             * Returns a copy of the IP address byte array
             * This is used mainly for SWIG so that other
             * language can get a value of the byte array.
             * By using carrays.i in SWIG, any access to
             * this copy can be done via SWIG array functions.
             *
             * To get the actual pointer, the getIpAddress
             * should be used.
             * @param copy which will hold a copy of the byte array
             * @param size of the copy buffer
             */
            void getCopyOfIpAddress(unsigned char copy[], uint32_t size) const;

            /**
             * Get the type of the IP address
             * @return the type of IP address
             */
            fiftyoneDegreesIpType getType() const { return type; };

            /**
             *@}
             */
    
        private:
            /**
             * Initialization function
             * @param ipAddress the byte array IP address
             * @param type the type of the IP
             */
            void init(const unsigned char *ipAddressData,
                      fiftyoneDegreesIpType addressType);

            /** The type of the IP address */
            fiftyoneDegreesIpType type;
            /** The IP address byte array */
            unsigned char ipAddress[FIFTYONE_DEGREES_IPV6_LENGTH];
        };
    }
}

#endif
