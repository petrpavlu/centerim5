#include "CIMWindowManager.h"
#include "Conf.h"

#include <glib.h>
#include <glibmm/main.h>


CIMWindowManager* CIMWindowManager::instance = NULL;

CIMWindowManager* CIMWindowManager::Instance(void)
{
	if (!instance) instance = new CIMWindowManager();
	return instance;
}


bool CIMWindowManager::Resize(void)
{
	if (resizepending) {
		screenW = RealScreenWidth();
		screenH = RealScreenHeight();
		
		calculate_sizes();

		signal_resize();

		resizepending = false;
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
	size.x = areaSizes[BuddyList].x+1;
	size.width = screenW - size.x;
	size.height = (int)(size.height * (screenH/(double)originalH));
	size.y = screenH - size.height;
	areaSizes[Log] = size;
}
