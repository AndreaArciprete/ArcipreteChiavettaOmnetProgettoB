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

#include "Flood.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
namespace inet {

Define_Module(Flood);
Flood::Flood() {
}

Flood::~Flood() {
}


void Flood::initialize(int stage){
    NetworkProtocolBase::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {
            //initialize sequence number to 0
            seqNum = 0;
            nbDataPacketsReceived = 0;
            nbDataPacketsSent = 0;
            nbDataPacketsForwarded = 0;
            nbHops = 0;
            headerLength = par("headerLength");
            defaultTtl = par("defaultTtl");
            plainFlooding = par("plainFlooding");
            /* Il parametro plainFlooding è sempre posto a true (nel file Flood.ned) in quanto vogliamo che
               qualora un nodo riceva un pacchetto avente la stessa coppia (src, seq) di quella di un altro pacchetto precedentemente trasmesso, il pacchetto non venga inoltrato
               ulteriormente ma venga scartato
             */
            if (plainFlooding) {
                //these parameters are only needed for plain flooding
                bcMaxEntries = par("bcMaxEntries"); //Numero massimo di entries che ci possono essere all'interno della lista dei pacchetti già inoltrati
                bcDelTime = par("bcDelTime"); //Tempo di cancellazione decorso il quale l'entry all'interno della lista dei pacchetti già inoltrati viene cancellata
            }
        }
}


void Flood::handleUpperPacket(Packet *packet){
/*Questo metodo prende il messaggio proveniente dal livello applicativo, lo incapsula in un pacchetto di rete
  e lo invia al livello sottostante
 */
EV << "Ricevuto messaggio dal livello applicativo" << endl;
encapsulate(packet);
auto floodHeader = packet->peekAtFront<FloodingHeader>();

if (plainFlooding) {
    /*
     Se la lista bcMsgs ha raggiunto la capienza massima allora occorre scartare eventuali pacchetti obsoleti
     */
    if (bcMsgs.size() >= bcMaxEntries) {
        /*Dalla lista bcMsgs dei pacchetti già inoltrati andiamo a cancellare tutti i pacchetti obsoleti, cioè i pacchetti aventi
          un tempo di cancellazione (delTime) < del tempo di simulazione corrente (simTime());
         */
        for (auto it = bcMsgs.begin(); it != bcMsgs.end(); ) {
            //Stiamo scorrendo la lista per eliminare eventuali pacchetti obsoleti
            if (it->delTime < simTime())
                it = bcMsgs.erase(it); //erase consente di rimuovere un elemento dalla lista
            else
                ++it;
        }
        /*Se l'operazione precedente non ha individuato alcun pacchetto obsoleto e dunque la lista presenta ancora la capienza massima
          allora si elimina il pacchetto in testa alla lista in modo da far spazio al nuovo pacchetto
         */
        if (bcMsgs.size() >= bcMaxEntries) {
            EV << "bcMsgs is full, delete oldest entry" << endl;
            bcMsgs.pop_front();
        }
    }
/*Nella lista bcMsgs andiamo ad inserire il pacchetto appena creato in maniera tale che se il livello di rete del nodo sorgente riceve
  un pacchetto avente la stessa coppia (src, seq) di quella del pacchetto appena inserito nella lista allora tale pacchetto non verrà inoltrato
  ulteriormente (per capire se una pacchetto debba essere o meno inoltrato utilizziamo il metodo notBroadcasted presente in Flooding.cc)
 */
    bcMsgs.push_back(Bcast(floodHeader->getSeqNum(), floodHeader->getSourceAddress(), simTime() + bcDelTime));
}
//Inviamo il pacchetto al livello sottostante
EV << "Inviamo il pacchetto avente numero di sequenza: " << floodHeader->getSeqNum() << " al livello sottostante" << endl;
sendDown(packet);
nbDataPacketsSent++;
}

void Flood::handleLowerPacket(Packet *packet){
    auto floodHeader = packet->peekAtFront<FloodingHeader>();

       //Se entriamo in questo if significa che il pacchetto ricevuto non lo avevamo ancora ricevuto
       if (notBroadcasted(floodHeader.get())) {
           //Se entriamo in questo if significa che il livello di rete del nodo di destinazione ha ricevuto correttamente il pacchetto.
           if (interfaceTable->isLocalAddress(floodHeader->getDestinationAddress())) {
               EV << "Sono il livello di rete del nodo destinatario, ho ricevuto correttamente il pacchetto e invio il messaggio al livello sovrastante" << endl;
               nbHops = nbHops + (defaultTtl + 1 - floodHeader->getTtl()); //Numero medio di hops.
               decapsulate(packet);
               sendUp(packet);
               nbDataPacketsReceived++;
           }
           /*Se entriamo in questo else significa che il livello di rete che ha ricevuto il pacchetto non è quello del
             nodo di destinazione ma è il livello di rete di un forwarder node che dunque, deve, se il TTL è > 0, decrementare di 1 il TTL e
             inoltrare il pacchetto ai nodi vicini
            */
           else {
               // Controlliamo il TTL
               if (floodHeader->getTtl() > 0) {
                   char nomePacchettoRisposta[300];
                   char nomeHost[100];
                   EV << "Sono il livello di rete di un forwarder node, il TTL prima del suo decremento è pari a: " << floodHeader->getTtl() << ", inoltro il pacchetto al livello sottostante" << endl;
                   decapsulate(packet);

                   auto packetCopy = new Packet();

                   packetCopy->insertAtBack(packet->peekDataAt(b(0), packet->getDataLength()));
                   auto floodHeaderCopy = staticPtrCast<FloodingHeader>(floodHeader->dupShared());
                   sprintf(nomeHost, "%s", floodHeader->getSrcAddr().str().c_str());
                   sprintf(nomePacchettoRisposta,"%s, Reply: Ping%d",nomeHost, floodHeader->getSeqNum());
                   packetCopy->setName(nomePacchettoRisposta);
                   floodHeaderCopy->setTtl(floodHeader->getTtl() - 1);
                   packetCopy->insertAtFront(floodHeaderCopy);
                   cObject *const pCtrlInfo = packetCopy->removeControlInfo();
                   if (pCtrlInfo != nullptr)
                       delete pCtrlInfo;
                   setDownControlInfo(packetCopy, MacAddress::BROADCAST_ADDRESS);
                   sendDown(packetCopy);
                   nbDataPacketsForwarded++;
                   delete packet;
               }
               //Entriamo in questo else se il TTL non è maggiore di 0, in tal caso il pacchetto viene scartato
               else {
                   //max hops reached -> delete
                   EV << "E' stato raggiunto il numero massimo di hops e dunque dal momento che il TTL = " << floodHeader->getTtl() << ", il pacchetto deve essere scartato" << endl;
                   delete packet;
               }
           }
       }
       else {
           EV << "Dal momento che è stato già ricevuto un pacchetto avente la stessa coppia (src, seq), esso va scartato" << endl;
           delete packet;
       }
}


void Flood::decapsulate(Packet *packet){
    auto floodHeader = packet->popAtFront<FloodingHeader>();
        auto payloadLength = floodHeader->getPayloadLengthField();
        if (packet->getDataLength() < payloadLength) {
            throw cRuntimeError("Errore! Lunghezza del Payload non supportata");
        }
        if (packet->getDataLength() > payloadLength)
            packet->setBackOffset(packet->getFrontOffset() + payloadLength);
        packet->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&getProtocol());
        packet->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(floodHeader);
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::udp);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::udp);
        auto addressInd = packet->addTagIfAbsent<L3AddressInd>();
        addressInd->setSrcAddress(floodHeader->getSourceAddress());
        addressInd->setDestAddress(floodHeader->getDestinationAddress());
        packet->addTagIfAbsent<HopLimitInd>()->setHopLimit(floodHeader->getTtl());
}

