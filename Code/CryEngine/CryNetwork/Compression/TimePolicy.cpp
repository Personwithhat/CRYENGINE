// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "CompressionPolicyTime.h"
#include "Protocol/Serialize.h"

#include "TimePolicy.h"

bool CTimePolicy::Load(XmlNodeRef node, const string& filename)
{
	m_stream = eTS_Network;

	if (XmlNodeRef params = node->findChild("Params"))
	{
		string stream = params->getAttr("stream");
		if (stream == "Network")
			m_stream = eTS_Network;
		else if (stream == "Pong")
			m_stream = eTS_NetworkPong;
		else if (stream == "Ping")
			m_stream = eTS_NetworkPing;
		else if (stream == "Physics")
			m_stream = eTS_Physics;
		else if (stream == "Remote")
			m_stream = eTS_RemoteTime;
		else if (stream == "PongElapsed")
			m_stream = eTS_PongElapsed;
		else
			NetWarning("Unknown time-stream '%s' found at %s:%d", stream.c_str(), filename.c_str(), params->getLine());
	}

	return true;
}

#if USE_MEMENTO_PREDICTORS
bool CTimePolicy::ReadMemento(CByteInputStream& in) const
{
	return true;
}

bool CTimePolicy::WriteMemento(CByteOutputStream& out) const
{
	return true;
}

void CTimePolicy::NoMemento() const
{
}
#endif


/*
	PERSONAL TODO PERSONAL CRYTEK:
		CTimeValue serialization here is inaccurate, only 2 digits of accuracy......

		Why is serializing floats 'fine' but accurately serializing more than 2 digits past the decimal for CTimeValue not??
		I'd presume data size constraints and so on would be the same.
*/
#if USE_ARITHSTREAM
bool CTimePolicy::ReadValue(CCommInputStream& in, CTimeValue& value, CArithModel* pModel, uint32 age) const
{
	value = pModel->ReadTime(in, m_stream);

	NetLogPacketDebug("CTimePolicy::ReadValue %" PRId64 " (%f)", (int64)value.GetMilliSeconds(), in.GetBitSize());
	return true;
}

bool CTimePolicy::WriteValue(CCommOutputStream& out, CTimeValue value, CArithModel* pModel, uint32 age) const
{
	pModel->WriteTime(out, m_stream, value);

	return true;
}
#else
bool CTimePolicy::ReadValue(CNetInputSerializeImpl* in, CTimeValue& value, uint32 age) const
{
	value = in->ReadTime(m_stream);
	NetLogPacketDebug("CTimePolicy::ReadValue %" PRId64 " (%f)", (int64)value.GetMilliSeconds(), in->GetBitSize());
	return true;
}

bool CTimePolicy::WriteValue(CNetOutputSerializeImpl* out, CTimeValue value, uint32 age) const
{
	out->WriteTime(m_stream, value);
	return true;
}
#endif

void CTimePolicy::GetMemoryStatistics(ICrySizer* pSizer) const
{
	SIZER_COMPONENT_NAME(pSizer, "CTimePolicy");
	pSizer->Add(*this);
}
#if NET_PROFILE_ENABLE
int CTimePolicy::GetBitCount(CTimeValue value)
{
	return BITCOUNT_TIME;
}
#endif

REGISTER_COMPRESSION_POLICY(CTimePolicy, "Time");
