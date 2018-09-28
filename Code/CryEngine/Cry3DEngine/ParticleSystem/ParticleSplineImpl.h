// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  Created:     04/03/2015 by Filipe amim
//  Description:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef PARTICLESPLINEIMPL_H
#define PARTICLESPLINEIMPL_H

#pragma once

namespace pfx2
{

ILINE float CParticleSpline::Interpolate(const mpfloat& timeIn) const
{
	mpfloat time = std::min(std::max(timeIn, m_keys[0].time), m_keys.back().time);

	const SplineKey* pKey = &*m_keys.begin();
	const SplineKey* pKeyEnd = &*(m_keys.end() - 2);
	while (pKey < pKeyEnd && pKey[1].time < time)
		++pKey;

	const mpfloat inTime = pKey->time;
	const mpfloat multTime = pKey->timeMult;
	const float t = BADF ((time - inTime) * multTime);
	assert(t >= 0 && t <= 1);
	const float v0 = pKey->value;
	const float v1 = pKey[1].value;
	const float c0 = pKey->coeff0;
	const float c1 = pKey->coeff1;

	const float u = 1.0f - t;
	const float tu = t * u;
	const float v = (v0 * u + v1 * t) + (c0 * u + c1 * t) * tu;

	return v;
}

#ifdef CRY_PFX2_USE_SSE
ILINE floatv CParticleSpline::Interpolate(const floatv time) const
{
	const SplineKey* __restrict pKey = &*m_keys.begin();
	const SplineKey* __restrict pEndKey = &*(m_keys.end() - 1);

	float tKey  = BADF pKey->time;
	float tMult = BADF pKey->timeMult;
	float tEnd  = BADF pEndKey->time;

	// clamp time to curve bounds
	const floatv minTime = _mm_load1_ps(&tKey);
	const floatv maxTime = _mm_load1_ps(&tEnd);
	const floatv tk = clamp(time, minTime, maxTime);

	// search key
	floatv t0 = _mm_load1_ps(&tKey);
	floatv tm = _mm_load1_ps(&tMult);
	floatv v0 = _mm_load1_ps(&pKey->value);
	floatv c0 = _mm_load1_ps(&pKey->coeff0);
	floatv c1 = _mm_load1_ps(&pKey->coeff1);
	floatv v1 = _mm_load1_ps(&pKey[1].value);

	for (pKey++; pKey < pEndKey; pKey++)
	{
		const floatv t = _mm_load1_ps(&tKey);
		const mask32v4 condMask = t < tk;
		if (!Any(condMask))
			break;
		t0 = if_else(condMask, t, t0);
		tm = if_else(condMask, _mm_load1_ps(&tMult), tm);
		v0 = if_else(condMask, _mm_load1_ps(&pKey->value), v0);
		c0 = if_else(condMask, _mm_load1_ps(&pKey->coeff0), c0);
		c1 = if_else(condMask, _mm_load1_ps(&pKey->coeff1), c1);
		v1 = if_else(condMask, _mm_load1_ps(&pKey[1].value), v1);
	}

	// calculate curve
	const floatv one = ToFloatv(1.0f);
	const floatv t = (tk - t0) * tm;
	const floatv u = one - t;
	const floatv tu = t * u;
	const floatv v = Lerp(v0, v1, t) + Lerp(c0, c1, t) * tu;

	return v;
}
#endif

ILINE void CParticleSpline::Interpolate(const mpfloat& time, ValueType& value)
{
	if (GetKeyCount() == 0)
		value[0] = 1.0f;
	else if (GetKeyCount() == 1)
		value[0] = m_keys[0].value;
	else
		value[0] = Interpolate(time);
}

ILINE float CParticleDoubleSpline::Interpolate(const mpfloat& time, float unormRand) const
{
	const float low = m_splines[0].Interpolate(time);
	const float high = m_splines[1].Interpolate(time);
	const float result = Lerp(low, high, unormRand);
	return result;
}

#ifdef CRY_PFX2_USE_SSE
ILINE floatv CParticleDoubleSpline::Interpolate(floatv time, floatv unormRand) const
{
	const floatv low = m_splines[0].Interpolate(time);
	const floatv high = m_splines[1].Interpolate(time);
	const floatv result = Lerp(low, high, unormRand);
	return result;
}
#endif

ILINE Vec3 CParticleColorSpline::Interpolate(const mpfloat& time) const
{
	return Vec3(
	  m_splines[0].Interpolate(time),
	  m_splines[1].Interpolate(time),
	  m_splines[2].Interpolate(time));
}

#ifdef CRY_PFX2_USE_SSE
ILINE Vec3v CParticleColorSpline::Interpolate(floatv time) const
{
	return Vec3v(
	  m_splines[0].Interpolate(time),
	  m_splines[1].Interpolate(time),
	  m_splines[2].Interpolate(time));
}
#endif

}

#endif
