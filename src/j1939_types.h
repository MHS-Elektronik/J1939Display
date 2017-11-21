#ifndef __J1939_TYPES_H__
#define __J1939_TYPES_H__

#include can_types.h

#ifdef __cplusplus
  extern "C" {
#endif

#define J1939_SA_MASK        0x000000FF
#define J1939_PGN_MASK       0x03FFFF00
#define J1939_PRIORITY_MASK  0x1C000000


#define MsgJ1939Id       J1939Id.Id
#define MsgJ1939IdSA     J1939Id.Field.SA
#define MsgJ1939IdPGN    J1939Id.Field.PGN
#define MsgJ1939Priority J1939Id.Field.Priority  


#pragma pack(push, 1)
union TJ1939Id
  {
  uint32_t Id;
  struct TJ1939IdFields Field;
  };
#pragma pack(pop) 

#pragma pack(push, 1)
struct TCanMsg
  {
  unsigned SA:8;       // Source Address
  unsigned PGN:18;     // Parameter Group Number
  unsigned Priority:3; // Priority
  };
#pragma pack(pop)

#pragma pack(push, 1)
struct TJ1939CanMsg
  {
  union TJ1939Id J1939Id;
  union TCanFlags Flags;
  union TCanData Data;
  struct TTime Time;
  };
#pragma pack(pop)


#ifdef __cplusplus
  }
#endif

#endif   