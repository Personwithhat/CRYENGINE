// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryCore/Project/CryModuleDefs.h>
#define eCryModule eCryM_EnginePlugin
#define GAME_API   DLL_EXPORT

#include <CryCore/Platform/platform.h>
#include <CrySystem/ISystem.h>
#include <Cry3DEngine/I3DEngine.h>
#include <CryNetwork/ISerialize.h>
#include <CryEntitySystem/IEntitySystem.h>

//template<class T>
static IEntityClass* RegisterEntityClass(const char* name, const char* editorCategory = "", const char* editorIcon = "", bool bIconOnTop = false)
{
	IEntityClassRegistry::SEntityClassDesc clsDesc;
	clsDesc.sName = name;

	clsDesc.editorClassInfo.sCategory = editorCategory;
	clsDesc.editorClassInfo.sIcon = editorIcon;
	clsDesc.editorClassInfo.bIconOnTop = bIconOnTop;

	/*struct CObjectCreator
	{
		static IEntityComponent* Create(IEntity* pEntity, SEntitySpawnParams& params, void* pUserData)
		{
			return pEntity->GetOrCreateComponentClass<T>();
		}
	};

	clsDesc.pUserProxyCreateFunc = &CObjectCreator::Create;*/
	clsDesc.flags |= ECLF_INVISIBLE;

	return gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(clsDesc);
}
