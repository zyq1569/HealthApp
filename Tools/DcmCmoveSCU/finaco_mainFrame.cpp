#include "finaco_mainFrame.h"

finaco_mainFrame::finaco_mainFrame( wxWindow* parent )
:mainFrame( parent )
{
#if defined(_WIN32)
    SetMinSize(wxSize(850, 700));
    SetSize(wxSize(850, 700));
#elif defined(__WXMAC__)
    SetMinSize(wxSize(850, 700));
    SetSize(wxSize(850, 700));
    m_about->Hide();
    m_exit->Hide();
#endif
    // Create the UI
    m_studies->InsertColumn(0, _(""));
    m_studies->InsertColumn(1, _("Name"));
    m_studies->InsertColumn(2, _("Date"));
    m_studies->InsertColumn(3, _("Description"));
    m_studies->InsertColumn(4, "StudyUID");
    m_studies->InsertColumn(5, "Status");
    m_studies->EnableCheckBoxes();

    m_studies->SetColumnWidth(0, wxLIST_AUTOSIZE);
    m_studies->SetColumnWidth(1, 200);
    m_studies->SetColumnWidth(2, 100);
    m_studies->SetColumnWidth(3, 200);
    m_studies->SetColumnWidth(4, 200);

    // Fill in settings
    m_engine.LoadDestinationList();
    m_engine.LoadGlobalDestinationList();

    FillDestinationList();

    wxConfig::Get()->SetPath("/Settings");
    m_moveae->SetValue(wxConfig::Get()->Read("MoveAE"));
    m_querydestination->SetStringSelection(wxConfig::Get()->Read("QueryDestination"));
    m_movedestination->SetStringSelection(wxConfig::Get()->Read("MoveDestination"));
    m_threads->SetValue(wxConfig::Get()->ReadLong("Threads", 4));

    {
        wxBusyInfo wait("Please wait, loading existing CmoveSCU.db ...");
        m_engine.patientdata.createdb();
        FillStudyList();
    }        
}

void finaco_mainFrame::OnDestinationEdit( wxCommandEvent& event )
{
    finaco_destination dlg(this);
    dlg.m_destinations = m_engine.destinations;

    if (dlg.ShowModal() == wxID_OK)
    {
        wxString oldsel1 = m_querydestination->GetStringSelection();
        wxString oldsel2 = m_movedestination->GetStringSelection();
        m_engine.destinations = dlg.m_destinations;
        m_engine.SaveDestinationList();

        FillDestinationList();

        m_querydestination->SetStringSelection(oldsel1);
        m_movedestination->SetStringSelection(oldsel2);
    }
}


void finaco_mainFrame::FillStudyList()
{
    wxBusyCursor wait;
    m_studies->Disconnect(wxEVT_LIST_ITEM_CHECKED, wxListEventHandler(finaco_mainFrame::m_studiesOnListItemChecked), NULL, this);
    m_studies->Disconnect(wxEVT_LIST_ITEM_UNCHECKED, wxListEventHandler(finaco_mainFrame::m_studiesOnListItemUnchecked), NULL, this);

    m_studies->DeleteAllItems();
    m_engine.patientdata.GetStudies(boost::bind(&finaco_mainFrame::fillstudiescallback, this, _1));

    m_studies->Connect(wxEVT_LIST_ITEM_CHECKED, wxListEventHandler(finaco_mainFrame::m_studiesOnListItemChecked), NULL, this);
    m_studies->Connect(wxEVT_LIST_ITEM_UNCHECKED, wxListEventHandler(finaco_mainFrame::m_studiesOnListItemUnchecked), NULL, this);
}

void finaco_mainFrame::m_studiesOnListColClick( wxListEvent& event )
{

}

void finaco_mainFrame::m_studiesOnListItemChecked(wxListEvent& event)
{
    m_engine.patientdata.SetStudyCheck(m_studies->GetItemText(event.GetIndex(), 4).ToUTF8().data(), true);
}

void finaco_mainFrame::m_studiesOnListItemUnchecked(wxListEvent& event)
{
    m_engine.patientdata.SetStudyCheck(m_studies->GetItemText(event.GetIndex(), 4).ToUTF8().data(), false);
}

void finaco_mainFrame::OnClear( wxCommandEvent& event )
{
    wxBusyCursor wait;
    m_studies->DeleteAllItems();
    m_engine.patientdata.Clear();
}

void finaco_mainFrame::OnLoad(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Load...", wxEmptyString, wxEmptyString,\
        "sqlite database (*.db)|*.db", wxFD_OPEN | wxRESIZE_BORDER | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_studies->DeleteAllItems();
        boost::filesystem::path p(dlg.GetPath().fn_str());
        m_engine.patientdata.Load(p);
        FillStudyList();
    }
}

