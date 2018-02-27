#ifndef NETWORK_H
#define NETWORK_H

#include "RakPeerInterface.h"
#include <string.h>
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "ReplicaManager3.h"
#include "NetworkIDManager.h"
#include "VariableDeltaSerializer.h"
#include "RakSleep.h"
#include "GetTime.h"
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "../game/world.h"
#include "network_ids.h"

#define STATICLIB

#include "miniupnpc.h"
#include "upnpcommands.h"
#include "upnperrors.h"

// ReplicaManager3 is in the namespace RakNet
using namespace RakNet;

enum
{
	CLIENT,
	SERVER
} network_topology;

struct network_object : public Replica3
{
	network_object();
	~network_object();

	virtual RakNet::RakString GetName(void) const=0;

	virtual void WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const;
	void PrintStringInBitstream(RakNet::BitStream *bs);


	virtual void SerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection);
	virtual bool DeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection);

	virtual void SerializeDestruction(RakNet::BitStream *destructionBitstream, RakNet::Connection_RM3 *destinationConnection);

	virtual bool DeserializeDestruction(RakNet::BitStream *destructionBitstream, RakNet::Connection_RM3 *sourceConnection);
	virtual void DeallocReplica(RakNet::Connection_RM3 *sourceConnection);

	/// Overloaded Replica3 function
	virtual void OnUserReplicaPreSerializeTick(void);

	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters);

	virtual void Deserialize(RakNet::DeserializeParameters *deserializeParameters);

	virtual void SerializeConstructionRequestAccepted(RakNet::BitStream *serializationBitstream, RakNet::Connection_RM3 *requestingConnection)	{
		serializationBitstream->Write(GetName() + RakNet::RakString(" SerializeConstructionRequestAccepted"));
	}
	virtual void DeserializeConstructionRequestAccepted(RakNet::BitStream *serializationBitstream, RakNet::Connection_RM3 *acceptingConnection) {
		PrintStringInBitstream(serializationBitstream);
	}
	virtual void SerializeConstructionRequestRejected(RakNet::BitStream *serializationBitstream, RakNet::Connection_RM3 *requestingConnection)	{
		serializationBitstream->Write(GetName() + RakNet::RakString(" SerializeConstructionRequestRejected"));
	}
	virtual void DeserializeConstructionRequestRejected(RakNet::BitStream *serializationBitstream, RakNet::Connection_RM3 *rejectingConnection) {
		PrintStringInBitstream(serializationBitstream);
	}

	virtual void OnPoppedConnection(RakNet::Connection_RM3 *droppedConnection)
	{
		// Same as in SerializeDestruction(), no longer track this system
		variableDeltaSerializer.RemoveRemoteSystemVariableHistory(droppedConnection->GetRakNetGUID());
	}
	void NotifyReplicaOfMessageDeliveryStatus(RakNetGUID guid, uint32_t receiptId, bool messageArrived)
	{
		// When using UNRELIABLE_WITH_ACK_RECEIPT, the system tracks which variables were updated with which sends
		// So it is then necessary to inform the system of messages arriving or lost
		// Lost messages will flag each variable sent in that update as dirty, meaning the next Serialize() call will resend them with the current values
		variableDeltaSerializer.OnMessageReceipt(guid,receiptId,messageArrived);
	}

	// Demonstrate per-variable synchronization
	// We manually test each variable to the last synchronized value and only send those values that change
	int var1Unreliable,var2Unreliable,var3Reliable,var4Reliable;

	// Class to save and compare the states of variables this Serialize() to the last Serialize()
	// If the value is different, true is written to the bitStream, followed by the value. Otherwise false is written.
	// It also tracks which variables changed which Serialize() call, so if an unreliable message was lost (ID_SND_RECEIPT_LOSS) those variables are flagged 'dirty' and resent
	VariableDeltaSerializer variableDeltaSerializer;
};

struct ClientCreatible_ClientSerialized : public network_object {
	virtual RakNet::RakString GetName(void) const {return RakNet::RakString("ClientCreatible_ClientSerialized");}
	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		return network_object::Serialize(serializeParameters);
	}
	virtual RM3ConstructionState QueryConstruction(RakNet::Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_ClientConstruction(destinationConnection,network_topology!=CLIENT);
	}
	virtual bool QueryRemoteConstruction(RakNet::Connection_RM3 *sourceConnection) {
		return QueryRemoteConstruction_ClientConstruction(sourceConnection,network_topology!=CLIENT);
	}

	virtual RM3QuerySerializationResult QuerySerialization(RakNet::Connection_RM3 *destinationConnection) {
		return QuerySerialization_ClientSerializable(destinationConnection,network_topology!=CLIENT);
	}
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection(RakNet::Connection_RM3 *droppedConnection) const {
		return QueryActionOnPopConnection_Client(droppedConnection);
	}
};
struct ServerCreated_ClientSerialized : public network_object {
	virtual RakNet::RakString GetName(void) const {return RakNet::RakString("ServerCreated_ClientSerialized");}
	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		return network_object::Serialize(serializeParameters);
	}
	virtual RM3ConstructionState QueryConstruction(RakNet::Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_ServerConstruction(destinationConnection,network_topology!=CLIENT);
	}
	virtual bool QueryRemoteConstruction(RakNet::Connection_RM3 *sourceConnection) {
		return QueryRemoteConstruction_ServerConstruction(sourceConnection,network_topology!=CLIENT);
	}
	virtual RM3QuerySerializationResult QuerySerialization(RakNet::Connection_RM3 *destinationConnection) {
		return QuerySerialization_ClientSerializable(destinationConnection,network_topology!=CLIENT);
	}
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection(RakNet::Connection_RM3 *droppedConnection) const {
		return QueryActionOnPopConnection_Server(droppedConnection);
	}
};
struct ClientCreatible_ServerSerialized : public network_object {
	virtual RakNet::RakString GetName(void) const {return RakNet::RakString("ClientCreatible_ServerSerialized");}
	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		if (network_topology==CLIENT)
			return RM3SR_DO_NOT_SERIALIZE;
		return network_object::Serialize(serializeParameters);
	}
	virtual RM3ConstructionState QueryConstruction(RakNet::Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_ClientConstruction(destinationConnection,network_topology!=CLIENT);
	}
	virtual bool QueryRemoteConstruction(RakNet::Connection_RM3 *sourceConnection) {
		return QueryRemoteConstruction_ClientConstruction(sourceConnection,network_topology!=CLIENT);
	}
	virtual RM3QuerySerializationResult QuerySerialization(RakNet::Connection_RM3 *destinationConnection) {
		return QuerySerialization_ServerSerializable(destinationConnection,network_topology!=CLIENT);
	}
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection(RakNet::Connection_RM3 *droppedConnection) const {
		return QueryActionOnPopConnection_Client(droppedConnection);
	}
};
struct ServerCreated_ServerSerialized : public network_object {
	virtual RakNet::RakString GetName(void) const {return RakNet::RakString("ServerCreated_ServerSerialized");}
	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		if (network_topology==CLIENT)
			return RM3SR_DO_NOT_SERIALIZE;

