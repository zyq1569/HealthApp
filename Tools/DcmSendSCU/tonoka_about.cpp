#include "tonoka_about.h"

tonoka_about::tonoka_about( wxWindow* parent )
:
about( parent )
{
  m_version->SetLabel(wxString("Version ") + TONOKA_VERSION);
  m_buildinfo->SetLabel(wxString("Build date: ") + __DATE__);
}
