#ifndef NETWORK_IDS_H
#define NETWORK_IDS_H

#include "BitStream.h"
#include "MessageIdentifiers.h"

enum {
ID_SET_TIMED_MINE = ID_USER_PACKET_ENUM,
ID_SET_BLOCK,
ID_SET_CHUNK,
ID_DEL_CHUNK
};

#endif // NETWORK_IDS_H
