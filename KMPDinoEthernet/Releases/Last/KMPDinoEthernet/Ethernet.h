#ifndef ethernet_h
#define ethernet_h

#include <inttypes.h>
#include "w5200.h"
#include "IPAddress.h"
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "Dhcp.h"

class EthernetClass {
    private:
    IPAddress _dnsServerAddress;
    DhcpClass* _dhcp;
    public:
    //static uint8_t _state[MAX_SOCK_NUM];
    static uint16_t _server_port[MAX_SOCK_NUM];
    // Initialize the Ethernet shield to use the provided MAC address and gain the rest of the
    // configuration through DHCP.
    // Returns 0 if the DHCP configuration failed, and 1 if it succeeded
    int begin(uint8_t *mac_address);
    void begin(uint8_t *mac_address, IPAddress local_ip);
    void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server);
    void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
    // KMP
    void begin(uint8_t* mac_address, uint8_t* local_ip, uint8_t* gateway, uint8_t* subnet);
    // KMP
    void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);
    int maintain();

    IPAddress localIP();
    IPAddress subnetMask();
    IPAddress gatewayIP();
    IPAddress dnsServerIP();
    // KMP
    void macAddress(uint8_t* macResult);
    void setAddresses(uint8_t* local_ip, uint8_t* gateway, uint8_t* subnet);
    void setAddresses(uint8_t* local_ip, uint8_t* gateway, uint8_t* subnet, uint8_t* mac);
    // KMP
    friend class EthernetClient;
    friend class EthernetServer;
};

extern EthernetClass Ethernet;

#endif
