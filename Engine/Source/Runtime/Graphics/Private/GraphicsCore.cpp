#include <Engine_pch.h>

#include <Runtime/Graphics/Public/GraphicsCore.h>

namespace Insight
{
	namespace Graphics
	{
		// -----------------------------
		//	Extern Variable Definitions
		// -----------------------------
		//
		ICommandManager* g_pCommandManager = NULL;
		IContextManager* g_pContextManager = NULL;
		IDevice* g_pDevice = NULL;
		IGeometryBufferManager* g_pGeometryManager = NULL;
		IConstantBufferManager* g_pConstantBufferManager = NULL;
	}
}
