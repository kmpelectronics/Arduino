// ICMPProtocol.h
// The library is written on the basis of Blake Foster <blfoster@vassar.edu>.
// Currently implemented only Ping functionality.
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Information:
//  - Wiki ICMP: http://en.wikipedia.org/wiki/Ping_(networking_utility)
//  - Check sum: http://sickbits.net/code2/sendicmp.c.txt
//  - Other ICMP info: http://support.microsoft.com/kb/170292
// ICMP packet structure:
// -------------------------------------
//  01234567 01234567 01234567 01234567
// -------------------------------------
// |  Type  |  Code  |   Check sum     |
// -------------------------------------
// |   Identifier    |    Sequence     |
// -------------------------------------
// |           Payload Data            |
// -------------------------------------
// Version: 1.0.0
// Date: 17.04.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#ifndef	ICMPPING_H_INCLUDED
#define	ICMPPING_H_INCLUDED

#include <Ethernet.h>
#include "utility/w5100.h"

// ICMP Echo request type.
#define ICMP_ECHOREQ 8
// ICMP Echo reply type.
#define ICMP_ECHOREP 0
// ICMP Code for ping.
#define ICMP_CODE 0
// ICMP header size in bytes.
#define ICMP_HEADER_SIZE 8
// Payload data size send with ping.
#define PAYLOAD_DATA_SIZE 16
// ICMP packed size.
#define ICMP_PACKET_SIZE ICMP_HEADER_SIZE + PAYLOAD_DATA_SIZE
// Payload start with char number.
#define PAYLOAD_CONTENT_START_CHAR 32 // Space ' '
// Ping timeout in milliseconds.
#define PING_TIMEOUT 1000
// Time To Live packet.
#define TTL 255

/**
* Indicates whether a ping succeeded or failed due to one of various error
* conditions. These correspond to error conditions that occur in this
* library, not anything defined in the ICMP protocol.
*/
enum ICMPStatus
{
    SUCCESS = 0,
    /**
    * Timed out sending the request.
    */
    SEND_TIMEOUT = 1,
    /**
    * Died waiting for a response.
    */
    NO_RESPONSE = 2,
    /**
    * Back the wrong type.
    */
    BAD_RESPONSE = 3,
    /**
    * Can not get free socket from W5200.
    */
    NO_FREE_SOCKET = 4 //
};

/**
* Header for an ICMP packet.
*/
struct ICMPHeader
{
    // Type of ICMP message.
    uint8_t type;
    // Code. More information: http://support.microsoft.com/kb/170292.
    uint8_t code;
    // The Identifier and Sequence Number can be used by the client to match the reply with the request that caused the reply.
    uint16_t id;
    // Sequence Number.
    uint16_t seq;
};

/**
* Struct returned from Ping().
*/
struct ICMPEchoReply
{
    // Time to live.
    uint8_t ttl;
    // SUCCESS if the ping succeeded. One of various error codes if it failed.
    ICMPStatus status;
    // The IP address that we received the response from. Something is broken if this doesn't match the IP address we pinged.
    uint8_t addr[4];
    // Time in milliseconds for replay.
    unsigned long time;
    // Sequence number.
    uint16_t seq;
};


/**
* @brief Represent class sending ICMP echo ping requests.
*/
class ICMPProtocol
{
    public:
        // Construct an ICMP ping object.
        ICMPProtocol();

        /*
        @brief Ping address.
        @param addr: IP address to ping, as an array of four octets.
        @param id: The id to put in the ping packets. Can be pretty much any arbitrary number.
        @param nRetries: Number of times to retry before giving up.
        @return ICMPEchoReply containing the response. The status field in
        the return value indicates whether the echo request succeeded or
        failed. If the request failed, the status indicates the reason for
        failure on the last retry.
        */
        ICMPEchoReply Ping(uint8_t * addr, uint16_t id, int nRetries);

    private:
        /*
        @brief Send echo request.
        @param socket: Socket through which communicates
        @param addr: IP address to ping.
        @param hdr: ICMP header data.
        @return ICMPStatus return status.
        */
        ICMPStatus sendEchoRequest(SOCKET socket, uint8_t * addr, const ICMPHeader& hdr);
        /*
        @brief Receive echo reply.
        @param socket: Socket through which communicates
        @param hdr: ICMP header data.
        @param addr: IP address to ping.
        @param ICMPEchoReply return received status.
        */
        void receiveEchoReply(SOCKET socket, ICMPHeader& hdr, ICMPEchoReply& echoReply);

        // Store next sequence number.
        uint16_t _nextSeq;
};

#endif