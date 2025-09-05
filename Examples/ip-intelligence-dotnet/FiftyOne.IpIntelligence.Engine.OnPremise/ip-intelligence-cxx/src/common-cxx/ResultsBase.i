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

%include std_string.i
%include std_vector.i

%include "Value.i"

%nodefaultctor ResultsBase;

%rename (ResultsBaseSwig) ResultsBase;

class ResultsBase {
public:
	~ResultsBase();

	int getAvailableProperties();
	bool containsProperty(const std::string &propertyName);
	std::vector<std::string> getProperties();
	std::string getPropertyName(int requiredPropertyIndex);

	Value<std::vector<std::string>> getValues(const std::string &propertyName);
	Value<std::vector<std::string>> getValues(int requiredPropertyIndex);

	Value<std::string> getValueAsString(const std::string &propertyName);
	Value<std::string> getValueAsString(int requiredPropertyIndex);

	Value<bool> getValueAsBool(const std::string &propertyName);
	Value<bool> getValueAsBool(int requiredPropertyIndex);

	Value<int> getValueAsInteger(const std::string &propertyName);
	Value<int> getValueAsInteger(int requiredPropertyIndex);

	Value<double> getValueAsDouble(const std::string &propertyName);
	Value<double> getValueAsDouble(int requiredPropertyIndex);
};