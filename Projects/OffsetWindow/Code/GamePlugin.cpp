#include "StdAfx.h"
#include "GamePlugin.h"

#include "Components/Player.h"

#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/EnvPackage.h>
#include <CrySchematyc/Utils/SharedString.h>

#include <CryRenderer\IRenderAuxGeom.h>

#include <IGameObjectSystem.h>
#include <IGameObject.h>

// For game context management.
#include <GameSession\GameSessionHandler.h>
#include <GameRulesSystem.h>

// For server console text
#include <CrySystem\ITextModeConsole.h>

// Included only once per DLL module.
#include <CryCore/Platform/platform_impl.inl>

bool g_WasSpawned = false;

// Define if you're running local-only instance, e.g. no servers (treat self as server)
//#define LOCAL_ONLY true

CGamePlugin::~CGamePlugin()
{
	// Remove any registered listeners before 'this' becomes invalid
	if (gEnv->pGameFramework != nullptr)
	{
		gEnv->pGameFramework->RemoveNetworkedClientListener(*this);
	}

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);
}

bool CGamePlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
{
	// Register for engine system events, in our case we need ESYSTEM_EVENT_GAME_POST_INIT to load the map
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CGamePlugin");
	
	// Listen to main update calls.
	EnableUpdate(EUpdateStep::MainUpdate, true);

	return true;
}

void CGamePlugin::MainUpdate(const CTimeValue& frameTime){
	if(!gEnv->IsDedicated() && !g_WasSpawned){ return; }

	// NOTE: This window project as a whole assumes this is all on Windows for now. Can be updated later.

	// Get monitor resolution
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfo(MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);
	const int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	const int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	// Get window width resolution
	HWND m_hWnd = gEnv->IsDedicated() ? GetConsoleWindow(): (HWND)gEnv->pRenderer->GetHWND();
	RECT wndrect;
	GetWindowRect(m_hWnd, &wndrect);
	int width = wndrect.right - wndrect.left;
	int height = wndrect.bottom - wndrect.top;

	int offY = wndrect.top  - ((monitorHeight - height) / 2 + gEnv->pConsole->GetCVar("r_offset_y")->GetIVal());
	int offX = wndrect.left - ((monitorWidth - width) / 2   + gEnv->pConsole->GetCVar("r_offset_x")->GetIVal());

	// Draw debug window offsets
	char debugStr[256];
	cry_sprintf(debugStr, " X: %d Y: %d ", offX, offY); // Spaces are to avoid trash #'s/etc. at end/start!!

	if(!gEnv->IsDedicated())
		IRenderAuxText::Draw2dLabel((float)width / 2, (float)height / 2, 5.0f, ColorF(220, 20, 60), true, "%s", debugStr);
	else{
		// NOTE: This will override all existing buffers on the console window. 
		
		// PERSONAL TODO:
		// There really should be a better way to manage this. E.G. 'NewBuffer()' and so on to add/remove buffers for debug purposes.
		// Sometimes a lot easier than just logging it in the back, and for server debug really useful.
		ITextModeConsole* pTextModeConsole = gEnv->pSystem->GetITextModeConsole();
		pTextModeConsole->PutText(0, 0, debugStr);
	}
};

void CGamePlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
		// Called when the game framework has initialized and we are ready for game logic to start
		case ESYSTEM_EVENT_GAME_POST_INIT:
		{
			// Listen for client connection events, in order to create the local player
			gEnv->pGameFramework->AddNetworkedClientListener(*this);

			// Don't need to load the map in editor
			if (gEnv->IsEditor())
			{
				assert(0 && "Shouldn't be run in editor");
			}else{
				// NOTE: 
				// Works overall and loads the project WITHOUT needing a bogus level etc.
				// With some engine edits to 'skip' level loading if no-level-loading flag is set and no level name is defined

				auto pAction = gEnv->pGameFramework;
				CRY_ASSERT(!pAction->StartedGameContext());
				SGameContextParams ctx;
				SGameStartParams params;

				// Don't load game rules if ya don't have any.
				if (!pAction->GetIGameRulesSystem()->GetCurrentGameRules())
				{
					params.flags |= (eGSF_NoSpawnPlayer | eGSF_NoGameRules);
				}

				// For singleplayer with no server. E.g. treat self as server.
			#ifdef LOCAL_ONLY 				
				assert(!gEnv->IsDedicated());

				params.flags |= eGSF_NonBlockingConnect | eGSF_Server | eGSF_Client | eGSF_LocalOnly | eGSF_NoLevelLoading;
				params.hostname = "localhost";
				params.maxPlayers = 1;

				params.pContextParams = &ctx;
				params.port = gEnv->pConsole->GetCVar("sv_port")->GetIVal();
				params.session = pAction->GetIGameSessionHandler()->GetGameSessionHandle();

				pAction->StartGameContext(&params);
			#else
				// For a multiplayer setup with X clients and 1 server.
				if (!gEnv->IsDedicated()) {
					params.flags |= eGSF_Client | eGSF_NoLevelLoading;
					params.hostname = gEnv->pConsole->GetCVar("cl_serveraddr")->GetString();
					params.pContextParams = NULL;
					params.port = (gEnv->pLobby && gEnv->bMultiplayer) ? gEnv->pLobby->GetLobbyParameters().m_connectPort : gEnv->pConsole->GetCVar("cl_serverport")->GetIVal();
					pAction->StartGameContext(&params);
				}else{
					ICVar* max_players = gEnv->pConsole->GetCVar("sv_maxplayers");

					params.flags |= eGSF_Server | eGSF_NoLevelLoading | eGSF_ImmersiveMultiplayer;
					params.maxPlayers = max_players ? max_players->GetIVal() : 16;
					params.pContextParams = &ctx;
					params.port = gEnv->pConsole->GetCVar("sv_port")->GetIVal();
					params.session = pAction->GetIGameSessionHandler()->GetGameSessionHandle();

					pAction->StartGameContext(&params);
				}
			#endif
			}
		}
		break;

		case ESYSTEM_EVENT_REGISTER_SCHEMATYC_ENV:
		{
			// Register all components that belong to this plug-in
			auto staticAutoRegisterLambda = [](Schematyc::IEnvRegistrar& registrar)
			{
				// Call all static callback registered with the CRY_STATIC_AUTO_REGISTER_WITH_PARAM
				Detail::CStaticAutoRegistrar<Schematyc::IEnvRegistrar&>::InvokeStaticCallbacks(registrar);
			};

			if (gEnv->pSchematyc)
			{
				gEnv->pSchematyc->GetEnvRegistry().RegisterPackage(
					stl::make_unique<Schematyc::CEnvPackage>(
						CGamePlugin::GetCID(),
						"EntityComponents",
						"Crytek GmbH",
						"Components",
						staticAutoRegisterLambda
						)
				);
			}
		}
		break;
		case ESYSTEM_EVENT_FULL_SHUTDOWN:
		case ESYSTEM_EVENT_FAST_SHUTDOWN:
		{
			// Deregister the Schematyc packages which were registered in the entity system.
			if (gEnv->pSchematyc)
			{
				gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage(CGamePlugin::GetCID());
			}
		}
		break;
	}
}

bool CGamePlugin::OnClientConnectionReceived(int channelId, bool bIsReset)
{
	// Connection received from a client, create a player entity and component
	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
	spawnParams.sName = "Player";
	
	// Set local player details
	if (m_players.size() == 0 && !gEnv->IsDedicated())
	{
		spawnParams.id = LOCAL_PLAYER_ENTITY_ID;
		spawnParams.nFlags |= ENTITY_FLAG_LOCAL_PLAYER;
	}

	// Spawn the player entity
	IEntity* pPlayerEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams);

	// Set the local player entity channel id, and bind it to the network so that it can support Multiplayer contexts
	pPlayerEntity->GetNetEntity()->SetChannelId(channelId);
	pPlayerEntity->GetNetEntity()->BindToNetwork();

	// Create the player component instance
	pPlayerEntity->CreateComponentClass<CPlayerComponent>();

	// Push the component into our map, with the channel id as the key
	m_players.emplace(std::make_pair(channelId, pPlayerEntity->GetId()));

	return true;
}

bool CGamePlugin::OnClientReadyForGameplay(int channelId, bool bIsReset)
{
	// Revive players when the network reports that the client is connected and ready for gameplay
	auto it = m_players.find(channelId);
	if (it != m_players.end())
	{
		if (IEntity* pPlayerEntity = gEnv->pEntitySystem->GetEntity(it->second))
		{
			if (CPlayerComponent* pPlayer = pPlayerEntity->GetComponent<CPlayerComponent>())
			{
				pPlayer->Revive();
			}
		}
	}

	return true;
}

void CGamePlugin::OnClientDisconnected(int channelId, EDisconnectionCause cause, const char* description, bool bKeepClient)
{
	// Client disconnected, remove the entity and from map
	auto it = m_players.find(channelId);
	if (it != m_players.end())
	{
		gEnv->pEntitySystem->RemoveEntity(it->second);

		m_players.erase(it);
	}
}

CRYREGISTER_SINGLETON_CLASS(CGamePlugin)