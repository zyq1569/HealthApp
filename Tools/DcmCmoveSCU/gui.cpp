///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "gui.h"

///////////////////////////////////////////////////////////////////////////

destination::destination( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    
    wxBoxSizer* bSizer23;
    bSizer23 = new wxBoxSizer( wxVERTICAL );
    
    bSizer23->SetMinSize( wxSize( 469,555 ) ); 
    wxBoxSizer* bSizer24;
    bSizer24 = new wxBoxSizer( wxHORIZONTAL );
    
    m_destinationList = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
    m_destinationList->SetMinSize( wxSize( 5,5 ) );
    
    bSizer24->Add( m_destinationList, 1, wxALL|wxEXPAND, 5 );
    
    wxBoxSizer* bSizer30;
    bSizer30 = new wxBoxSizer( wxVERTICAL );
    
    m_add = new wxButton( this, wxID_ANY, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer30->Add( m_add, 0, wxALL, 5 );
    
    m_delete = new wxButton( this, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer30->Add( m_delete, 0, wxALL, 5 );
    
    m_echo = new wxButton( this, wxID_ANY, _("Echo"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer30->Add( m_echo, 0, wxALL, 5 );
    
    
    bSizer24->Add( bSizer30, 0, wxEXPAND, 5 );
    
    
    bSizer23->Add( bSizer24, 1, wxEXPAND, 5 );
    
    
    bSizer23->Add( 0, 5, 0, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2;
    fgSizer2 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer2->AddGrowableCol( 1 );
    fgSizer2->SetFlexibleDirection( wxVERTICAL );
    fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText3 = new wxStaticText( this, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText3->Wrap( -1 );
    fgSizer2->Add( m_staticText3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxBoxSizer* bSizer31;
    bSizer31 = new wxBoxSizer( wxHORIZONTAL );
    
    m_name = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizer31->Add( m_name, 1, wxALL, 5 );
    
    
    fgSizer2->Add( bSizer31, 1, wxEXPAND, 5 );
    
    m_staticText4 = new wxStaticText( this, wxID_ANY, _("Destination Host:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText4->Wrap( -1 );
    fgSizer2->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxBoxSizer* bSizer32;
    bSizer32 = new wxBoxSizer( wxHORIZONTAL );
    
    m_destinationHost = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizer32->Add( m_destinationHost, 1, wxALL, 5 );
    
    
    fgSizer2->Add( bSizer32, 1, wxEXPAND, 5 );
    
    m_staticText5 = new wxStaticText( this, wxID_ANY, _("Destination Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText5->Wrap( -1 );
    fgSizer2->Add( m_staticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxBoxSizer* bSizer29;
    bSizer29 = new wxBoxSizer( wxHORIZONTAL );
    
    m_destinationPort = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizer29->Add( m_destinationPort, 1, wxALL, 5 );
    
    
    fgSizer2->Add( bSizer29, 1, wxEXPAND, 5 );
    
    m_staticText6 = new wxStaticText( this, wxID_ANY, _("Destination AE Title:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText6->Wrap( -1 );
    fgSizer2->Add( m_staticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxBoxSizer* bSizer33;
    bSizer33 = new wxBoxSizer( wxHORIZONTAL );
    
    m_destinationAETitle = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizer33->Add( m_destinationAETitle, 1, wxALL, 5 );
    
    
    fgSizer2->Add( bSizer33, 1, wxEXPAND, 5 );
    
    m_staticText7 = new wxStaticText( this, wxID_ANY, _("Our AE Title:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText7->Wrap( -1 );
    fgSizer2->Add( m_staticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxBoxSizer* bSizer25;
    bSizer25 = new wxBoxSizer( wxHORIZONTAL );
    
    m_ourAETitle = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizer25->Add( m_ourAETitle, 1, wxALL, 5 );
    
    
    fgSizer2->Add( bSizer25, 1, wxEXPAND, 5 );
    
    
    bSizer23->Add( fgSizer2, 0, wxEXPAND, 5 );
    
    
    bSizer23->Add( 0, 5, 0, wxEXPAND, 5 );
    
    m_sdbSizer2 = new wxStdDialogButtonSizer();
    m_sdbSizer2OK = new wxButton( this, wxID_OK );
    m_sdbSizer2->AddButton( m_sdbSizer2OK );
    m_sdbSizer2Cancel = new wxButton( this, wxID_CANCEL );
    m_sdbSizer2->AddButton( m_sdbSizer2Cancel );
    m_sdbSizer2->Realize();
    
    bSizer23->Add( m_sdbSizer2, 0, wxALIGN_LEFT|wxALIGN_TOP|wxBOTTOM|wxEXPAND, 5 );
    
    
    this->SetSizer( bSizer23 );
    this->Layout();
    
    this->Centre( wxBOTH );
    
    // Connect Events
    this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( destination::OnInitDialog ) );
    m_destinationList->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( destination::OnDeselected ), NULL, this );
    m_destinationList->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( destination::OnSelect ), NULL, this );
    m_add->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnAdd ), NULL, this );
    m_delete->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnDelete ), NULL, this );
    m_echo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnEcho ), NULL, this );
    m_name->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnNameText ), NULL, this );
    m_destinationHost->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationHostText ), NULL, this );
    m_destinationPort->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationPortText ), NULL, this );
    m_destinationAETitle->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationAETitleText ), NULL, this );
    m_ourAETitle->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnOurAETitleText ), NULL, this );
    m_sdbSizer2OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnOK ), NULL, this );
}

