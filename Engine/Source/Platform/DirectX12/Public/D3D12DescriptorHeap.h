#pragma once

#include <Runtime/Core.h>
#include <Runtime/Graphics/Public/GraphicsCore.h>

#include "Runtime/Graphics/Public/IDescriptorHeap.h"
#include "Platform/DirectX12/Public/Common/D3D12Utility.h"


namespace Insight
{
	namespace Graphics
	{
		namespace DX12
		{
            // This handle refers to a descriptor or a descriptor table (contiguous descriptors) that is shader visible.
            class DescriptorHandle
            {
            public:
                DescriptorHandle()
                {
                    m_CpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
                    m_GpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
                }

                /*
                // Should we allow constructing handles that might not be shader visible?
                DescriptorHandle( D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle )
                    : m_CpuHandle(CpuHandle)
                {
                    m_GpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
                }
                */

                DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle)
                    : m_CpuHandle(CpuHandle), m_GpuHandle(GpuHandle)
                {
                }

                DescriptorHandle operator+ (INT OffsetScaledByDescriptorSize) const
                {
                    DescriptorHandle ret = *this;
                    ret += OffsetScaledByDescriptorSize;
                    return ret;
                }

                void operator += (INT OffsetScaledByDescriptorSize)
                {
                    if (m_CpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
                        m_CpuHandle.ptr += OffsetScaledByDescriptorSize;
                    if (m_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
                        m_GpuHandle.ptr += OffsetScaledByDescriptorSize;
                }

                const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &m_CpuHandle; }
                operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_CpuHandle; }
                operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return m_GpuHandle; }

                size_t GetCpuPtr() const { return m_CpuHandle.ptr; }
                uint64_t GetGpuPtr() const { return m_GpuHandle.ptr; }
                bool IsNull() const { return m_CpuHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
                bool IsShaderVisible() const { return m_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }

            private:
                D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
                D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;
            };


			class INSIGHT_API D3D12DescriptorHeap : public IDescriptorHeap
			{
            public:
                D3D12DescriptorHeap(void) {}
                ~D3D12DescriptorHeap(void) { Destroy(); }

                virtual void* GetNativeHeap() override { return RCast<ID3D12DescriptorHeap*>(m_Heap.Get()); }


                virtual void Create(const EString& DebugHeapName, EResourceHeapType Type, uint32_t MaxCount) override;
                void Destroy(void) { m_Heap = nullptr; }

                bool HasAvailableSpace(uint32_t Count) const { return Count <= m_NumFreeDescriptors; }
                DescriptorHandle Alloc(uint32_t Count = 1);

                DescriptorHandle operator[] (uint32_t arrayIdx) const { return m_FirstHandle + arrayIdx * m_DescriptorSize; }

                uint32_t GetOffsetOfHandle(const DescriptorHandle& DHandle) {
                    return (uint32_t)(DHandle.GetCpuPtr() - m_FirstHandle.GetCpuPtr()) / m_DescriptorSize;
                }

                bool ValidateHandle(const DescriptorHandle& DHandle) const;

                ID3D12DescriptorHeap* GetHeapPointer() const { return m_Heap.Get(); }

                uint32_t GetDescriptorSize(void) const { return m_DescriptorSize; }

            private:

                Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
                D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
                uint32_t m_DescriptorSize;
                uint32_t m_NumFreeDescriptors;
                DescriptorHandle m_FirstHandle;
                DescriptorHandle m_NextFreeHandle;
			};
		}
	}
}
