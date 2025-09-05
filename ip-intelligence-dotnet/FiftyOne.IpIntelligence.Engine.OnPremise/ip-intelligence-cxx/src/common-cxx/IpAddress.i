%include "ip.i"

%nodefaultctor IpAddress;

%rename (IpAddressSwig) IpAddress;

#ifdef SWIGJAVA
%include "JavaTypes.i"

%apply (unsigned char *UCHAR, fiftyoneDegreesIpType type) {(unsigned char ipAddress[], fiftyoneDegreesIpType type)}
%apply (unsigned char *UCHAR, uint32_t UINT32) {(unsigned char copy[], uint32_t size)}
#endif

class IpAddress {
public:
    IpAddress(const unsigned char ipAddress[], fiftyoneDegreesIpType type);
    IpAddress(const char *ipAddressString);
    void getCopyOfIpAddress(unsigned char copy[], uint32_t size);
    fiftyoneDegreesIpType getType();
};
