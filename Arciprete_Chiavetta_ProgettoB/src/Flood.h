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

#ifndef __INET_FLOOD_H
#define __INET_FLOOD_H

#include "inet/networklayer/flooding/Flooding.h"
#include "inet/common/packet/Packet.h"

using namespace omnetpp;
namespace inet {

class Flood : public Flooding
{
    public:
        Flood();
        virtual ~Flood();
        virtual void initialize(int stage) override;
    protected:
        virtual void handleUpperPacket(Packet *packet) override;
        virtual void handleLowerPacket(Packet *packet) override;
        void decapsulate(Packet *packet);
        void encapsulate(Packet *packet);
};
} // namespace inet
#endif /* __INET_FLOOD_H */
