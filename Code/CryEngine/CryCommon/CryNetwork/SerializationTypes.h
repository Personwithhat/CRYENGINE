// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

SERIALIZATION_TYPE(bool)
SERIALIZATION_TYPE(float)
SERIALIZATION_TYPE(Vec2)
SERIALIZATION_TYPE(Vec3)
SERIALIZATION_TYPE(Quat)
SERIALIZATION_TYPE(Ang3)
SERIALIZATION_TYPE(int8)
SERIALIZATION_TYPE(int16)
SERIALIZATION_TYPE(int32)
SERIALIZATION_TYPE(int64)
SERIALIZATION_TYPE(uint8)
SERIALIZATION_TYPE(uint16)
SERIALIZATION_TYPE(uint32)
SERIALIZATION_TYPE(uint64)
SERIALIZATION_TYPE(ScriptAnyValue) // not for network - only for save games
SERIALIZATION_TYPE(SNetObjectID)
SERIALIZATION_TYPE(XmlNodeRef) // not for network - only for save games

// POINT OF INTEREST: Serialization type is just like mpfloat type, but the chain works! :O
SERIALIZATION_TYPE(CTimeValue)
#define MP_FUNCTION(T) SERIALIZATION_TYPE(T)
#include "../CrySystem/mpfloat.types"
#undef MP_FUNCTION