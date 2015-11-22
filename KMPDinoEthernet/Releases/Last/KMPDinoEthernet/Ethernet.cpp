#include "w5200.h"
#include "Ethernet.h"
#include "Dhcp.h"

// XXX: don't make assumptions about the value of MAX_SOCK_NUM.
//uint8_t EthernetClass::_state[MAX_SOCK_NUM] = {
//0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t EthernetClass::_server_port[MAX_SOCK_NUM] = {
0, 0, 0, 0, 0, 0, 0, 0 };

int EthernetClass::begin(uint8_t *mac_address)
{
    _dhcp = new DhcpClass();

    // Initialize the basic info
    W5200.init();
    W5200.setMACAddress(mac_address);
    W5200.setIPAddress(IPAddress(0,0,0,0).raw_address());

    // Now try to get our config info from a DHCP server
    int ret = _dhcp->beginWithDHCP(mac_address);
    if(ret == 1)
    {
        // We've successfully found a DHCP server and got our configuration info, so set things
        // accordingly
        //W5200.setIPAddress(_dhcp->getLocalIp().raw_address());
        //W5200.setGatewayIp(_dhcp->getGatewayIp().raw_address());
        //W5200.setSubnetMask(_dhcp->getSubnetMask().raw_address());
        setAddresses(_dhcp->getLocalIp().raw_address(), _dhcp->getSubnetMask().raw_address(), _dhcp->getGatewayIp().raw_address());
        _dnsServerAddress = _dhcp->getDnsServerIp();
    }

    return ret;
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip)
{
    // Assume the DNS server will be the machine on the same network as the local IP
    // but with last octet being '1'
    IPAddress dns_server = local_ip;
    dns_server[3] = 1;
    begin(mac_address, local_ip, dns_server);
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server)
{
    // Assume the gateway will be the machine on the same network as the local IP
    // but with last octet being '1'
    IPAddress gateway = local_ip;
    gateway[3] = 1;
    begin(mac_address, local_ip, dns_server, gateway);
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
    IPAddress subnet(255, 255, 255, 0);
    begin(mac_address, local_ip, dns_server, gateway, subnet);
}

void EthernetClass::begin(uint8_t* mac_address, uint8_t* local_ip, uint8_t* gateway, uint8_t* subnet)
{
    IPAddress dns_server = IPAddress(local_ip);
    begin(mac_address, IPAddress(local_ip), dns_server, IPAddress(gateway), IPAddress(subnet));
}

void EthernetClass::begin(uint8_t *mac, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
    W5200.init();
    //W5200.setMACAddress(mac);
    //W5200.setIPAddress(local_ip._address);
    //W5200.setGatewayIp(gateway._address);
    //W5200.setSubnetMask(subnet._address);

    setAddresses(local_ip.raw_address(), gateway.raw_address(), subnet.raw_address(), mac);
    _dnsServerAddress = dns_server;
}

int EthernetClass::maintain()
{
    int rc = DHCP_CHECK_NONE;
    if(_dhcp != NULL)
    {
        //we have a pointer to dhcp, use it
        rc = _dhcp->checkLease();
        switch ( rc )
        {
            case DHCP_CHECK_NONE:
            //nothing done
            break;
            case DHCP_CHECK_RENEW_OK:
            case DHCP_CHECK_REBIND_OK:
            //we might have got a new IP.
            setAddresses(_dhcp->getLocalIp().raw_address(), _dhcp->getSubnetMask().raw_address(), _dhcp->getGatewayIp().raw_address());

            _dnsServerAddress = _dhcp->getDnsServerIp();
            
            //W5200.setIPAddress(_dhcp->getLocalIp().raw_address());
            //W5200.setGatewayIp(_dhcp->getGatewayIp().raw_address());
            //W5200.setSubnetMask(_dhcp->getSubnetMask().raw_address());
            break;
            default:
            //this is actually a error, it will retry though
            break;
        }
    }
    return rc;
}

void EthernetClass::setAddresses(uint8_t* local_ip, uint8_t* gateway, uint8_t* subnet)
{
    W5200.setIPAddress(local_ip);
    W5200.setGatewayIp(gateway);
    W5200.setSubnetMask(subnet);
}

void EthernetClass::setAddresses( uint8_t* local_ip, uint8_t* gateway, uint8_t* subnet, uint8_t* mac )
{
    W5200.setMACAddress(mac);
    setAddresses(local_ip, gateway, subnet);
}


IPAddress EthernetClass::localIP()
{
    IPAddress ret;
    W5200.getIPAddress(ret.raw_address());
    return ret;
}

IPAddress EthernetClass::subnetMask()
{
    IPAddress ret;
    W5200.getSubnetMask(ret.raw_address());
    return ret;
}

IPAddress EthernetClass::gatewayIP()
{
    IPAddress ret;
    W5200.getGatewayIp(ret.raw_address());
    return ret;
}

void EthernetClass::macAddress(uint8_t* macResult)
{
    return W5200.getMACAddress(macResult);
}

IPAddress EthernetClass::dnsServerIP()
{
    return _dnsServerAddress;
}

EthernetClass Ethernet;
