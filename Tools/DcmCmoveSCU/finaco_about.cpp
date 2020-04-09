#include "finaco_about.h"

finaco_about::finaco_about( wxWindow* parent ):about( parent )
{
    m_version->SetLabel(wxString("Version ") + FINACO_VERSION);
    m_buildinfo->SetLabel(wxString("Build date: ") + __DATE__);
}
