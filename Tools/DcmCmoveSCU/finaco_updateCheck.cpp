#include "finaco_updateCheck.h"

finaco_updateCheck::finaco_updateCheck( wxWindow* parent )
:
updateCheck( parent )
{

}

void finaco_updateCheck::updateCheckOnInitDialog( wxInitDialogEvent& event )
{
if (m_updatetext.length() > 0)
m_needtoupgrade->SetLabel(m_updatetext);

m_remoteversion->SetLabel(m_version);
m_remotemessage->SetLabel(m_message);
}