		return network_object::Serialize(serializeParameters);
	}
	virtual RM3ConstructionState QueryConstruction(RakNet::Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_ServerConstruction(destinationConnection,network_topology!=CLIENT);
	}
	virtual bool QueryRemoteConstruction(RakNet::Connection_RM3 *sourceConnection) {
		return QueryRemoteConstruction_ServerConstruction(sourceConnection,network_topology!=CLIENT);
	}
	virtual RM3QuerySerializationResult QuerySerialization(RakNet::Connection_RM3 *destinationConnection) {
		return QuerySerialization_ServerSerializable(destinationConnection,network_topology!=CLIENT);
	}
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection(RakNet::Connection_RM3 *droppedConnection) const {
		return QueryActionOnPopConnection_Server(droppedConnection);
	}
};
struct P2PReplica : public network_object {
	virtual RakNet::RakString GetName(void) const {return RakNet::RakString("P2PReplica");}
	virtual RM3ConstructionState QueryConstruction(RakNet::Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_PeerToPeer(destinationConnection);
	}
	virtual bool QueryRemoteConstruction(RakNet::Connection_RM3 *sourceConnection) {
		return QueryRemoteConstruction_PeerToPeer(sourceConnection);
	}
	virtual RM3QuerySerializationResult QuerySerialization(RakNet::Connection_RM3 *destinationConnection) {
		return QuerySerialization_PeerToPeer(destinationConnection);
	}
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection(RakNet::Connection_RM3 *droppedConnection) const {
		return QueryActionOnPopConnection_PeerToPeer(droppedConnection);
	}
};

class network_connection : public Connection_RM3 {
public:
	network_connection(const SystemAddress &_systemAddress, RakNetGUID _guid) : Connection_RM3(_systemAddress, _guid) {}
	virtual ~network_connection() {}

	// See documentation - Makes all messages between ID_REPLICA_MANAGER_DOWNLOAD_STARTED and ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE arrive in one tick
	bool QueryGroupDownloadMessages(void) const {return true;}

	virtual Replica3 *AllocReplica(RakNet::BitStream *allocationId, ReplicaManager3 *replicaManager3)
	{
		RakNet::RakString typeName;
		allocationId->Read(typeName);
		if (typeName=="ClientCreatible_ClientSerialized") return new ClientCreatible_ClientSerialized;
		if (typeName=="ServerCreated_ClientSerialized") return new ServerCreated_ClientSerialized;
		if (typeName=="ClientCreatible_ServerSerialized") return new ClientCreatible_ServerSerialized;
		if (typeName=="ServerCreated_ServerSerialized") return new ServerCreated_ServerSerialized;
		return 0;
	}
protected:
};

class network_mananger : public ReplicaManager3
{
	virtual Connection_RM3* AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const {
		return new network_connection(systemAddress,rakNetGUID);
	}
	virtual void DeallocConnection(Connection_RM3 *connection) const {
		delete connection;
	}
};

class network
{
    public:
        network( config *config, texture* image, block_list *block_list);
        virtual ~network();

        void init_upnp();

        void start();
        void start_sever();
        void start_client( std::string ip = "127.0.0.1");

        void sendBlockChange( Chunk *chunk, glm::vec3 pos, int id);
        void readBlockChange( BitStream *bitstream);

        void readChunk( BitStream *bitstream);
        void sendChunk( Chunk *chunk, RakNet::AddressOrGUID address);
        void sendAllChunks( world *world,RakNet::AddressOrGUID address);

        bool process();

        void draw( graphic *graphic, config *config, glm::mat4 viewmatrix);

        world *getWorld() { return p_starchip; }
        bool isServer() { return p_isServer; }
        bool isClient() { return p_isClient; }
    protected:

    private:
        RakNet::SocketDescriptor p_socketdescriptor;
        NetworkIDManager p_networkIdManager;
        RakNet::RakPeerInterface *p_rakPeerInterface;
        network_mananger p_replicaManager;

        RakNet::Packet *p_packet;
        bool p_isClient;
        bool p_isServer;

        int p_port;
        int p_maxamountplayer;
        std::string p_ip;
        world *p_starchip;
};

#endif // NETWORK_H