void Flood::encapsulate(Packet *appPkt)
{
    L3Address netwAddrDst;
    L3Address netwAddrSrc;

    EV << "Incapsulamento" <<endl;

    auto cInfo = appPkt->removeControlInfo();
    auto pkt = makeShared<FloodingHeader>();
    pkt->setChunkLength(b(headerLength));

    auto hopLimitReq = appPkt->removeTagIfPresent<HopLimitReq>();
    int ttl = (hopLimitReq != nullptr) ? hopLimitReq->getHopLimit() : -1;
    delete hopLimitReq;
    if (ttl == -1)
        ttl = defaultTtl;

    pkt->setSeqNum(seqNum);
    seqNum++;
    pkt->setTtl(ttl);

    auto addressReq = appPkt->findTag<L3AddressReq>();
    if (addressReq == nullptr) {
        netwAddrDst = netwAddrDst.getAddressType()->getBroadcastAddress();
    }
    else {
        pkt->setProtocol(appPkt->getTag<PacketProtocolTag>()->getProtocol());
        netwAddrDst = addressReq->getDestAddress();
        netwAddrSrc = addressReq->getSrcAddress();
        delete cInfo;
    }

    pkt->setSrcAddr(netwAddrSrc);
    pkt->setDestAddr(netwAddrDst);
    pkt->setPayloadLengthField(appPkt->getDataLength());
    setDownControlInfo(appPkt, MacAddress::BROADCAST_ADDRESS);
    appPkt->insertAtFront(pkt);
    EV << "Pacchetto incapsulato correttamente" << endl;
}


} // namespace inet
