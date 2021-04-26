#pragma once

#include "Runtime/Core.h"

#include "Runtime/Graphics/Public/IGeometryBufferManager.h"

#include "Platform/DirectX12/Public/Resource/D3D12VertexBuffer.h"
#include "Platform/DirectX12/Public/Resource/D3D12IndexBuffer.h"
#include "Runtime/Graphics/Public/GraphicsCore.h"
#include "Runtime/Graphics/Private/ICommandManager.h" // TEMP

namespace Insight
{
	namespace Graphics
	{
		namespace DX12
		{
			class INSIGHT_API D3D12GeometryBufferManager : public IGeometryBufferManager
			{
			public:
				D3D12GeometryBufferManager() {}
				virtual ~D3D12GeometryBufferManager() {}
				
				virtual VertexBufferUID AllocateVertexBuffer() override
				{
					VertexBufferUID NewUID = s_NextVertexBufferID++;

					auto InsertResult = m_VertexBufferLUT.try_emplace(NewUID, D3D12VertexBuffer{});
					IE_ASSERT(InsertResult.second == true); // Trying to create a vertex buffer with an already existing ID! This is not allowed.
					
					m_VertexBufferLUT[NewUID].SetUID(NewUID);

					return NewUID;
				}

				virtual IndexBufferUID AllocateIndexBuffer() override
				{
					IndexBufferUID NewUID = s_NextIndexBufferID++;
					auto InsertResult = m_IndexBufferLUT.try_emplace(NewUID, D3D12IndexBuffer{});
					IE_ASSERT(InsertResult.second == true); // Trying to create a index buffer with an already existing ID! This is not allowed.

					m_IndexBufferLUT[NewUID].SetUID(NewUID);

					return NewUID;
				}

				// TODO Fix multiple file names so these can go inside this class's cpp file.

				FORCE_INLINE virtual void DeAllocateVertexBuffer(VertexBufferUID& UID) override
				{
					g_pCommandManager->IdleGPU();

					IE_ASSERT(UID != IE_INVALID_VERTEX_BUFFER_HANDLE);
					m_VertexBufferLUT.erase(UID);
				}

				FORCE_INLINE virtual void DeAllocateIndexBuffer(IndexBufferUID& UID) override
				{
					g_pCommandManager->IdleGPU();
					
					IE_ASSERT(UID != IE_INVALID_INDEX_BUFFER_HANDLE);
					m_IndexBufferLUT.erase(UID);
				}

				FORCE_INLINE virtual IVertexBuffer& GetVertexBufferByUID(VertexBufferUID& UID) override
				{
					IE_ASSERT(UID != IE_INVALID_VERTEX_BUFFER_HANDLE);
					return m_VertexBufferLUT.at(UID);
				}

				FORCE_INLINE virtual IIndexBuffer& GetIndexBufferByUID(IndexBufferUID& UID) override
				{
					IE_ASSERT(UID != IE_INVALID_INDEX_BUFFER_HANDLE);
					return m_IndexBufferLUT.at(UID);
				}


			protected:
				std::unordered_map<VertexBufferUID, D3D12VertexBuffer> m_VertexBufferLUT;
				std::unordered_map<IndexBufferUID, D3D12IndexBuffer> m_IndexBufferLUT;


			};
		}
	}
}
