#include "Graphics.h"
#include "..\\Components\\MeshRenderComponent.h"
#include "..\\Editor\\Editor.h"
#include "..\\Input\\InputManager.h"


bool Graphics::Initialize(HWND hwnd, int width, int height, Engine* engine)
{
	windowWidth = width;
	windowHeight = height;
	m_pEngine = engine;

	fpsTimer.Start();

	if (!InitializeDirectX(hwnd))
		return false;


	if (!InitializeShaders())
		return false;


	if (!InitializeScene())
		return false;
	
	pIO = new ImGuiIO();
	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(pDevice.Get(), pDeviceContext.Get());
	*pIO = io;

	return true;
}

void Graphics::InitSkybox()
{
	skybox = new Entity((&m_pEngine->GetScene()), *(new ID()));
	MeshRenderer* me = skybox->AddComponent<MeshRenderer>();
	me->Initialize("Data\\Objects\\SkyCube.fbx", pDevice.Get(), pDeviceContext.Get(), cb_vs_vertexshader);
	skybox->GetTransform().SetPosition(0.0f, 0.0f, 0.0f);
	skybox->GetTransform().SetScale(150.0f, 150.0f, 150.0f);
	skybox->GetTransform().SetRotation(0.0f, 0.0f, 0.0f);
	m_pEngine->GetScene().AddEntity(skybox);
	
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	try
	{
		std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

		if (adapters.size() < 1)
		{
			ErrorLogger::Log("No IDXGI Adapters found.");
			return false;
		}

		DXGI_SWAP_CHAIN_DESC scd = { 0 };
		scd.BufferDesc.Width = windowWidth;
		scd.BufferDesc.Height = windowHeight;
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = 1;
		scd.OutputWindow = hwnd;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		HRESULT hr;
		hr = D3D11CreateDeviceAndSwapChain(adapters[0].pAdapter, //IDXGI Adapter
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL, // NULL for software driver type
			NULL, // Flags for runtime layers
			NULL, // Feature levels array
			0, //NUmber of Feature levels in array
			D3D11_SDK_VERSION,
			&scd, // Swapchain desciption
			pSwapchain.GetAddressOf(), // Swapchain address
			pDevice.GetAddressOf(), // Device address
			NULL, //Supported feature level
			pDeviceContext.GetAddressOf()); // Device context address
		COM_ERROR_IF_FAILED(hr, "Failed to create device swap chain.");

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		COM_ERROR_IF_FAILED(hr, "GetBuffer Failed.");

		hr = pDevice->CreateRenderTargetView(backBuffer.Get(), NULL, pRenderTargetView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create render target view.");

		// Describe out Depth/Stencil Buffer
		CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, windowWidth, windowHeight);
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		hr = pDevice->CreateTexture2D(&depthStencilDesc, NULL, pDepthStencilBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer Texture2D.");

		hr = pDevice->CreateDepthStencilView(pDepthStencilBuffer.Get(), NULL, pDepthStencilView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view.");

		pDeviceContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
		
		// Create depth stencil state
		CD3D11_DEPTH_STENCIL_DESC depthstancildesc(D3D11_DEFAULT); // Z Buffer
		depthstancildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
		hr = pDevice->CreateDepthStencilState(&depthstancildesc, pDepthStencilState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Filaed to create depth stencil state.");

		// Create and set the ViewPort
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight));
		pDeviceContext->RSSetViewports(1, &viewport);

		// Create Rasterizer State
		CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
		hr = pDevice->CreateRasterizerState(&rasterizerDesc, pRasterizerState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		D3D11_DEPTH_STENCIL_DESC dssDesc;
		ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		dssDesc.DepthEnable = true;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;


		// Create Blend State
		D3D11_BLEND_DESC blendDesc = { 0 };
		D3D11_RENDER_TARGET_BLEND_DESC rtbd = { 0 };
		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[0] = rtbd;
		hr = pDevice->CreateBlendState(&blendDesc, pBlendState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create blend state.");

		pSpriteBatch = std::make_unique<DirectX::SpriteBatch>(pDeviceContext.Get());
		pSpriteFont = std::make_unique<DirectX::SpriteFont>(pDevice.Get(), L"Data\\Fonts\\calibri.spritefont");

		// Create sampler description for sampler state
		CD3D11_SAMPLER_DESC sampleDesc(D3D11_DEFAULT);
		sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		hr = pDevice->CreateSamplerState(&sampleDesc, samplerState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");
	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}


void Graphics::RenderFrame()
{
	// -- Update Light Shader Information -- //
	cb_ps_light.data.dynamicLightColor = light.lightColor;
	cb_ps_light.data.dynamicLightStrength = light.lightStrength;
	cb_ps_light.data.dynamicLightPosition = light.GetPositionFloat3();
	cb_ps_light.data.dynamicLightAttenuation_a = light.attenuation_a;
	cb_ps_light.data.dynamicLightAttenuation_b = light.attenuation_b;
	cb_ps_light.data.dynamicLightAttenuation_c = light.attenuation_c;
	cb_ps_light.ApplyChanges();

	pDeviceContext->PSSetConstantBuffers(0, 1, cb_ps_light.GetAddressOf());

	// -- Clear Background Color for Scene -- //
	float bgcolor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	pDeviceContext->ClearRenderTargetView(pRenderTargetView.Get(), bgcolor);
	pDeviceContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	pDeviceContext->IASetInputLayout(default_vertexshader.GetInputLayout());
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->RSSetState(pRasterizerState.Get());
	pDeviceContext->OMSetDepthStencilState(pDepthStencilState.Get(), 0);
	pDeviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
	pDeviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());

	// Set Shaders to be used
	pDeviceContext->VSSetShader(default_vertexshader.GetShader(), NULL, 0);
	pDeviceContext->PSSetShader(default_pixelshader.GetShader(), NULL, 0);

	// Set PBR Shader
	//pDeviceContext->PSSetShader(PBR_pixelshader.GetShader(), NULL, 0);
	
	std::list<Entity*>* entities = m_pEngine->GetScene().GetAllEntities();
	std::list<Entity*>::iterator iter;
	for (iter = entities->begin(); iter != entities->end(); iter++)
	{
		(*iter)->Draw(camera3D.GetViewMatrix() * camera3D.GetProjectionMatrix());
	}

	// -- Update 2D shader Information -- //
	pDeviceContext->IASetInputLayout(vertexshader_2d.GetInputLayout());
	pDeviceContext->PSSetShader(pixelshader_2d.GetShader(), NULL, 0);
	pDeviceContext->VSSetShader(vertexshader_2d.GetShader(), NULL, 0);
	//sprite.Draw(camera2D.GetWorldmatrix() * camera2D.GetOrthoMatrix()); // Draws hello world sprite image

	// -- Draw Text -- //
	static int fpsCounter = 0;
	static std::string fpsString = "FPS: 0";
	fpsCounter += 1;
	if (fpsTimer.GetDeltaTime() > 1000.0)
	{
		fpsString = "FPS: " + std::to_string(fpsCounter);
		fpsCounter = 0;
		fpsTimer.Restart();
	}
	pSpriteBatch->Begin();
	pSpriteFont->DrawString(pSpriteBatch.get(), StringHelper::StringToWide(fpsString).c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
	pSpriteBatch->End();
		
	UpdateImGui();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	if (pIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// -- Flip Buffer and Present-- //
	pSwapchain->Present(1, NULL); // Enable Vertical sync with 1 or 0
}

void Graphics::Update(const float& deltaTime)
{
	
}

void Graphics::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


bool Graphics::InitializeShaders()
{
	std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
	{
#ifdef _DEBUG // Debug Mode
#ifdef _WIN64 //x64
		shaderfolder = L"..\\bin\\x64\\Debug\\";
#else // x86 (Win32)
		shaderfolder = L"..\\bin\\Win32\\Debug\\";
#endif
#else // Release Mode
#ifdef _WIN64 //x64
		shaderfolder = L"..\\bin\\x64\\Release\\";
#else //x86 (Win32)
		shaderfolder = L"..\\bin\\Win32\\Release\\";
#endif
#endif
	}
	
	// 2D shaders
	D3D11_INPUT_ELEMENT_DESC layout2D[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  }

	};
	UINT numElements2D = ARRAYSIZE(layout2D);

	if (!vertexshader_2d.Initialize(pDevice, shaderfolder + L"vertexshader_2d.cso", layout2D, numElements2D))
		return false;

	if (!pixelshader_2d.Initialize(pDevice, shaderfolder + L"pixelshader_2d.cso"))
		return false;

	// 3D shaders
	// -- Initialize Default Shaders -- //
	D3D11_INPUT_ELEMENT_DESC defaultLayout3D[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  }
	};
	UINT defaultNumElements3D = ARRAYSIZE(defaultLayout3D);


	if (!default_vertexshader.Initialize(pDevice, shaderfolder + L"vertexshader.cso", defaultLayout3D, defaultNumElements3D))
		return false;

	if (!default_pixelshader.Initialize(pDevice, shaderfolder + L"pixelshader.cso"))
		return false;

	// -- Initialize PBR Shaders -- //
	/*D3D11_INPUT_ELEMENT_DESC PBRLayout3D[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BASECOLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"OPACITY", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  }
	};
	UINT PBRNumElements3D = ARRAYSIZE(PBRLayout3D);

	if (!PBR_vertexshader.Initialize(pDevice, shaderfolder + L"PBR_vertexshader.cso", PBRLayout3D, PBRNumElements3D))
		return false;

	if (!PBR_pixelshader.Initialize(pDevice, shaderfolder + L"PBR_pixelshader.cso"))
		return false;*/

	return true;
}

bool Graphics::InitializeScene()
{
	try
	{
		// Initialize Constant Buffer(s)
		HRESULT hr = cb_vs_vertexshader_2d.Initialize(pDevice.Get(), pDeviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer 2d for vertex shader.");

		hr = cb_vs_vertexshader.Initialize(pDevice.Get(), pDeviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer for vertex shader.");
		
		hr = cb_ps_light.Initialize(pDevice.Get(), pDeviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer for pixel shader.");

		// Initialize light shader values
		cb_ps_light.data.ambientLightColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		cb_ps_light.data.ambientLightStrength = 1.0f;
		cb_ps_light.data.ambientLightStrength = 0.268f;

		light.lightStrength = 6.848f;
		light.attenuation_a = 1.968f;
		light.attenuation_b = 0.2f;
		light.attenuation_c = 0.0f;
		

		// Light
		if (!light.Initialize(pDevice.Get(), pDeviceContext.Get(), cb_vs_vertexshader))
		{
			ErrorLogger::Log("Failed to initilize light");
			return false;
		}
		// Hello World sprite
		if (!sprite.Initialize(pDevice.Get(), pDeviceContext.Get(), 256, 256, "Data\\Textures\\sprite_256x256.png", cb_vs_vertexshader_2d))
		{
			ErrorLogger::Log("Failed to initilize sprite");
			return false;
		}

		camera2D.SetProjectionValues((float)windowWidth, (float)windowHeight, 0.0f, 1.0f);

		camera3D.SetPosition(0.0f, 5.0f, -10.0f);
		camera3D.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 1000.0f);
		
	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

void Graphics::UpdateImGui()
{
	using namespace Debug;

	// Start ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	/*bool show = true;
	ImGui::ShowDemoWindow(&show);*/
	
	ImGui::Begin("Editor");
	
	if (ImGui::Button("Play", { 150.0f, 20.0f }))
	{
		Editor::Instance()->PlayGame();// Editor calles scene OnStart
	}

	if (ImGui::Button("Stop", { 150.0f, 20.0f }))
	{

		Editor::Instance()->StopGame();
	}

	ImGui::End();

	
	ImGui::Begin("Light Controls");

	ImGui::DragFloat3("Ambient Light Color", &cb_ps_light.data.ambientLightColor.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Ambient Light Strength", &cb_ps_light.data.ambientLightStrength, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3("Dynamic Light Color", &light.lightColor.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Dynamic Light Strength", &light.lightStrength, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("DynamicLight Attenuation A", &light.attenuation_a, 0.01f, 0.1f, 10.0f);
	ImGui::DragFloat("DynamicLight Attenuation B", &light.attenuation_b, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("DynamicLight Attenuation C", &light.attenuation_c, 0.01f, 0.0f, 10.0f);

	ImGui::End();

	Entity* selectedEntity = Editor::Instance()->GetSelectedEntity();
	std::string entityName = selectedEntity->GetID().GetName();
	//char buffer[30] = {};
	ImGui::Begin("Inspector");
	/*if (ImGui::InputText(entityName.c_str(), buffer, sizeof(buffer)))
	{
		selectedGameObject->SetName(buffer);
	}*/
	ImGui::Text(entityName.c_str());
	ImGui::DragFloat3("Position", &Editor::Instance()->GetSelectedEntity()->GetTransform().GetPosition().x, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat3("Rotation", &Editor::Instance()->GetSelectedEntity()->GetTransform().GetRotation().x, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat3("Scale", &Editor::Instance()->GetSelectedEntity()->GetTransform().GetScale().x, 0.1f, -100.0f, 100.0f);
	
	ImGui::End();

	// Assemble Draw Data
	ImGui::Render();
}

