// Copyright 2001-2017 Crytek GmbH / Crytek Group. All rights reserved. 

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Description:  Helper classes for authentication
   -------------------------------------------------------------------------
   History:
   - 26/07/2004   : Created by Craig Tiller
*************************************************************************/
#include "StdAfx.h"
#include "Authentication.h"
#include <CrySystem/ITimer.h>

ILINE static uint32 GetRandomNumber()
{
	NetWarning("Using low quality random numbers: security compromised");
	return cry_random_uint32();
}

SAuthenticationSalt::SAuthenticationSalt() :
	fTime(gEnv->pTimer->GetFrameStartTime()),
	nRand(GetRandomNumber())
{
}

void SAuthenticationSalt::SerializeWith(TSerialize ser)
{
	ser.Value("fTime", fTime);
	ser.Value("nRand", nRand);
}

CWhirlpoolHash SAuthenticationSalt::Hash(const string& password) const
{
	//char n1[32];
	char n2[32];
	const char* n1 = fTime.GetSeconds().str(); // PERSONAL VERIFY: Will this hashing/etc. work properly?
	cry_sprintf(n2, "%.8x", nRand);
	string buffer = password + ":" + n1 + ":" + n2;
	return CWhirlpoolHash(buffer);
}
