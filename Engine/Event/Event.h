#include <Entity/Entity.h>

namespace Engine{
template <typename ...Args>
class Event {

public:

	using  EventFunction = std::function<void(Args...)>;
	using EventId = std::string;

	Event(const EventFunction& func) :m_eventFunction(func) {}


	void operator()(Args ...params) const {
		if (m_eventFunction) {
			m_eventFunction(params...);
		}
	}



protected:


	EventId m_eventId;
	EventFunction m_eventFunction;

};


template<typename... Args>
class Publisher {

public:
	Publisher() {};

	using eventsList = std::list<Event<Args...>>;

	void trigger(Args ...params) const {

		for (const auto& event_handle : registeredEvents) {
			event_handle(params...);
		}
	}

	void registerEvent(const Engine::Event<Args...>& event) {
		registeredEvents.push_back(event);
	}

	void unregisterEvent(const Engine::Event<Args ...>& event_) {
		for (const auto& event_handle : registeredEvents) {
			if (event_.m_eventId == event_handle)registeredEvents.erase(event_handle);
		}

	}

	void operator += (const Engine::Event<Args ...>& event ) {

		registerEvent(event);
	}

	void operator-=(const Engine::Event<Args...>& event) {
		unregisterEvent(event);
	}


protected:
	eventsList registeredEvents;


};


};