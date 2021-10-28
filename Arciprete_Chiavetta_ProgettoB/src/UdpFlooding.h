//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_UDPFLOODING_H
#define __INET_UDPFLOODING_H

#include "inet/transportlayer/udp/Udp.h"
#include "inet/common/packet/Packet.h"

using namespace omnetpp;
namespace inet {

class UdpFlooding : public Udp
{
    public:
        UdpFlooding();
        virtual ~UdpFlooding();
    protected:
        virtual void handleLowerPacket(Packet *appData) override;
};
} // namespace inet
#endif /* __INET_UDPFLOODING_H */
