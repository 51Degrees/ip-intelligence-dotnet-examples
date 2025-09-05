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

%include "common-cxx/EngineBase.i"
%include "ResultsIpi.i"
%include "ConfigIpi.i"
%include "common-cxx/EvidenceBase.i"
%include "EvidenceIpi.i"
%include "common-cxx/ip.i"

%newobject process;

%nodefaultctor EngineIpi;

%rename (EngineIpiSwig) EngineIpi;

class EngineIpi : public EngineBase {
public:
    EngineIpi(
        const std::string &fileName,
        ConfigIpi *config,
        RequiredPropertiesConfig *properties);
    EngineIpi(
        unsigned char data[],
        long length,
        ConfigIpi *config,
        RequiredPropertiesConfig *properties);
    Date getPublishedTime();
    Date getUpdateAvailableTime();
    std::string getDataFilePath();
    std::string getDataFileTempPath();
    void refreshData();
    void refreshData(const char *fileName);
    void refreshData(unsigned char data[], long length);
    ResultsIpi* process(EvidenceIpi *evidence);
    ResultsIpi* process(const char *ipAddress);
    ResultsIpi* process(
        unsigned char ipAddress[],
        long length,
        fiftyoneDegreesIpType type);
    ResultsBase* processBase(EvidenceBase *evidence);
};
