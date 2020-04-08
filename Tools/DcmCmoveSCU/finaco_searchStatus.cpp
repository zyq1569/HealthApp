#include "finaco_searchStatus.h"

finaco_searchStatus::finaco_searchStatus( wxWindow* parent )
:
searchStatus( parent )
{

#if defined(_WIN32)
	SetMinSize(wxSize(600, 120));
	SetSize(wxSize(600, 120));
#elif defined(__WXMAC__)
	SetMinSize(wxSize(400, 100));
	SetSize(wxSize(400, 100));
#endif
	Center();
	timer.Connect(wxEVT_TIMER, wxTimerEventHandler(finaco_searchStatus::OnTimer), NULL, this);
	timer.Start(500);
}

void finaco_searchStatus::OnStop( wxCommandEvent& event )
{
	m_scanner->Cancel();
}


void finaco_searchStatus::OnTimer(wxTimerEvent& event)
{
	if (m_scanner->IsDone())
	{
		timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(finaco_searchStatus::OnTimer), NULL, this);
		EndModal(0);
	}
	else
	{
		m_progress->Pulse();
	}
}

finaco_searchStatus::~finaco_searchStatus()
{
	timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(finaco_searchStatus::OnTimer), NULL, this);
}