destination::~destination()
{
    // Disconnect Events
    this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( destination::OnInitDialog ) );
    m_destinationList->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( destination::OnDeselected ), NULL, this );
    m_destinationList->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( destination::OnSelect ), NULL, this );
    m_add->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnAdd ), NULL, this );
    m_delete->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnDelete ), NULL, this );
    m_echo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnEcho ), NULL, this );
    m_name->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnNameText ), NULL, this );
    m_destinationHost->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationHostText ), NULL, this );
    m_destinationPort->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationPortText ), NULL, this );
    m_destinationAETitle->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnDestinationAETitleText ), NULL, this );
    m_ourAETitle->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( destination::OnOurAETitleText ), NULL, this );
    m_sdbSizer2OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( destination::OnOK ), NULL, this );
    
}

mainFrame::mainFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
    this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
    this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
    
    wxBoxSizer* bSizer14;
    bSizer14 = new wxBoxSizer( wxHORIZONTAL );
    
    wxBoxSizer* bSizer16;
    bSizer16 = new wxBoxSizer( wxVERTICAL );
    
    wxFlexGridSizer* fgSizer1;
    fgSizer1 = new wxFlexGridSizer( 4, 3, 0, 0 );
    fgSizer1->AddGrowableCol( 1 );
    fgSizer1->SetFlexibleDirection( wxHORIZONTAL );
    fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
    
    m_staticText1 = new wxStaticText( this, wxID_ANY, _("Query:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText1->Wrap( -1 );
    fgSizer1->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    m_querydestination = new wxComboBox( this, wxID_ANY, _("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_READONLY ); 
    fgSizer1->Add( m_querydestination, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
    
    m_button6 = new wxButton( this, wxID_ANY, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer1->Add( m_button6, 0, wxALL, 5 );
    
    m_staticText2 = new wxStaticText( this, wxID_ANY, _("Move:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText2->Wrap( -1 );
    fgSizer1->Add( m_staticText2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    m_movedestination = new wxComboBox( this, wxID_ANY, _("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_READONLY ); 
    fgSizer1->Add( m_movedestination, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
    
    m_panel2 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    fgSizer1->Add( m_panel2, 1, wxEXPAND | wxALL, 5 );
    
    m_staticText15 = new wxStaticText( this, wxID_ANY, _("MoveAE:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText15->Wrap( -1 );
    fgSizer1->Add( m_staticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    m_moveae = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer1->Add( m_moveae, 1, wxALL|wxEXPAND, 5 );
    
    m_panel31 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    fgSizer1->Add( m_panel31, 1, wxEXPAND | wxALL, 5 );
    
    m_staticText17 = new wxStaticText( this, wxID_ANY, _("Threads"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText17->Wrap( -1 );
    fgSizer1->Add( m_staticText17, 0, wxALL, 5 );
    
    m_threads = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, 4 );
    fgSizer1->Add( m_threads, 0, wxALL, 5 );
    
    
    bSizer16->Add( fgSizer1, 0, wxEXPAND, 5 );
    
    wxBoxSizer* bSizer13;
    bSizer13 = new wxBoxSizer( wxHORIZONTAL );
    
    bSizer13->SetMinSize( wxSize( 100,100 ) ); 
    wxBoxSizer* bSizer15;
    bSizer15 = new wxBoxSizer( wxVERTICAL );
    
    m_studies = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
    m_studies->SetMinSize( wxSize( -1,150 ) );
    
    bSizer15->Add( m_studies, 1, wxALL|wxEXPAND, 5 );
    
    wxBoxSizer* bSizer25;
    bSizer25 = new wxBoxSizer( wxHORIZONTAL );
    
    m_button121 = new wxButton( this, wxID_ANY, _("Load"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer25->Add( m_button121, 0, wxALL, 5 );
    
    m_button13 = new wxButton( this, wxID_ANY, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer25->Add( m_button13, 0, wxALL, 5 );
    
    m_button12 = new wxButton( this, wxID_ANY, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer25->Add( m_button12, 0, wxALL, 5 );
    
    
    bSizer15->Add( bSizer25, 0, wxEXPAND, 5 );
    
    
    bSizer13->Add( bSizer15, 2, wxEXPAND, 5 );
    
    
    bSizer16->Add( bSizer13, 1, wxEXPAND, 5 );
    
    
    bSizer14->Add( bSizer16, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxTOP, 5 );
    
    wxBoxSizer* bSizer17;
    bSizer17 = new wxBoxSizer( wxVERTICAL );
    
    wxBoxSizer* bSizer161;
    bSizer161 = new wxBoxSizer( wxVERTICAL );
    
    m_update = new wxButton( this, wxID_ANY, _("Query"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer161->Add( m_update, 0, wxALL, 5 );
    
    m_move = new wxButton( this, wxID_ANY, _("Move"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer161->Add( m_move, 0, wxALL, 5 );
    
    
    bSizer17->Add( bSizer161, 0, wxEXPAND, 5 );
    
    wxBoxSizer* bSizer191;
    bSizer191 = new wxBoxSizer( wxVERTICAL );
    
    m_panel3 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    bSizer191->Add( m_panel3, 1, wxEXPAND | wxALL, 5 );
    
    
    bSizer17->Add( bSizer191, 1, wxEXPAND, 5 );
    
    wxBoxSizer* bSizer181;
    bSizer181 = new wxBoxSizer( wxVERTICAL );
    
    m_about = new wxButton( this, wxID_ANY, _("About"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer181->Add( m_about, 0, wxALL, 5 );
    
    m_exit = new wxButton( this, wxID_ANY, _("Exit"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer181->Add( m_exit, 0, wxALL|wxBOTTOM, 5 );
    
    
    bSizer17->Add( bSizer181, 0, wxEXPAND, 5 );
    
    
    bSizer14->Add( bSizer17, 0, wxBOTTOM|wxEXPAND|wxRIGHT|wxTOP, 5 );
    
    
    this->SetSizer( bSizer14 );
    this->Layout();
    
    this->Centre( wxBOTH );
    
    // Connect Events
    m_button6->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnDestinationEdit ), NULL, this );
    m_studies->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( mainFrame::m_studiesOnListColClick ), NULL, this );
    m_button121->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnLoad ), NULL, this );
    m_button13->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnSave ), NULL, this );
    m_button12->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnClear ), NULL, this );
    m_update->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnQuery ), NULL, this );
    m_move->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnMove ), NULL, this );
    m_about->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnAbout ), NULL, this );
    m_exit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnExit ), NULL, this );
}

mainFrame::~mainFrame()
{
    // Disconnect Events
    m_button6->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnDestinationEdit ), NULL, this );
    m_studies->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( mainFrame::m_studiesOnListColClick ), NULL, this );
    m_button121->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnLoad ), NULL, this );
    m_button13->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnSave ), NULL, this );
    m_button12->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnClear ), NULL, this );
    m_update->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnQuery ), NULL, this );
    m_move->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnMove ), NULL, this );
    m_about->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnAbout ), NULL, this );
    m_exit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( mainFrame::OnExit ), NULL, this );
    
}

