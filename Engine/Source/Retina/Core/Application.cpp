#include <Engine_pch.h>

#include "Application.h"

#include "Retina/Input/Input.h"
#include "Retina/Runtime/AActor.h"
#include "Retina/Layer_Types/ImGui_Layer.h"
#include "Retina/Core/rn_Exception.h"
#include "Renderer/Renderer.h"

#if defined RN_PLATFORM_WINDOWS
#include "Renderer/Platform/Windows/DirectX_11/Wrappers/D3D11_ImGui_Layer.h"
#include "Renderer/Platform/Windows/DirectX_12/Wrappers/D3D12_ImGui_Layer.h"
#include "Platform/Windows/Windows_Window.h"
#endif

// TODO: Make the project hot reloadable
// Scenes (Development-Project)
// ----------------------------
// DemoScene
// MultipleLights
static const char* ProjectName = "Development-Project";
static const char* TargetSceneName = "Norway.iescene";

namespace Retina {

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		RN_ASSERT(!s_Instance, "Trying to create Application instance when one already exists!");
		s_Instance = this;
	}

	bool Application::InitializeAppForWindows(HINSTANCE & hInstance, int nCmdShow)
	{
		m_pWindow = std::unique_ptr<Window>(Window::Create());
		m_pWindow->SetEventCallback(RN_BIND_EVENT_FN(Application::OnEvent));

		WindowsWindow* pWindow = (WindowsWindow*)m_pWindow.get();
		pWindow->SetWindowsSessionProps(hInstance, nCmdShow);
		if (!pWindow->Init(WindowProps())) {
			RN_CORE_FATAL(L"Fatal Error: Failed to initialize window.");
			return false;
		}

		if (!InitializeCoreApplication()) {
			RN_CORE_FATAL(L"Fatal Error: Failed to initiazlize application for Windows.");
			return false;
		}

		return true;
	}

	Application::~Application()
	{
		Shutdown();
	}

	bool Application::InitializeCoreApplication()
	{
		// Initize the main file system
		FileSystem::Init(ProjectName);

		// Create and initialize the renderer
		Renderer::SetSettingsAndCreateContext(FileSystem::LoadGraphicsSettingsFromJson());
		Renderer::Init();

		// Create the main game layer
		m_pGameLayer = new GameLayer();

		// Load the Scene
		std::string DocumentPath(FileSystem::GetProjectDirectory());
		DocumentPath += "/Assets/Scenes/";
		DocumentPath += TargetSceneName;
		if (!m_pGameLayer->LoadScene(DocumentPath)) {
			throw ieException("Failed to initialize scene");
		}

		// Push core app layers to the layer stack
		PushEngineLayers();

		return true;
	}

	void Application::PostInit()
	{
		Renderer::PostInit();

		m_pWindow->PostInit();

		ResourceManager::Get().PostAppInit();
		RN_CORE_TRACE("Application Initialized");

		m_pGameLayer->PostInit();

		m_AppInitialized = true;
	}

	static void RenderThread()
	{
		bool IsAppRunning = Application::Get().IsApplicationRunning();
		FrameTimer GraphicsTimer;
		while (IsAppRunning)
		{
			GraphicsTimer.Tick();
			float DeltaMs = GraphicsTimer.DeltaTime();

			Renderer::OnUpdate(DeltaMs);
			Renderer::OnPreFrameRender();
			//GeometryManager::GatherGeometry();
			Renderer::OnRender();
			Renderer::OnMidFrameRender();
			Renderer::ExecuteDraw();
			Renderer::SwapBuffers();
		}
	}

	void Application::Run()
	{
		RN_ADD_FOR_GAME_DIST(
			BeginPlay(AppBeginPlayEvent{})
		);

		//std::thread RenderThread(RenderThread);

		while(m_Running) {

			m_FrameTimer.Tick();
			const float& DeltaMs = m_FrameTimer.DeltaTime();
			m_pWindow->SetWindowTitleFPS(m_FrameTimer.FPS());


			m_pWindow->OnUpdate(DeltaMs);
			m_pGameLayer->Update(DeltaMs);

			for (Layer* layer : m_LayerStack) { 
				layer->OnUpdate(DeltaMs);
			}

			m_pGameLayer->PreRender();
			m_pGameLayer->Render();

			// Render Editor UI
			RN_STRIP_FOR_GAME_DIST(
				m_pImGuiLayer->Begin();
				for (Layer* Layer : m_LayerStack) {
					Layer->OnImGuiRender();
				}
				m_pGameLayer->OnImGuiRender();
				m_pImGuiLayer->End();
			);

			m_pWindow->EndFrame();
		}
		
		//RenderThread.join();
	}

	void Application::Shutdown()
	{
	}

	void Application::OnEvent(Event & e)
	{
		EventDispatcher Dispatcher(e);
		Dispatcher.Dispatch<WindowCloseEvent>(RN_BIND_EVENT_FN(Application::OnWindowClose));
		Dispatcher.Dispatch<WindowResizeEvent>(RN_BIND_EVENT_FN(Application::OnWindowResize));
		Dispatcher.Dispatch<WindowToggleFullScreenEvent>(RN_BIND_EVENT_FN(Application::OnWindowFullScreen));
		Dispatcher.Dispatch<SceneSaveEvent>(RN_BIND_EVENT_FN(Application::SaveScene));
		Dispatcher.Dispatch<AppBeginPlayEvent>(RN_BIND_EVENT_FN(Application::BeginPlay));
		Dispatcher.Dispatch<AppEndPlayEvent>(RN_BIND_EVENT_FN(Application::EndPlay));
		Dispatcher.Dispatch<AppScriptReloadEvent>(RN_BIND_EVENT_FN(Application::ReloadScripts));
		Dispatcher.Dispatch<ShaderReloadEvent>(RN_BIND_EVENT_FN(Application::ReloadShaders));

		Input::GetInputManager().OnEvent(e);
		Runtime::ACamera::Get().OnEvent(e);
		
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled()) break;
		}
	}

	void Application::PushEngineLayers()
	{
		switch (Renderer::GetAPI())
		{
#if defined RN_PLATFORM_WINDOWS
		case Renderer::eTargetRenderAPI::D3D_11:
		{
			RN_STRIP_FOR_GAME_DIST(m_pImGuiLayer = new D3D11ImGuiLayer());
			break;
		}
		case Renderer::eTargetRenderAPI::D3D_12:
		{
			RN_STRIP_FOR_GAME_DIST(m_pImGuiLayer = new D3D12ImGuiLayer());
			break;
		}
#endif
		default:
		{
			RN_CORE_ERROR("Failed to create ImGui layer in application with API of type \"{0}\"", Renderer::GetAPI());
			break;
		}
		}

		RN_STRIP_FOR_GAME_DIST(m_pEditorLayer = new EditorLayer());
		RN_STRIP_FOR_GAME_DIST(PushOverlay(m_pImGuiLayer);)
		RN_STRIP_FOR_GAME_DIST(PushOverlay(m_pEditorLayer);)
	}

	void Application::PushLayer(Layer * layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer * layer)
	{
		m_LayerStack.PushOverLay(layer);
		layer->OnAttach();
	}

	bool Application::OnWindowClose(WindowCloseEvent & e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		m_pWindow->Resize(e.GetWidth(), e.GetHeight(), e.GetIsMinimized());
		return true;
	}

	bool Application::OnWindowFullScreen(WindowToggleFullScreenEvent& e)
	{
		m_pWindow->ToggleFullScreen(e.GetFullScreenEnabled());
		return true;
	}

	bool Application::SaveScene(SceneSaveEvent& e)
	{
		std::future<bool> Future = std::async(std::launch::async, FileSystem::WriteSceneToJson, m_pGameLayer->GetScene());
		return true;
	}

	bool Application::BeginPlay(AppBeginPlayEvent& e)
	{
		PushLayer(m_pGameLayer);
		return true;
	}

	bool Application::EndPlay(AppEndPlayEvent& e)
	{
		m_pGameLayer->EndPlay();
		m_LayerStack.PopLayer(m_pGameLayer);
		m_pGameLayer->OnDetach();
		return true;
	}

	bool Application::ReloadScripts(AppScriptReloadEvent& e)
	{
		RN_CORE_INFO("Reloading C# Scripts");
		ResourceManager::Get().GetMonoScriptManager().ReCompile();
		return true;
	}

	bool Application::ReloadShaders(ShaderReloadEvent& e)
	{
		Renderer::OnShaderReload();
		return true;
	}

}
