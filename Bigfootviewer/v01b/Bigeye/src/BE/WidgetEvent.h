#ifndef _INCLUDED_BIGEYE_BIGEYEWIDGETEVENT_H
#define _INCLUDED_BIGEYE_BIGEYEWIDGETEVENT_H

#include <vector>

namespace BE 
{
	class Widget;

	struct WidgetEvent
	{
		Widget* mWidget;
		void* mEvent;

		WidgetEvent() : mWidget(NULL), mEvent(NULL) {}
		WidgetEvent(Widget* inWidget, void* inEvent = NULL) : mWidget(inWidget), mEvent(inEvent) {}
	};

	class WidgetEventManager
	{
	public:

		void AddEvent(const WidgetEvent& inEvent)
		{
			mEvents.push_back(inEvent);
		}

		void ClearEvents()			{ mEvents.clear(); }  
		size_t GetEventCount()		{ return mEvents.size(); }
		const WidgetEvent& GetEvent(size_t inIndex) { return mEvents[inIndex]; }

	protected:

		typedef std::vector<WidgetEvent> Events;
		Events mEvents;
	};
}

#endif