about::about( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    
    wxBoxSizer* bSizer28;
    bSizer28 = new wxBoxSizer( wxVERTICAL );
    
    m_version = new wxStaticText( this, wxID_ANY, _("Version"), wxDefaultPosition, wxDefaultSize, 0 );
    m_version->Wrap( -1 );
    bSizer28->Add( m_version, 0, wxALL, 5 );
    
    m_buildinfo = new wxStaticText( this, wxID_ANY, _("Build Info"), wxDefaultPosition, wxDefaultSize, 0 );
    m_buildinfo->Wrap( -1 );
    bSizer28->Add( m_buildinfo, 0, wxALL, 5 );
    
    m_copyright = new wxStaticText( this, wxID_ANY, _("Copyright (C) 2015-2016 Ing-Long Eric Kuo"), wxDefaultPosition, wxDefaultSize, 0 );
    m_copyright->Wrap( -1 );
    bSizer28->Add( m_copyright, 0, wxALL, 5 );
    
    wxBoxSizer* bSizer27;
    bSizer27 = new wxBoxSizer( wxHORIZONTAL );
    
    m_staticText13 = new wxStaticText( this, wxID_ANY, _("Homepage:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText13->Wrap( -1 );
    bSizer27->Add( m_staticText13, 0, wxALL, 5 );
    
    m_hyperlink1 = new wxHyperlinkCtrl( this, wxID_ANY, wxEmptyString, wxT("modify:sourse from  http://www.draconpern.com/software/CmoveSCU"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    bSizer27->Add( m_hyperlink1, 0, wxALL, 5 );
    
    
    bSizer28->Add( bSizer27, 1, wxEXPAND, 5 );
    
    m_sdbSizer1 = new wxStdDialogButtonSizer();
    m_sdbSizer1OK = new wxButton( this, wxID_OK );
    m_sdbSizer1->AddButton( m_sdbSizer1OK );
    m_sdbSizer1->Realize();
    
    bSizer28->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 5 );
    
    
    this->SetSizer( bSizer28 );
    this->Layout();
    
    this->Centre( wxBOTH );
}

about::~about()
{
}

moveStatus::moveStatus( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
    
    wxBoxSizer* bSizer30;
    bSizer30 = new wxBoxSizer( wxHORIZONTAL );
    
    m_progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
    m_progress->SetValue( 0 ); 
    bSizer30->Add( m_progress, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxTOP, 15 );
    
    m_stop = new wxButton( this, wxID_ANY, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer30->Add( m_stop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15 );
    
    
    this->SetSizer( bSizer30 );
    this->Layout();
    bSizer30->Fit( this );
    
    this->Centre( wxBOTH );
    
    // Connect Events
    m_stop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( moveStatus::OnStop ), NULL, this );
}

moveStatus::~moveStatus()
{
    // Disconnect Events
    m_stop->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( moveStatus::OnStop ), NULL, this );
    
}

searchStatus::searchStatus( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
    
    wxBoxSizer* bSizer28;
    bSizer28 = new wxBoxSizer( wxHORIZONTAL );
    
    m_progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL|wxGA_SMOOTH );
    m_progress->SetValue( 0 ); 
    bSizer28->Add( m_progress, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxTOP, 15 );
    
    m_stop = new wxButton( this, wxID_ANY, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer28->Add( m_stop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15 );
    
    
    this->SetSizer( bSizer28 );
    this->Layout();
    bSizer28->Fit( this );
    
    this->Centre( wxBOTH );
    
    // Connect Events
    m_stop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( searchStatus::OnStop ), NULL, this );
}

searchStatus::~searchStatus()
{
    // Disconnect Events
    m_stop->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( searchStatus::OnStop ), NULL, this );
    
}

