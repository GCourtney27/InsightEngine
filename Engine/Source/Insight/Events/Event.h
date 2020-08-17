#pragma once
#include <ie_pch.h>

#include <Insight/Core.h>

namespace Insight {

	// Event system is currently a blocking event system
	// TODO: Implement defered event system

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, ToggleWindowFullScreen, ShaderReload,
		AppBeginPlay, AppEndPlay, AppTick, AppUpdate, AppRender, AppScriptReload,
		SceneSave,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, RawMouseMoved, MouseScrolled,
		PhysicsCollisionEvent
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT_SHIFT(0),
		EventCategoryInput = BIT_SHIFT(1),
		EventCategoryKeyboard = BIT_SHIFT(2),
		EventCategoryMouse = BIT_SHIFT(3),
		EventCategoryMouseButton = BIT_SHIFT(4),
		EventCatecoryPhysics = BIT_SHIFT(5)
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class INSIGHT_API Event
	{
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char * GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool Handled() const { return m_Handled; }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	protected:
		bool m_Handled = false;
	};

	class EventDispatcher
	{
		template<typename Event>
		using EventFn = std::function<bool(Event&)>;

	public:
		EventDispatcher(Event& event)
			: m_Event(event) {}

		template<typename Event>
		bool Dispatch(EventFn<Event> func)
		{
			if (m_Event.GetEventType() == Event::GetStaticType())
			{
				m_Event.m_Handled = func(*(Event*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

}