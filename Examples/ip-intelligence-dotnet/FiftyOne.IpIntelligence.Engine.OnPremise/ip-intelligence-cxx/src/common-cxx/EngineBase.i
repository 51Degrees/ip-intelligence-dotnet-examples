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

%include "RequiredPropertiesConfig.i"
%include "ResultsBase.i"
%include "Date.i"
%include "MetaData.i"

%newobject getUpdateAvailableTime;
%newobject getPublishedTime;
%newobject processBase;

%nodefaultctor EngineBase;

%rename (EngineBaseSwig) EngineBase;

class EngineBase
{
public:
	virtual ~EngineBase();
	void setLicenseKey(const std::string &licenceKey);
	void setDataUpdateUrl(const std::string &updateUrl);
	MetaData* getMetaData();
	bool getAutomaticUpdatesEnabled();
	virtual ResultsBase* processBase(EvidenceBase *evidence);
	virtual void refreshData();
	virtual void refreshData(const char *fileName);
	virtual void refreshData(unsigned char data[], long length);
	virtual std::string getDataUpdateUrl();
	virtual Date getPublishedTime();
	virtual Date getUpdateAvailableTime();
	virtual std::string getDataFilePath();
	virtual std::string getDataFileTempPath();
	virtual std::string getProduct();
	virtual std::string getType();
	std::vector<std::string>* getKeys();
	bool getIsThreadSafe();
};
