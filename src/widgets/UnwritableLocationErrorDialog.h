/**********************************************************************

  Audacity: A Digital Audio Editor

  UnwritableLocationError.h

  Dmitry Vedenko

**********************************************************************/

#pragma once

	
#include "widgets/wxPanelWrapper.h"

/// An error dialog about unwritable location, that allows to navigate to settings quickly
class UnwritableLocationErrorDialog final : public wxDialogWrapper
{
public:
   explicit UnwritableLocationErrorDialog(wxWindow* parent, const wxString& path);
   ~UnwritableLocationErrorDialog() override;

   void OnOk(wxCommandEvent& event);
   void OnError(wxCommandEvent& event);

   DECLARE_EVENT_TABLE()
};
