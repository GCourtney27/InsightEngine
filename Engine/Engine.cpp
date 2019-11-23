#include "Engine.h"
#include "Editor\\Editor.h"
#include "Graphics\\Graphics.h"
#include "..\\Systems\\FileSystem.h"
#include "..\Systems\BenchmarkingTimer.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	windowHeight = height;
	windowWidth = width;
	
	Debug::ScopedTimer timerEngineStart;
	if (!FileSystem::Instance()->Initialize(this))
	{
		ErrorLogger::Log("Failed to initialize File System.");
		return false;
	}

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
	{
		ErrorLogger::Log("Filed to initialize WindowContainer.");
		return false;
	}

	if (!Graphics::Instance()->Initialize(this->render_window.GetHWND(), width, height, this))
	{
		ErrorLogger::Log("Failed to initialize Graphics Instance");
		return false;
	}
	// ============= Choose from these scenes
	// PBR_TexturedShowcase
	// PBR_UnTexturedShowcase
	// Norway - Disable physics for this level!
	// Test
	// PhysicsTest
	// Physics_Showcase
	scene.SetPhysicsEnabled(true);
	if (!FileSystem::Instance()->LoadSceneFromJSON("..\\Assets\\Scenes\\PhysicsTest.json", &scene, Graphics::Instance()->GetDevice(), Graphics::Instance()->GetDeviceContext()))
	{
		ErrorLogger::Log("Failed to initialize scene.");
		return false;
	}

	if (!LuaStateManager::GetStateManager()->Create())
	{
		ErrorLogger::Log("Failed to initialize Lua State Manager");
		return false;
	}

	
	// Enable this to draw the lights mesh (Commented does not effect the lights emission behavior)
	//scene.AddEntity(Graphics::Instance()->pointLight);
	//scene.AddEntity(Graphics::Instance()->directionalLight);

	if (!scene.Initialize())
	{
		ErrorLogger::Log("Failed to initialize scene");
		return false;
	}

	if (!Debug::Editor::Instance()->Initialize(this, this->render_window.GetHWND()))
	{
		ErrorLogger::Log("Failed to initialize Editor Instance.");
		return false;
	}
	timerEngineStart.Stop();
	Debug::Editor::Instance()->DebugLog("Engine initialized in " + std::to_string(timerEngineStart.GetTimeInSeconds()) + " seconds");

	return true;
}

bool Engine::ProccessMessages()
{
	return this->render_window.ProccessMessages();
}

void Engine::Update()
{
	timer.tick();
	float dt = timer.dt();
	float gamedt = timer.dt();

	/*while (!InputManager::Instance()->keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = InputManager::Instance()->keyboard.ReadChar();
	}

	while (!InputManager::Instance()->keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = InputManager::Instance()->keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}*/

	while (!InputManager::Instance()->mouse.EventBufferIsEmpty())
	{
		MouseEvent me = InputManager::Instance()->mouse.ReadEvent();
		if (InputManager::Instance()->mouse.IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)// && !Debug::Editor::Instance()->PlayingGame())
			{
				Graphics::Instance()->editorCamera.GetTransform().AdjustRotation((float)me.GetPosY() * 0.005f, (float)me.GetPosX() * 0.005f, 0.0f);
			}

		}

		if (me.GetType() == MouseEvent::EventType::LRelease)
			Debug::Editor::Instance()->rayCastEnabled = true;

		/*if (Debug::Editor::Instance()->PlayingGame())
		{
			if (InputManager::Instance()->mouse.IsRightDown())
			{
				if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
				{
					InputManager::Instance()->MouseMoved(true);
					InputManager::Instance()->SetMouseX((float)me.GetPosX());
					InputManager::Instance()->SetMouseY((float)me.GetPosY());
				}
				else
				{
					InputManager::Instance()->MouseMoved(false);
				}
			}
			
		}*/

	}
	
	if (InputManager::Instance()->keyboard.KeyIsPressed(VK_F5))
		Debug::Editor::Instance()->PlayGame();

	scene.Update(dt);

	if (Debug::Editor::Instance()->PlayingGame())
		scene.OnUpdate(gamedt);

	Graphics::Instance()->Update(dt);
	Debug::Editor::Instance()->Update(dt);


	// FORCE ENGINE CLOSE (Space + Esc)
	if (InputManager::Instance()->keyboard.KeyIsPressed(27) && InputManager::Instance()->keyboard.KeyIsPressed(' '))
	{
		exit(0); // Performs no cleanup
		PostMessage(this->render_window.GetHWND(), WM_QUIT, 0, 0);
	}

	if (InputManager::Instance()->keyboard.KeyIsPressed('C'))
	{
		Graphics::Instance()->pointLight->GetTransform().SetPosition(Graphics::Instance()->editorCamera.GetTransform().GetPositionFloat3());
		Graphics::Instance()->pointLight->GetTransform().SetRotation(Graphics::Instance()->editorCamera.GetTransform().GetRotationFloat3());
	}
	
	if (m_canSave == false)
	{
		m_saveDelay -= dt;
		if (m_saveDelay <= 0.0f)
		{
			m_saveDelay = 3.0f;
			m_canSave = true;
		}
	}
	if (InputManager::Instance()->keyboard.KeyIsPressed(VK_CONTROL) && InputManager::Instance()->keyboard.KeyIsPressed('S') && m_canSave)
	{
		m_canSave = false;
		if (!FileSystem::Instance()->WriteSceneToJSON(&scene))
			ErrorLogger::Log("Failed to save scene");
		else
			DEBUGLOG("SCENE SAVED");
			//DEBUGLOG("SCENE SAVING DISABLED!");
	}

}

void Engine::RenderFrame()
{
	Graphics::Instance()->RenderFrame();
}

void Engine::Shutdown()
{
	scene.Shutdown();
	//Graphics::Instance()->Shutdown();
}

void Engine::OnGameStart()
{
	scene.OnStart();
}



