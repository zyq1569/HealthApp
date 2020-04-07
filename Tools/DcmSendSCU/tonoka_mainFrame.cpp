#include "tonoka_mainFrame.h"

tonoka_mainFrame::tonoka_mainFrame( wxWindow* parent )
:
mainFrame( parent )
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
	m_studies->InsertColumn(5, "Path");
	m_studies->InsertColumn(6, "Status");
    m_studies->EnableCheckBoxes();

  	m_studies->SetColumnWidth(0, 50);
  	m_studies->SetColumnWidth(1, 200);
  	m_studies->SetColumnWidth(2, 100);
  	m_studies->SetColumnWidth(3, 200);
	m_studies->SetColumnWidth(4, 300);
	m_studies->SetColumnWidth(5, 300);

  	// Fill in settings
  	m_engine.LoadDestinationList();
  	m_engine.LoadGlobalDestinationList();

  	FillDestinationList();

  	wxConfig::Get()->SetPath("/Settings");
  	m_directory->SetValue(wxConfig::Get()->Read("Directory"));
  	m_destination->SetStringSelection(wxConfig::Get()->Read("Destination"));
	m_threads->SetValue(wxConfig::Get()->ReadLong("Threads", 4));
	m_depth->SetValue(wxConfig::Get()->ReadLong("Depth", 1));

	{
		wxBusyInfo wait("Please wait, loading existing tonoka.db ...");
		m_engine.patientdata.createdb();
		FillStudyList();
	}
}

tonoka_mainFrame::~tonoka_mainFrame()
{
	wxConfig::Get()->SetPath("/Settings");
	wxConfig::Get()->Write("Directory", m_directory->GetValue());
	wxConfig::Get()->Write("Destination", m_destination->GetStringSelection());
	wxConfig::Get()->Write("Threads", m_threads->GetValue());
	wxConfig::Get()->Write("Depth", m_depth->GetValue());
	wxConfig::Get()->Flush();
}

void tonoka_mainFrame::OnBrowse( wxCommandEvent& event )
{
	wxDirDialog dlg(this, "", m_directory->GetValue(), wxRESIZE_BORDER | wxDD_DIR_MUST_EXIST);
	if(dlg.ShowModal() == wxID_OK)
	{
		m_directory->SetValue(dlg.GetPath());
	}
}

void tonoka_mainFrame::OnDestinationEdit( wxCommandEvent& event )
{
  tonoka_destination dlg(this);
	dlg.m_destinations = m_engine.destinations;

	if (dlg.ShowModal() == wxID_OK)
	{
		wxString oldsel1 = m_destination->GetStringSelection();
		m_engine.destinations = dlg.m_destinations;
		m_engine.SaveDestinationList();

		FillDestinationList();

		m_destination->SetStringSelection(oldsel1);
	}
}

void tonoka_mainFrame::m_studiesOnListColClick( wxListEvent& event )
{
// TODO: Implement m_studiesOnListColClick
}

void tonoka_mainFrame::OnClear( wxCommandEvent& event )
{
	wxBusyCursor wait;
	m_studies->DeleteAllItems();
	m_engine.patientdata.Clear();
}

void tonoka_mainFrame::OnLoad(wxCommandEvent& event)
{
	wxFileDialog dlg(this, "Load...", wxEmptyString, wxEmptyString, "sqlite database (*.db)|*.db", wxFD_OPEN | wxRESIZE_BORDER | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxBusyInfo wait("Please wait, loading ...");
		m_studies->DeleteAllItems();
		boost::filesystem::path p(dlg.GetPath().fn_str());
		m_engine.patientdata.Load(p);
		FillStudyList();
	}
}

void tonoka_mainFrame::OnSave(wxCommandEvent& event)
{
	wxFileDialog dlg(this, "Save...", wxEmptyString, wxEmptyString, "sqlite database (*.db)|*.db", wxFD_SAVE | wxRESIZE_BORDER);
	if (dlg.ShowModal() == wxID_OK)
	{
    boost::filesystem::path p(dlg.GetPath().fn_str());
		m_engine.patientdata.Save(p);
	}
}

void tonoka_mainFrame::OnUpdate( wxCommandEvent& event )
{
	boost::filesystem::path p(m_directory->GetValue().fn_str());
	m_engine.StartScan(p, m_depth->GetValue());

	tonoka_searchStatus dlg(this);
	dlg.m_scanner = &m_engine.scanner;

	// show and wait for thread to end.
	dlg.ShowModal();

	wxBusyInfo wait("Please wait, loading ...");
	FillStudyList();
}

