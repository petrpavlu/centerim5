#ifndef __CIMWINDOWMANAGER_H__
#define __CIMWINDOWMANAGER_H__


#include <cppconsui/WindowManager.h>


class CIMWindowManager
 : public WindowManager
{
	public:
		enum ScreenArea {BuddyList=0, Log, Chat, Screen};
		
		Rect ScreenAreaSize(ScreenArea area);  // return size of selected area
		static CIMWindowManager* Instance(void);
		virtual bool Resize(void);
		void ScreenResized(void);
		virtual void Add(Window *window);
		
	protected:
		void calculate_sizes(void); // calculates area sizes
		Rect areaSizes[Screen+1];
		
	private:
		static CIMWindowManager *cimInstance;
		static const int originalW = 139;
		static const int originalH = 56;
};

#endif