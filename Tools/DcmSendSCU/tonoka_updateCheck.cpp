#include "tonoka_updateCheck.h"

tonoka_updateCheck::tonoka_updateCheck( wxWindow* parent )
:
updateCheck( parent )
{

}

void tonoka_updateCheck::updateCheckOnInitDialog( wxInitDialogEvent& event )
{
  if (m_updatetext.length() > 0)
    m_needtoupgrade->SetLabel(m_updatetext);

  m_remoteversion->SetLabel(m_version);
  m_remotemessage->SetLabel(m_message);
}
