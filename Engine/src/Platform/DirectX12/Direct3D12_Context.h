#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/RenderingContext.h"
#include "Platform/DirectX12/D3D_Api.h"

#include "Insight/Game/Camera.h"

// TODO: implement shader system that uses this
#include "ConstantBuffersPerObject_TEMP.h"
#include <DirectXMath.h>
// TODO: implement texture system that uses this
#include <wincodec.h>


namespace Insight {

	class WindowsWindow;

	class Direct3D12Context : public RenderingContext, public D3DApi
	{
	public:
		Direct3D12Context(WindowsWindow* windowHandle);
		virtual ~Direct3D12Context();
		
		virtual void OnUpdate() override;

		virtual bool Init() override;
		virtual void RenderFrame() override;
		virtual void SwapBuffers() override;
		virtual void OnWindowResize() override;

		inline ID3D12Device& GetDeviceContext() const { return *m_pLogicalDevice.Get(); }
		inline ID3D12DescriptorHeap& GetImGuiDescriptorHeap() const { return *m_pImGuiDescriptorHeap.Get(); }
		inline ID3D12GraphicsCommandList& GetCommandList() const { return *m_pCommandList.Get(); }

	private:
		// Per-Frame
		void PopulateCommandLists();
		void WaitForPreviousFrame();

		// D3D12 Initialize
		void CreateDXGIFactory();
		void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
		void CreateDevice();
		void CreateCommandQueue();
		void CreateSwapChain();
		void CreateDescriptorHeaps();
		void CreateRTVDescriptorHeap();
		void CreateDSVDescriptorHeap();
		void CreateDepthStencilBuffer();
		void CreateCommandAllocators();
		void CreateFenceEvent();
		void CreatePipelineStateObjects();
		void CrateConstantBufferResourceHeaps();
		void CreateViewport();
		void CreateScissorRect();
		void CloseCommandListAndSignalCommandQueue();
		void CreateImGuiDescriptorHeap();

		// TEMP! Move this!
		void InitShaders();
		DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
		WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
		int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
		int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);
		void InitDemoScene();
		//ConstantBuffer<ConstantBufferPerObject> cb_vertexShader;// TEMP! Move this!
		void LoadAssets();
		void LoadModels();
		void LoadTextures();
		//----------------

		void Cleanup();
		void WaitForGPU();
		void ToggleFullscreen(IDXGISwapChain* pSwapChain);
	private:
		HWND* m_pWindowHandle = nullptr;
		WindowsWindow* m_pWindow = nullptr;

		// Sync Values
		int m_FrameIndex = 0;
		int m_RtvDescriptorSize = 0;
		UINT64 m_FenceValue[m_FrameBufferCount] = {};
		HANDLE m_FenceEvent = {};

		bool m_RayTraceEnabled = false;
		bool m_UseWarpDevice = false;

		WRL::ComPtr<IDXGIAdapter1>				m_pPhysicalDevice;
		WRL::ComPtr<ID3D12Device5>				m_pLogicalDevice;
		WRL::ComPtr<IDXGIFactory4>				m_pDxgiFactory;
		WRL::ComPtr<IDXGISwapChain3>			m_pSwapChain;

		WRL::ComPtr<ID3D12CommandQueue>			m_pCommandQueue;
		WRL::ComPtr<ID3D12DescriptorHeap>		m_pRtvDescriptorHeap;
		WRL::ComPtr<ID3D12Resource>				m_pRenderTargets[m_FrameBufferCount];
		WRL::ComPtr<ID3D12CommandAllocator>		m_pCommandAllocators[m_FrameBufferCount];

		WRL::ComPtr<ID3D12Fence>				m_pFence[m_FrameBufferCount];
		WRL::ComPtr<ID3D12GraphicsCommandList>	m_pCommandList;

		WRL::ComPtr<ID3D12Resource>				m_pDepthStencilBuffer;
		WRL::ComPtr<ID3D12DescriptorHeap>		m_pDepthStencilDescriptorHeap;

		WRL::ComPtr<ID3D12PipelineState>		m_pPipelineStateObject_Default;
		WRL::ComPtr<ID3D12RootSignature>		m_pRootSignature_Default;

		D3D12_VIEWPORT							m_ViewPort = {};
		D3D12_RECT								m_ScissorRect = {};
		DXGI_SAMPLE_DESC						m_SampleDesc = {};

		// IMGUI this should move
		WRL::ComPtr <ID3D12DescriptorHeap>		m_pImGuiDescriptorHeap;


		//=== TEMPORARY! ===//
		//TODO: Move this to a model/vertex class
		WRL::ComPtr<ID3D12Resource> m_pVertexBuffer;
		WRL::ComPtr<ID3D12Resource> m_pVBufferUploadHeap;

		WRL::ComPtr<ID3D12Resource> m_pIndexBuffer;
		WRL::ComPtr<ID3D12Resource> m_pIndexBufferUploadHeap;
		D3D12_INDEX_BUFFER_VIEW		m_IndexBufferView = {};

		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};
		struct Vertex {
			Vertex(float x, float y, float z, float u, float v)
				: pos(x, y, z), texCoord(u, v) {}
			DirectX::XMFLOAT3 pos = {};
			DirectX::XMFLOAT2 texCoord = {};
		};

		struct ConstantBufferPerObject {
			DirectX::XMFLOAT4X4 wvpMatrix;
		};
		int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;
		ConstantBufferPerObject cbPerObject;
		WRL::ComPtr<ID3D12Resource> constantBufferUploadHeaps[m_FrameBufferCount];

		UINT8* cbvGPUAddress[m_FrameBufferCount]; 
		Camera camera;

		DirectX::XMFLOAT4X4 cube1WorldMat;
		DirectX::XMFLOAT4X4 cube1RotMat;
		DirectX::XMFLOAT4 cube1Position;

		DirectX::XMFLOAT4X4 cube2WorldMat;
		DirectX::XMFLOAT4X4 cube2RotMat;
		DirectX::XMFLOAT4 cube2PositionOffset;

		DirectX::XMFLOAT4X4 cube3WorldMat;
		DirectX::XMFLOAT4X4 cube3RotMat;
		DirectX::XMFLOAT4 cube3PositionOffset;

		int numCubeIndices; 

		// TEMP Textures
		WRL::ComPtr<ID3D12Resource>		  m_pTextureBuffer;
		WRL::ComPtr<ID3D12DescriptorHeap> m_pMainDescriptorHeap;
		WRL::ComPtr<ID3D12Resource>		  m_pTextureBufferUploadHeap;


	};

}
