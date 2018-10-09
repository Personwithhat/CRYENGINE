#include "StdAfx.h"
#include "Player.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/CoreAPI.h>
#include <CryCore/StaticInstanceList.h>

#include "GamePlugin.h"

// Required for networking to work. Gotta register this entity!
namespace {
	static void RegisterComponentPlayer(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CPlayerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterComponentPlayer);
}

void CPlayerComponent::Initialize()
{
	// Get the input component, wraps access to action mapping so we can easily get callbacks when inputs are triggered
	m_pInputComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CInputComponent>();

	// Register an action, and the callback that will be sent when it's triggered
	m_pInputComponent->RegisterAction("player", "moveleft", [this](int activationMode, float value)    { HandleInputFlagChange((TInputFlags)EInputFlag::MoveLeft, activationMode);  });
	m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, EKeyId::eKI_A); 	// Bind the 'A' key the "moveleft" action

	m_pInputComponent->RegisterAction("player", "moveright", [this](int activationMode, float value)   { HandleInputFlagChange((TInputFlags)EInputFlag::MoveRight, activationMode);  });
	m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, EKeyId::eKI_D);

	m_pInputComponent->RegisterAction("player", "moveforward", [this](int activationMode, float value) { HandleInputFlagChange((TInputFlags)EInputFlag::MoveForward, activationMode);  });
	m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, EKeyId::eKI_W);

	m_pInputComponent->RegisterAction("player", "moveback", [this](int activationMode, float value)    { HandleInputFlagChange((TInputFlags)EInputFlag::MoveBack, activationMode);  });
	m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, EKeyId::eKI_S);

	Revive();
	
	// For networking to work.
	m_pEntity->GetNetEntity()->BindToNetwork();

	g_WasSpawned = true;
}

Cry::Entity::EventFlags CPlayerComponent::GetEventMask() const
{
	return ENTITY_EVENT_START_GAME | ENTITY_EVENT_UPDATE;
}

void CPlayerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_START_GAME:
	{
		// Revive the entity when gameplay starts
		Revive();
	}
	break;
	case ENTITY_EVENT_UPDATE:
	{
		SEntityUpdateContext* pCtx = (SEntityUpdateContext*)event.nParam[0];

		// Update window positoining. 
		UpdateMovementRequest(pCtx->fFrameTime);
	}
	break;
	}
}

void CPlayerComponent::UpdateMovementRequest(float frameTime)
{
	static Vec3 offset = ZERO;

	// Movement speed in pixels 
	const float moveSpeed = 50;

	if (m_inputFlags & (TInputFlags)EInputFlag::MoveLeft)
	{
		offset.x -= moveSpeed * frameTime;
	}
	if (m_inputFlags & (TInputFlags)EInputFlag::MoveRight)
	{
		offset.x += moveSpeed * frameTime;
	}
	if (m_inputFlags & (TInputFlags)EInputFlag::MoveForward)
	{
		offset.y -= moveSpeed * frameTime;
	}
	if (m_inputFlags & (TInputFlags)EInputFlag::MoveBack)
	{
		offset.y += moveSpeed * frameTime;
	}

	// Ignore on server ofc.
	if (gEnv->IsDedicated()) { return; }

	// Get monitor resolution
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfo(MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);
	int x = monitorInfo.rcMonitor.left;
	int y = monitorInfo.rcMonitor.top;
	const int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	const int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	// Get window width resolution
	HWND m_hWnd = (HWND)gEnv->pRenderer->GetHWND();
	RECT wndrect;
	GetWindowRect(m_hWnd, &wndrect);
	int width  = wndrect.right - wndrect.left;
	int height = wndrect.bottom - wndrect.top;

	// Set window position, starting centered then applying offsets.
	x += (monitorWidth - width) / 2   + gEnv->pConsole->GetCVar("r_offset_x")->GetIVal() + (int)offset.x;
	y += (monitorHeight - height) / 2 + gEnv->pConsole->GetCVar("r_offset_y")->GetIVal() + (int)offset.y;
	SetWindowPos(m_hWnd, HWND_NOTOPMOST, x, y, width, height, SWP_SHOWWINDOW);
}

void CPlayerComponent::Revive()
{
	// Reset input now that the player respawned
	m_inputFlags = 0;
}

void CPlayerComponent::HandleInputFlagChange(TInputFlags flags, int activationMode, EInputFlagType type)
{
	switch (type)
	{
	case EInputFlagType::Hold:
	{
		if (activationMode == eIS_Released)
		{
			m_inputFlags &= ~flags;
		}
		else
		{
			m_inputFlags |= flags;
		}
	}
	break;
	case EInputFlagType::Toggle:
	{
		if (activationMode == eIS_Released)
		{
			// Toggle the bit(s)
			m_inputFlags ^= flags;
		}
	}
	break;
	}
}