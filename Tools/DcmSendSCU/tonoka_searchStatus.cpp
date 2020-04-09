#include "tonoka_searchStatus.h"

tonoka_searchStatus::tonoka_searchStatus( wxWindow* parent )
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
      timer.Connect(wxEVT_TIMER, wxTimerEventHandler(tonoka_searchStatus::OnTimer), NULL, this);
      timer.Start(500);
}

void tonoka_searchStatus::OnStop( wxCommandEvent& event )
{
  m_scanner->Cancel();
}


void tonoka_searchStatus::OnTimer(wxTimerEvent& event)
{
    if (m_scanner->IsDone())
    {
        timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(tonoka_searchStatus::OnTimer), NULL, this);
        EndModal(0);
    }
    else
    {
        m_progress->Pulse();
    }
}

tonoka_searchStatus::~tonoka_searchStatus()
{
    timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(tonoka_searchStatus::OnTimer), NULL, this);
}
