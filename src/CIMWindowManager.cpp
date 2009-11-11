#include "CIMWindowManager.h"
#include "Conf.h"

#include <glib.h>
#include <glibmm/main.h>

/** @TODO this function is a hack in fact. I don't know if it's good to inherit 
singletons 

The Instance() of CIMWindowManager needs to be called __before__ the Instance() of WindowManager
*/

CIMWindowManager* CIMWindowManager::Instance(void)
{
	CIMWindowManager * realInstance = dynamic_cast<CIMWindowManager*>(instance);
	if (!realInstance){
		assert(instance == NULL); // check that the WindowManager::instance wasn't created before nor deleted
		realInstance = new CIMWindowManager();
		instance = realInstance;
	}
	return realInstance;
}

bool CIMWindowManager::Resize(void)
{
	if (resizepending) {
		screenW = RealScreenWidth();
		screenH = RealScreenHeight();
		
		calculate_sizes();

		signal_resize();

		resizepending = false;

		Redraw();
	}

	return false;
}

void CIMWindowManager::ScreenResized(void)
{
	if (!resizepending) {
		resizepending = true;
		Glib::signal_timeout().connect(sigc::mem_fun(this, &CIMWindowManager::Resize), 0);
	}
}


Rect CIMWindowManager::ScreenAreaSize(ScreenArea area)
{
	return areaSizes[area];
}

void CIMWindowManager::calculate_sizes(void)
{
	Rect size = Conf::Instance()->GetBuddyListDimensions();
	size.width = (int)(size.width * (screenW/(double)originalW));
	size.height = screenH;
	areaSizes[BuddyList] = size;
	
	size = Conf::Instance()->GetLogDimensions();
	size.x = areaSizes[BuddyList].width;
	size.width = screenW - size.x;
	size.height = (int)(size.height * (screenH/(double)originalH));
	size.y = screenH - size.height;
	areaSizes[Log] = size;

	areaSizes[Chat].x = areaSizes[BuddyList].width;
	areaSizes[Chat].y = 0;
	areaSizes[Chat].width = screenW - areaSizes[Chat].x;
	areaSizes[Chat].height = screenH - areaSizes[Log].height;

	areaSizes[Screen].x = 0;
	areaSizes[Screen].y = 0;
	areaSizes[Screen].width = screenW;
	areaSizes[Screen].height = screenH;
}

void CIMWindowManager::Add(Window *window)
{
	WindowManager::Add(window);
	window->ScreenResized();
}
