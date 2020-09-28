#pragma once

#include "Event.h"

namespace Retina {



	class RETINA_API WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height, bool minimized)
			: m_Width(width), m_Height(height), m_Minimized(minimized) {}

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }
		inline bool GetIsMinimized() const { return m_Minimized; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResisedEvent: " << m_Width << ", " << m_Height << " | Minimized: " << m_Minimized;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int m_Width, m_Height;
		bool m_Minimized;
	};

	class RETINA_API WindowToggleFullScreenEvent : public Event
	{
	public:
		WindowToggleFullScreenEvent(bool enabled) 
			: m_Enabled(enabled) {}

		inline bool GetFullScreenEnabled() const { return m_Enabled; }

		EVENT_CLASS_TYPE(ToggleWindowFullScreen)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		bool m_Enabled;
	};

	class RETINA_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class RETINA_API PhysicsEvent : public Event
	{
	public:
		PhysicsEvent() {}
		EVENT_CLASS_TYPE(PhysicsCollisionEvent)
			EVENT_CLASS_CATEGORY(EventCategoryPhysics)

		struct CollisionDetails
		{
			Runtime::AActor* pCollider;
			float Velocity;
			/*ieVector3 Direction;
			ieVector3 Normal;*/
		} CollisionInfo;
	};

	class RETINA_API TranslationEvent : public Event
	{
	public:
		TranslationEvent() {}
		EVENT_CLASS_TYPE(WorldTranslationEvent)
			EVENT_CLASS_CATEGORY(EventCategoryTranslation)

		struct TranslationDetails
		{
			ieMatrix4x4 WorldMat;
		} TranslationInfo;
	};

	class RETINA_API AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

		EVENT_CLASS_TYPE(AppTick)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class RETINA_API AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class RETINA_API AppBeginPlayEvent : public Event
	{
	public:
		AppBeginPlayEvent() {}

		EVENT_CLASS_TYPE(AppBeginPlay)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class RETINA_API AppEndPlayEvent : public Event
	{
	public:
		AppEndPlayEvent() {}

		EVENT_CLASS_TYPE(AppEndPlay)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class RETINA_API AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class RETINA_API AppScriptReloadEvent : public Event
	{
	public:
		AppScriptReloadEvent() {}

		EVENT_CLASS_TYPE(AppScriptReload)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class RETINA_API SceneSaveEvent : public Event
	{
	public:
		SceneSaveEvent() {}

		EVENT_CLASS_TYPE(SceneSave)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class RETINA_API ShaderReloadEvent : public Event
	{
	public:
		ShaderReloadEvent() {}

		EVENT_CLASS_TYPE(ShaderReload)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
}