updateCheck::updateCheck( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    
    wxBoxSizer* bSizer28;
    bSizer28 = new wxBoxSizer( wxVERTICAL );
    
    wxBoxSizer* bSizer29;
    bSizer29 = new wxBoxSizer( wxVERTICAL );
    
    m_needtoupgrade = new wxStaticText( this, wxID_ANY, _("A new version of CmoveSCU is available:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_needtoupgrade->Wrap( -1 );
    bSizer29->Add( m_needtoupgrade, 0, wxALL, 5 );
    
    m_remoteversion = new wxStaticText( this, wxID_ANY, _("x.x.x.x"), wxDefaultPosition, wxDefaultSize, 0 );
    m_remoteversion->Wrap( -1 );
    bSizer29->Add( m_remoteversion, 0, wxALL, 5 );
    
    m_remotemessage = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
    bSizer29->Add( m_remotemessage, 1, wxALL|wxEXPAND, 5 );
    
    wxBoxSizer* bSizer27;
    bSizer27 = new wxBoxSizer( wxHORIZONTAL );
    
    m_staticText13 = new wxStaticText( this, wxID_ANY, _("Download at:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText13->Wrap( -1 );
    bSizer27->Add( m_staticText13, 0, wxALL, 5 );
    
    m_hyperlink11 = new wxHyperlinkCtrl( this, wxID_ANY, wxEmptyString, wxT("modify:sourse from http://www.draconpern.com/software/CmoveSCU"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    bSizer27->Add( m_hyperlink11, 0, wxALL, 5 );
    
    
    bSizer29->Add( bSizer27, 0, wxEXPAND, 5 );
    
    
    bSizer28->Add( bSizer29, 1, wxEXPAND, 5 );
    
    m_sdbSizer4 = new wxStdDialogButtonSizer();
    m_sdbSizer4OK = new wxButton( this, wxID_OK );
    m_sdbSizer4->AddButton( m_sdbSizer4OK );
    m_sdbSizer4->Realize();
    
    bSizer28->Add( m_sdbSizer4, 0, wxALL|wxEXPAND, 5 );
    
    
    this->SetSizer( bSizer28 );
    this->Layout();
    
    this->Centre( wxBOTH );
    
    // Connect Events
    this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( updateCheck::updateCheckOnInitDialog ) );
}

updateCheck::~updateCheck()
{
    // Disconnect Events
    this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( updateCheck::updateCheckOnInitDialog ) );
    
}
