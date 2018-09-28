// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Description: Implements a seat action which handle the vehicle movement

   -------------------------------------------------------------------------
   History:
   - 20:10:2006: Created by Mathieu Pinard

*************************************************************************/
#include "StdAfx.h"
#include "CryAction.h"
#include "IVehicleSystem.h"
#include "Vehicle.h"
#include "VehicleSeat.h"
#include "VehicleSeatActionMovement.h"

//------------------------------------------------------------------------
CVehicleSeatActionMovement::CVehicleSeatActionMovement()
	: m_delayedStop(0),
	m_actionForward(0.0f),
	m_pVehicle(NULL),
	m_pSeat(NULL)
{
}

//------------------------------------------------------------------------
CVehicleSeatActionMovement::~CVehicleSeatActionMovement()
{
}

//------------------------------------------------------------------------
bool CVehicleSeatActionMovement::Init(IVehicle* pVehicle, IVehicleSeat* pSeat, const CVehicleParams& table)
{
	return Init(pVehicle, pSeat);
}

//------------------------------------------------------------------------
bool CVehicleSeatActionMovement::Init(IVehicle* pVehicle, IVehicleSeat* pSeat)
{
	IVehicleMovement* pMovement = pVehicle->GetMovement();
	if (!pMovement)
		return false;

	m_pVehicle = pVehicle;
	m_pSeat = pSeat;

	return true;
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::Reset()
{
	CRY_ASSERT(m_pVehicle);

	m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_NoUpdate);

	m_actionForward = 0.0f;
	m_delayedStop.SetSeconds(0);

	if (IVehicleMovement* pMovement = m_pVehicle->GetMovement())
		pMovement->Reset();
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::StartUsing(EntityId passengerId)
{
	IVehicleMovement* pMovement = m_pVehicle->GetMovement();
	CRY_ASSERT(pMovement);

	if (!pMovement)
		return;

	if (m_delayedStop >= 0)
	{
		m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_NoUpdate);
		m_delayedStop.SetSeconds(0);

		pMovement->StopDriving();
	}

	pMovement->StartDriving(passengerId);
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::StopUsing()
{
	IActorSystem* pActorSystem = CCryAction::GetCryAction()->GetIActorSystem();
	CRY_ASSERT(pActorSystem);

	IVehicleMovement* pMovement = m_pVehicle->GetMovement();
	if (!pMovement)
		return;

	CRY_ASSERT(m_pSeat);

	// default to continuing for a bit
	m_delayedStop.SetSeconds("0.8");

	IActor* pActor = pActorSystem->GetActor(m_pSeat->GetPassenger());

	if (pActor && pActor->IsPlayer())
	{
		// if stopped already don't go anywhere
		IPhysicalEntity* pPhys = m_pVehicle->GetEntity()->GetPhysics();
		pe_status_dynamics status;
		if (pPhys && pPhys->GetStatus(&status))
		{
			if (status.v.GetLengthSquared() < 25.0f)
				m_delayedStop.SetSeconds(0);
		}

		if (m_actionForward > 0.0f)
			m_delayedStop.SetSeconds("1.5");

		if (pMovement->GetMovementType() == IVehicleMovement::eVMT_Air)
			m_delayedStop *= 2;

		m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_AlwaysUpdate);

		// prevent full pedal being kept pressed, but give it a bit
		pMovement->OnAction(eVAI_MoveForward, eAAM_OnPress, 0.1f);
	}
	else
	{
		if (pMovement->GetMovementType() == IVehicleMovement::eVMT_Air)
		{
			m_delayedStop.SetSeconds(0);
			m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_AlwaysUpdate);
		}
		else
		{
			pMovement->StopDriving();
		}
	}
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::OnAction(const TVehicleActionId actionId, int activationMode, float value)
{
	if (actionId == eVAI_MoveForward)
		m_actionForward = value;
	else if (actionId == eVAI_MoveBack)
		m_actionForward = -value;

	IVehicleMovement* pMovement = m_pVehicle->GetMovement();
	CRY_ASSERT(pMovement);

	pMovement->OnAction(actionId, activationMode, value);
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::Update(const CTimeValue& deltaTime)
{
	IVehicleMovement* pMovement = m_pVehicle->GetMovement();
	CRY_ASSERT(pMovement);

	bool isReadyToStopEngine = true;

	if (isReadyToStopEngine)
	{
		if (m_delayedStop > 0)
			m_delayedStop -= deltaTime;

		if (m_delayedStop <= 0)
		{
			m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_NoUpdate);
			pMovement->StopDriving();
		}
	}
}

DEFINE_VEHICLEOBJECT(CVehicleSeatActionMovement);
