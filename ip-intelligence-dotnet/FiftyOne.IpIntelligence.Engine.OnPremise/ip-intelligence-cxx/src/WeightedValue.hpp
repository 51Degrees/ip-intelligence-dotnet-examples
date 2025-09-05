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

#ifndef FIFTYONE_DEGREES_WEIGHTED_VALUE_HPP
#define FIFTYONE_DEGREES_WEIGHTED_VALUE_HPP

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		template <class T> class WeightedValue {
			/**
			 * Class that represents a single value returned for a
			 * IP Intelligence property. Each value is associated
			 * with a weight in percentage to represent the likeness
			 * of its accuracy.
			 */
			public:
				/**
				 * @name Constructors
				 * @{
				 */

				/**
				 * Construct a default instance with default value
				 * and 0 weight
				 */
				WeightedValue<T>() {
					this->value = T();
					this->rawWeight = 0;
				};

				/**
				 * Construct an instance with provided value and weight
				 * @param value the value of the instance
				 * @param weight of the value
				 */
				WeightedValue<T>(T value, uint16_t weight) {
					this->value = value;
					this->rawWeight = weight;
				};

				/**
				 * @}
				 * @name Setters and Getters
				 * @{
				 */
		
				/**
				 * Get the value
				 * @return the value
				 */
				T getValue() const { return value; };

				/**
				 * Set the value
				 * @param v the value to set
				 */
				void setValue(T v) { value = v; };

				/**
				 * Get the weight (0.0 ~ 1.0)
				 * @return the weight
				 */
				float getWeight() const { return rawWeight / 65535.f; };

				/**
				 * Get the raw weight (1 ~ 65535)
				 * @return the raw weight
				 */
				uint16_t getRawWeight() const { return rawWeight; };

				/**
				 * Set the raw weight (1 ~ 65535)
				 * @param w the raw weight to set
				 */
				void setRawWeight(uint16_t w) { rawWeight = w; };

				/**
				 * @}
				 */
			private:
				/** The value */
				T value;
				/** The weight of the value, (1 ~ 65535) */
				uint16_t rawWeight;
		};
	}
}

#endif
