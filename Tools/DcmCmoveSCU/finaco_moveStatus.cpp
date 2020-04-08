#include "finaco_moveStatus.h"

finaco_moveStatus::finaco_moveStatus( wxWindow* parent )
:
moveStatus( parent )
{

#if defined(_WIN32)
	SetMinSize(wxSize(600, 120));
	SetSize(wxSize(600, 120));
#elif defined(__WXMAC__)
	SetMinSize(wxSize(400, 100));
	SetSize(wxSize(400, 100));
#endif
	Center();
	timer.Connect(wxEVT_TIMER, wxTimerEventHandler(finaco_moveStatus::OnTimer), NULL, this);
	timer.Start(500);
}

void finaco_moveStatus::OnStop( wxCommandEvent& event )
{
	m_mover->Cancel();
}

void finaco_moveStatus::OnTimer(wxTimerEvent& event)
{
	if (m_mover->IsDone())
	{
		timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(finaco_moveStatus::OnTimer), NULL, this);
		EndModal(0);
	}
	else
	{
		m_progress->Pulse();
	}
}

finaco_moveStatus::~finaco_moveStatus()
{
	timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(finaco_moveStatus::OnTimer), NULL, this);
}