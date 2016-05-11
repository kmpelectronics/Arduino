// ICMPProtocol.cpp
// The library is written on the basis of Blake Foster <blfoster@vassar.edu>.
// Currently implemented only Ping functionality.
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// License: See the GNU General Public License for more details at http://www.gnu.org/copyleft/gpl.html
// Version: 1.0.0
// Date: 17.04.2014
// Author: Plamen Kovandjiev <p.kovandiev@kmpelectronics.eu>

#include "ICMPProtocol.h"
#include "utility/util.h"

/**
* @brief Convert two bytes in word in little endian Arduino format.
* @param highOrder high order byte.
* @param lowOrder low order byte.
* @return Word.
*/
uint16_t Uint16From2Uint8(const uint8_t& highOrder, const uint8_t& lowOrder)
{
    // make a 16-bit unsigned integer given the low order and high order bytes.
    // lowOrder first because the Arduino is little endian.
    uint8_t value [] = {lowOrder, highOrder};

    return *(uint16_t *)&value;
}

/**
* @brief Add check sum in ICMP packet.
* @param icmpPacket ICMP packet data.
*/
void CalcChecksum(uint8_t * icmpPacket)
{
    // calculate the checksum of an ICMPEcho with all fields but icmpHeader.checksum populated
    unsigned long sum = 0;

    // add the header, bytes reversed since we're using little-endian arithmetic.
    //hdr.type, hdr.code
    sum += Uint16From2Uint8(*(icmpPacket++), *(icmpPacket++));

    // Save check sum position.
    uint8_t* checkSumPos = icmpPacket;
    // Skip check sum position.
    icmpPacket += 2;

    // Add id.
    sum += *(uint16_t *)icmpPacket;
    icmpPacket += 2;
    // Add sequence.
    sum += *(uint16_t *)icmpPacket;
    icmpPacket += 2;

    // Add the payload data.
    for (uint8_t i = 0; i < PAYLOAD_DATA_SIZE; i+=2)
    {
    sum += Uint16From2Uint8(*(icmpPacket++), *(icmpPacket++));
    }

    // Add high 16 to low 16.
    sum = (sum >> 16) + (sum & 0xffff);
    // Add carry.
    sum += (sum >> 16);
    // Fill check sum.
    *(uint16_t *)checkSumPos = ~sum;
}

/**
* @brief Find and return free socket number.
* @return Free socket number. If not find free - return MAX_SOCK_NUM.
*/
uint8_t GetFreeSocket()
{
    for (int i = 0; i < MAX_SOCK_NUM; i++) 
    {
        uint8_t s = W5100.readSnSR(i);
        if (s == SnSR::CLOSED || s == SnSR::FIN_WAIT
        || s == SnSR::CLOSE_WAIT) 
        {
            return i;
        }
    }

    return MAX_SOCK_NUM;
}

ICMPProtocol::ICMPProtocol()
{
    _nextSeq = 0;
}

ICMPEchoReply ICMPProtocol::Ping(uint8_t * addr, uint16_t id, int nRetries)
{
    ICMPEchoReply result;
    
    // Get free socket.
    SOCKET socket = GetFreeSocket();
    // If not find free - return.
    if(socket == MAX_SOCK_NUM)
    {
        result.status = NO_FREE_SOCKET;
        return result;
    }

    // Fill header.
    ICMPHeader hdr;
    hdr.type = ICMP_ECHOREQ;
    hdr.code = ICMP_CODE;
    hdr.id = id;
    hdr.seq = ++_nextSeq;

    // Prepare and open socket.
    W5100.execCmdSn(socket, Sock_CLOSE);
    W5100.writeSnIR(socket, 0xFF);
    W5100.writeSnMR(socket, SnMR::IPRAW);
    W5100.writeSnPROTO(socket, IPPROTO::ICMP);
    W5100.writeSnPORT(socket, 0);
    W5100.execCmdSn(socket, Sock_OPEN);

    // Calculate ping time.
    unsigned long startTime = millis();

    // If not SUCCESS retries.
    for (int i = 0; i < nRetries; ++i)
    {
        result.status = sendEchoRequest(socket, addr, hdr);
        if (result.status == SUCCESS)
        {
            receiveEchoReply(socket, hdr, result);
        }
        
        if (result.status == SUCCESS)
        {
            break;
        }
    }

    result.time = millis() - startTime;

    // Close socket.    
    W5100.execCmdSn(socket, Sock_CLOSE);
    W5100.writeSnIR(socket, 0xFF);

    return result;
}

