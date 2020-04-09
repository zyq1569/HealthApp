#ifndef __tonoka_searchStatus__
#define __tonoka_searchStatus__

/**
@file
Subclass of searchStatus, which is generated by wxFormBuilder.
*/

#include "gui.h"

//// end generated include

#include "dicomscanner.h"
#include <wx/timer.h>

/** Implementing searchStatus */
class tonoka_searchStatus : public searchStatus
{
	protected:
		// Handlers for searchStatus events.
		void OnStop( wxCommandEvent& event );
	public:
		/** Constructor */
		tonoka_searchStatus( wxWindow* parent );
	//// end generated class members

		~tonoka_searchStatus();
		wxTimer timer;
		void OnTimer(wxTimerEvent& event);
		DICOMFileScanner *m_scanner;

};

#endif // __tonoka_searchStatus__