void tonoka_mainFrame::OnSend( wxCommandEvent& event )
{
	if (m_destination->GetSelection() == wxNOT_FOUND)
	{
		wxMessageBox(_("Please select a destination."), _("Error"), wxOK, this);
		return;
	}

	m_engine.StartSend(m_destination->GetSelection(), m_threads->GetValue());

	tonoka_sendStatus dlg(this);
	dlg.m_sender = &m_engine.sender;

	// show and wait for thread to end.
	dlg.ShowModal();

	wxBusyInfo wait("Please wait, loading ...");
	FillStudyList();
}

void tonoka_mainFrame::OnAbout( wxCommandEvent& event )
{
  tonoka_about dlg(this);
  dlg.ShowModal();
}

void tonoka_mainFrame::OnExit( wxCommandEvent& event )
{
  Close();
}


void tonoka_mainFrame::FillDestinationList()
{
	// add to combo box
	m_destination->Clear();
	std::vector<DestinationEntry>::iterator itr;
	for(itr = m_engine.globalDestinations.begin(); itr != m_engine.globalDestinations.end(); itr++)
		m_destination->Append(wxString::FromUTF8((*itr).name.c_str()) + L" (*)");

	for(itr = m_engine.destinations.begin(); itr != m_engine.destinations.end(); itr++)
		m_destination->Append(wxString::FromUTF8((*itr).name.c_str()));
}

void tonoka_mainFrame::FillStudyList()
{
	wxBusyCursor wait;
	m_studies->Disconnect(wxEVT_LIST_ITEM_CHECKED, wxListEventHandler(tonoka_mainFrame::m_studiesOnListItemChecked), NULL, this);
	m_studies->Disconnect(wxEVT_LIST_ITEM_UNCHECKED, wxListEventHandler(tonoka_mainFrame::m_studiesOnListItemUnchecked), NULL, this);
	m_studies->Freeze();
	m_studies->DeleteAllItems();
	m_engine.patientdata.GetStudies(boost::bind(&tonoka_mainFrame::fillstudiescallback, this, _1));
	m_studies->Thaw();
	m_studies->Connect(wxEVT_LIST_ITEM_CHECKED, wxListEventHandler(tonoka_mainFrame::m_studiesOnListItemChecked), NULL, this);
	m_studies->Connect(wxEVT_LIST_ITEM_UNCHECKED, wxListEventHandler(tonoka_mainFrame::m_studiesOnListItemUnchecked), NULL, this);
}

void tonoka_mainFrame::m_studiesOnListItemChecked(wxListEvent& event)
{
	m_engine.patientdata.SetStudyCheck(m_studies->GetItemText(event.GetIndex(), 4).ToUTF8().data(), true);
}

void tonoka_mainFrame::m_studiesOnListItemUnchecked(wxListEvent& event)
{
	m_engine.patientdata.SetStudyCheck(m_studies->GetItemText(event.GetIndex(), 4).ToUTF8().data(), false);
}


int tonoka_mainFrame::fillstudiescallback(Study &study)
{
	int count = m_studies->GetItemCount();
	m_studies->InsertItem(count, wxString::FromUTF8(study.studyuid.c_str()));
	m_studies->SetItem(count, 1, study.patname);
	m_studies->SetItem(count, 2, wxString::FromUTF8(study.studydate.c_str()));
	m_studies->SetItem(count, 3, wxString::FromUTF8(study.studydesc.c_str()));
	m_studies->SetItem(count, 4, wxString::FromUTF8(study.studyuid.c_str()));
	m_studies->SetItem(count, 5, study.path.c_str());
	m_studies->CheckItem(count, study.checked);

	return 0;
}

void tonoka_mainFrame::OnTextSearch(wxCommandEvent& event)
{
	int selected = m_studies->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::string text = m_searchtext->GetValue().ToUTF8();
	//std::string foundstudyuid = m_engine.patientdata.TextSearch(selected + 1, text);

	int newselected = m_studies->FindItem(selected, text);
	if (newselected == -1)
		wxMessageBox("Not found", "Search");
	m_studies->SetItemState(newselected, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	m_studies->EnsureVisible(newselected);
}