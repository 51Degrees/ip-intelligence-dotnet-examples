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

#ifndef FIFTYONE_DEGREES_DATE_HPP
#define FIFTYONE_DEGREES_DATE_HPP

#include "date.h"

namespace FiftyoneDegrees {
	namespace Common {
		/**
		 * The Date class wraps the C #fiftyoneDegreesDate structure and is
		 * used to represent dates in engines. This is a memory efficient way
		 * to store a date.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * FiftyoneDegrees::Common::Date *date;
		 * std::stringstream stream;
		 *
		 * // Format a string representation of a date in the format DD/MM/YYY
		 * stream << date->getDay() << "/"
		 *     << date->getMonth() << "/"
		 *     << date->getYear();
		 * string dateString = stream.str();
		 * ```
		 */
		class Date {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Construct a "null" date with year, month and day all set to zero.
			 */
			Date();

			/**
			 * Construct a date from a C date structure. This copies the date
			 * date structure so the argument can be freed after construction.
			 * @param date pointer to copy to the underlying date structure
			 */
			Date(const fiftyoneDegreesDate *date);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the year
			 * @return year
			 */
			int getYear() const;

			/**
			 * Get the month
			 * @return month
			 */
			int getMonth() const;

			/**
			 * Get the day of the month
			 * @return day of the month
			 */
			int getDay() const;

			/**
			 * @}
			 */
		private:
			/** The underlying date structure. */
			fiftyoneDegreesDate date;
		};
	}
}

#endif