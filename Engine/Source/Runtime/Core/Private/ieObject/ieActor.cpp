// Copyright Insight Interactive. All Rights Reserved.
#include <Engine_pch.h>

#include "Runtime/Core/Public/ieObject/ieActor.h"
#include "Runtime/Core/Public/ieObject/ieWorld.h"

namespace Insight
{
	ieActor::ieActor(ieWorld* pWorld, const FString& Name)
		: ieObjectBase(pWorld)
		, m_bCanReveiveTickEvents(true)
	{
		pWorld->AttachBegiPlayListener(IE_BIND_LOCAL_VOID_FN(ieActor::BeginPlay));
		pWorld->AttachTickListener(IE_BIND_LOCAL_EVENT_FN(ieActor::Tick));
	}
}