ICMPStatus ICMPProtocol::sendEchoRequest(SOCKET socket, uint8_t * addr, const ICMPHeader& hdr)
{
    // Prepare ICMP packet.
    uint8_t icmpPacket[ICMP_PACKET_SIZE];
    uint8_t * icmpPtr = icmpPacket;

    // Fill packet.
    // Type.
    *(icmpPtr++) = hdr.type;
    // Code.
    *(icmpPtr++) = hdr.code;
    // Skip Checksum, added letter from CalcChecksum void.
    icmpPtr += 2;
    // Identifier.
    *(uint16_t *)icmpPtr = htons(hdr.id);
    icmpPtr += 2;
    // Sequence.
    *(uint16_t *)icmpPtr = htons(hdr.seq);
    icmpPtr += 2;
    
    // Fill payload content.
    uint8_t fillChar = PAYLOAD_CONTENT_START_CHAR;
    for(uint16_t i = 0; i < PAYLOAD_DATA_SIZE; i++)
    {
        *(icmpPtr++) = fillChar++;
    }

    // Add check sum.
    CalcChecksum(icmpPacket);
    
    W5100.writeSnDIPR(socket, addr);
    W5100.writeSnTTL(socket, TTL);
    // The port isn't used, because ICMP is a network-layer protocol. So we
    // write zero. This probably isn't actually necessary.
    W5100.writeSnDPORT(socket, 0);

    // Send packet.
    W5100.send_data_processing(socket, icmpPacket, ICMP_PACKET_SIZE);
    W5100.execCmdSn(socket, Sock_SEND);

    // Waiting to send.
    while ((W5100.readSnIR(socket) & SnIR::SEND_OK) != SnIR::SEND_OK)
    {
        // If send timeout.
        if (W5100.readSnIR(socket) & SnIR::TIMEOUT)
        {
            W5100.writeSnIR(socket, (SnIR::SEND_OK | SnIR::TIMEOUT));
            return SEND_TIMEOUT;
        }
    }
    
    W5100.writeSnIR(socket, SnIR::SEND_OK);

    return SUCCESS;
}

void ICMPProtocol::receiveEchoReply(SOCKET socket, ICMPHeader& reqHdr, ICMPEchoReply& echoReply)
{
    // Calculate time waiting for reply.
    unsigned long endTime = millis() + PING_TIMEOUT;

    // Waiting for reply.
    while (millis() < endTime)
    {
        if (W5100.getRXReceivedSize(socket))
        {
            // Read IP header.
            uint8_t ipHeader [6];
            uint16_t buffer = W5100.readSnRX_RD(socket);
            W5100.read_data(socket, buffer, ipHeader, sizeof(ipHeader));
            buffer += sizeof(ipHeader);
            
            // Fill IP address.
            for (int i = 0; i < 4; ++i)
            {
                echoReply.addr[i] = ipHeader[i];
            }

            uint8_t dataLen = (ipHeader[4] << 8) + ipHeader[5];
            
            // Read only header. Other data (payload) not need.
            uint8_t icmpHdrBuff[ICMP_HEADER_SIZE];

            if (dataLen > ICMP_HEADER_SIZE)
                dataLen = ICMP_HEADER_SIZE;
            
            W5100.read_data(socket, buffer, icmpHdrBuff, dataLen);
            
            buffer += dataLen;
            W5100.writeSnRX_RD(socket, buffer);
            W5100.execCmdSn(socket, Sock_RECV);

            uint8_t * icmpPtr = icmpHdrBuff;
            // Read type.
            uint8_t type = *(icmpPtr++);
            // Skip code(1) and check sum(2) = 3 bytes.
            icmpPtr +=3;
            // Read id.
            uint16_t id = ntohs(*(uint16_t *)icmpPtr);
            icmpPtr +=2;
            // Read sequence.
            echoReply.seq = ntohs(*(uint16_t *)icmpPtr);

            // Since there aren't any ports in ICMP, we need to manually inspect the response
            // to see if it originated from the request we sent out.
            if (type == ICMP_ECHOREP && id == reqHdr.id && echoReply.seq == reqHdr.seq)
            {
                echoReply.ttl = W5100.readSnTTL(socket);
                echoReply.status = SUCCESS;
                return;
            }
        }
    }
    echoReply.status = NO_RESPONSE;
}