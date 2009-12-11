// TODO license
// TODO add and fix documentation

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "InputProcessor.h"
#include "WindowManager.h"

#include <glibmm.h>

class Application
: public InputProcessor
{
	public:
		/** Starts stdin input handling and runs glib main loop. */
		virtual void Run(void);
		/** Quits glib main loop and stops stdin input handling. */
		virtual void Quit(void);

		// glib IO callbacks
		/** Handle stdin IO errors (quits the application). */
		static gboolean io_input_error_(GIOChannel *source, GIOCondition cond, gpointer data)
			{ return ((Application *) data)->io_input_error(source, cond); }
		/** Process input data from stdin. */
		static gboolean io_input_(GIOChannel *source, GIOCondition cond, gpointer data)
			{ return ((Application *) data)->io_input(source, cond); }

		/** Callback for screen resizing. Application should overwrite this
		 * method and use it for calculating area size of all windows. */
		virtual void ScreenResized(void);

	protected:
		WindowManager *windowmanager;

		Application(Application *i);
		Application(const Application &);
		Application &operator=(const Application &);
		~Application(void);

	private:
		GIConv converter;

		GIOChannel *channel;
		guint channel_id;

		Glib::RefPtr<Glib::MainLoop> gmainloop;

		sigc::connection resize;

		gboolean io_input_error(GIOChannel *source, GIOCondition cond);
		gboolean io_input(GIOChannel *source, GIOCondition cond);

		void StdinInputInit(void);
		void StdinInputUnInit(void);
};

#endif /* __APPLICATION_H__ */