void finaco_mainFrame::OnSave(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Save...", wxEmptyString, wxEmptyString, "sqlite database (*.db)|*.db", wxFD_SAVE | wxRESIZE_BORDER);
    if (dlg.ShowModal() == wxID_OK)
    {
        boost::filesystem::path p(dlg.GetPath().fn_str());
        m_engine.patientdata.Save(p);
    }
}

void finaco_mainFrame::OnQuery( wxCommandEvent& event )
{
    if (m_querydestination->GetSelection() == wxNOT_FOUND)
    {
        wxMessageBox(_("Please select a destination."), _("Error"), wxOK, this);
        return;
    }

    m_engine.StartQuery(m_querydestination->GetSelection());

    finaco_searchStatus dlg(this);
    dlg.m_scanner = &m_engine.query;

    // show and wait for thread to end.
    dlg.ShowModal();

    m_studies->DeleteAllItems();
    m_engine.patientdata.GetStudies(boost::bind(&finaco_mainFrame::fillstudiescallback, this, _1));
}

void finaco_mainFrame::OnMove( wxCommandEvent& event )
{
    std::string moveae = m_moveae->GetValue().ToUTF8().data();

    if (moveae.length() == 0)
    {
        wxMessageBox(_("Please enter a moveAE."), _("Error"), wxOK, this);
        return;
    }

    if (m_movedestination->GetSelection() == wxNOT_FOUND)
    {
        wxMessageBox(_("Please select a move destination."), _("Error"), wxOK, this);
        return;
    }

    m_engine.StartMove(m_movedestination->GetSelection(), moveae, m_threads->GetValue());

    finaco_moveStatus dlg(this);
    dlg.m_mover = &m_engine.mover;

    // show and wait for thread to end.
    dlg.ShowModal();

    m_studies->DeleteAllItems();
    m_engine.patientdata.GetStudies(boost::bind(&finaco_mainFrame::fillstudiescallback, this, _1));
}

void finaco_mainFrame::OnAbout( wxCommandEvent& event )
{
    finaco_about dlg(this);
    dlg.ShowModal();
}

void finaco_mainFrame::OnExit( wxCommandEvent& event )
{
    Close();
}


finaco_mainFrame::~finaco_mainFrame()
{
    wxConfig::Get()->SetPath("/Settings");
    wxConfig::Get()->Write("MoveAE", m_moveae->GetValue());
    wxConfig::Get()->Write("QueryDestination", m_querydestination->GetStringSelection());
    wxConfig::Get()->Write("MoveDestination", m_movedestination->GetStringSelection());
    wxConfig::Get()->Write("Threads", m_threads->GetValue());
    wxConfig::Get()->Flush();
}

void finaco_mainFrame::FillDestinationList()
{
    // add to combo box
    m_querydestination->Clear();
    m_movedestination->Clear();

    std::vector<DestinationEntry>::iterator itr;
    for (itr = m_engine.globalDestinations.begin(); itr != m_engine.globalDestinations.end(); itr++)
    {
        m_querydestination->Append(wxString::FromUTF8((*itr).name.c_str()) + L" (*)");
        m_movedestination->Append(wxString::FromUTF8((*itr).name.c_str()) + L" (*)");
    }

    for (itr = m_engine.destinations.begin(); itr != m_engine.destinations.end(); itr++)
    {
        m_querydestination->Append(wxString::FromUTF8((*itr).name.c_str()));
        m_movedestination->Append(wxString::FromUTF8((*itr).name.c_str()));
    }
        
}

int finaco_mainFrame::fillstudiescallback(Study &study)
{
    m_studies->InsertItem(0, "");
    m_studies->SetItem(0, 1, study.patname);

    struct tm thedate;
    memset(&thedate, 0, sizeof(struct tm));
    try
    {
        thedate.tm_year = boost::lexical_cast<int>(study.studydate.substr(0, 4)) - 1900;
        thedate.tm_mon = boost::lexical_cast<int>(study.studydate.substr(4, 2)) - 1;
        thedate.tm_mday = boost::lexical_cast<int>(study.studydate.substr(6, 2));
    }
    catch (...)
    {
        thedate.tm_year = 0;
        thedate.tm_mon = 0;
        thedate.tm_mday = 1;
    }

    char buf[1024];
    memset(buf, 0, 1024);

    strftime(buf, 1024, "%x", &thedate);

    m_studies->SetItem(0, 2, wxString::FromUTF8(buf));
    m_studies->SetItem(0, 3, wxString::FromUTF8(study.studydesc.c_str()));
    m_studies->SetItem(0, 4, wxString::FromUTF8(study.studyuid.c_str()));
    m_studies->CheckItem(0, study.checked);    
    return 0;
}