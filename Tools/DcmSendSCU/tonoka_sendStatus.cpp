#include "tonoka_sendStatus.h"

tonoka_sendStatus::tonoka_sendStatus( wxWindow* parent )
:
sendStatus( parent )
{
  #if defined(_WIN32)
  	SetMinSize(wxSize(600, 120));
  	SetSize(wxSize(600, 120));
  #elif defined(__WXMAC__)
  	SetMinSize(wxSize(400, 100));
  	SetSize(wxSize(400, 100));
  #endif
  	Center();
  	timer.Connect(wxEVT_TIMER, wxTimerEventHandler(tonoka_sendStatus::OnTimer), NULL, this);
  	timer.Start(500);
}

void tonoka_sendStatus::OnStop( wxCommandEvent& event )
{
  m_sender->Cancel();
}

void tonoka_sendStatus::OnTimer(wxTimerEvent& event)
{
	if (m_sender->IsDone())
	{
		timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(tonoka_sendStatus::OnTimer), NULL, this);
		EndModal(0);
	}
	else
	{
		m_progress->Pulse();
	}
}

tonoka_sendStatus::~tonoka_sendStatus()
{
	timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(tonoka_sendStatus::OnTimer), NULL, this);
}
