///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GUI_H__
#define __GUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/combobox.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/frame.h>
#include <wx/hyperlink.h>
#include <wx/gauge.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class destination
///////////////////////////////////////////////////////////////////////////////
class destination : public wxDialog 
{
	private:
	
	protected:
		wxListCtrl* m_destinationList;
		wxButton* m_add;
		wxButton* m_delete;
		wxButton* m_echo;
		wxStaticText* m_staticText3;
		wxTextCtrl* m_name;
		wxStaticText* m_staticText4;
		wxTextCtrl* m_destinationHost;
		wxStaticText* m_staticText5;
		wxTextCtrl* m_destinationPort;
		wxStaticText* m_staticText6;
		wxTextCtrl* m_destinationAETitle;
		wxStaticText* m_staticText7;
		wxTextCtrl* m_ourAETitle;
		wxStdDialogButtonSizer* m_sdbSizer2;
		wxButton* m_sdbSizer2OK;
		wxButton* m_sdbSizer2Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnInitDialog( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnDeselected( wxListEvent& event ) { event.Skip(); }
		virtual void OnSelect( wxListEvent& event ) { event.Skip(); }
		virtual void OnAdd( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDelete( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEcho( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNameText( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDestinationHostText( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDestinationPortText( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDestinationAETitleText( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOurAETitleText( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		destination( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Destinations"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 469,555 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~destination();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class mainFrame
///////////////////////////////////////////////////////////////////////////////
class mainFrame : public wxFrame 
{
	private:
	
	protected:
		wxStaticText* m_staticText1;
		wxComboBox* m_querydestination;
		wxButton* m_button6;
		wxStaticText* m_staticText2;
		wxComboBox* m_movedestination;
		wxPanel* m_panel2;
		wxStaticText* m_staticText15;
		wxTextCtrl* m_moveae;
		wxPanel* m_panel31;
		wxStaticText* m_staticText17;
		wxSpinCtrl* m_threads;
		wxListCtrl* m_studies;
		wxButton* m_button121;
		wxButton* m_button13;
		wxButton* m_button12;
		wxButton* m_update;
		wxButton* m_move;
		wxPanel* m_panel3;
		wxButton* m_about;
		wxButton* m_exit;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnDestinationEdit( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_studiesOnListColClick( wxListEvent& event ) { event.Skip(); }
		virtual void OnLoad( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClear( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnQuery( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMove( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExit( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		mainFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("finaco"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 625,426 ), long style = wxDEFAULT_FRAME_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL );
		
		~mainFrame();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class about
///////////////////////////////////////////////////////////////////////////////
class about : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_version;
		wxStaticText* m_buildinfo;
		wxStaticText* m_copyright;
		wxStaticText* m_staticText13;
		wxHyperlinkCtrl* m_hyperlink1;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
	
	public:
		
		about( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("About"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 480,234 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~about();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class moveStatus
///////////////////////////////////////////////////////////////////////////////
class moveStatus : public wxDialog 
{
	private:
	
	protected:
		wxGauge* m_progress;
		wxButton* m_stop;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnStop( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		moveStatus( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Movng..."), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~moveStatus();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class searchStatus
///////////////////////////////////////////////////////////////////////////////
class searchStatus : public wxDialog 
{
	private:
	
	protected:
		wxGauge* m_progress;
		wxButton* m_stop;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnStop( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		searchStatus( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Searching..."), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~searchStatus();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class updateCheck
///////////////////////////////////////////////////////////////////////////////
class updateCheck : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_needtoupgrade;
		wxStaticText* m_remoteversion;
		wxTextCtrl* m_remotemessage;
		wxStaticText* m_staticText13;
		wxHyperlinkCtrl* m_hyperlink11;
		wxStdDialogButtonSizer* m_sdbSizer4;
		wxButton* m_sdbSizer4OK;
		
		// Virtual event handlers, overide them in your derived class
		virtual void updateCheckOnInitDialog( wxInitDialogEvent& event ) { event.Skip(); }
		
	
	public:
		
		updateCheck( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Update Check"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 480,300 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~updateCheck();
	
};

#endif //__GUI_H__
