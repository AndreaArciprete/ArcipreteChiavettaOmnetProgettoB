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

#include "UdpFlooding.h"
#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/MulticastTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/common/L4Tools.h"

namespace inet {

Define_Module(UdpFlooding);
UdpFlooding::UdpFlooding() {
}

UdpFlooding::~UdpFlooding() {
}

void UdpFlooding::handleLowerPacket(Packet *payload) {
            payload->setKind(UDP_I_DATA);
            delete payload->removeTagIfPresent<PacketProtocolTag>();
            delete payload->removeTagIfPresent<DispatchProtocolReq>();
            payload->addTagIfAbsent<SocketInd>()->setSocketId(socketsByIdMap.begin()->first);
            payload->addTagIfAbsent<TransportProtocolInd>()->setProtocol(&Protocol::udp);
            payload->addTagIfAbsent<L4PortInd>()->setSrcPort(1);
            payload->addTagIfAbsent<L4PortInd>()->setDestPort(par("destPort"));
            emit(packetSentToUpperSignal, payload);
            send(payload, "appOut");
}



} // namespace inet
