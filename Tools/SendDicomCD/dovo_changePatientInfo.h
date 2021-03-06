#ifndef __dovo_changePatientInfo__
#define __dovo_changePatientInfo__

/**
@file
Subclass of changePatientInfo, which is generated by wxFormBuilder.
*/

#include "gui.h"

//// end generated include

/** Implementing changePatientInfo */
class dovo_changePatientInfo : public changePatientInfo
{
    protected:
        // Handlers for changePatientInfo events.
        void OnInitDialog( wxInitDialogEvent& event );
        void m_enableEditOnCheckBox( wxCommandEvent& event );
        void OnOK( wxCommandEvent& event );
    public:
        /** Constructor */
        dovo_changePatientInfo( wxWindow* parent );
    //// end generated class members

        wxString m_patientID;
        wxString m_patientName;
        wxString m_birthday;
        bool m_changeInfo;


};

#endif // __dovo_changePatientInfo__
