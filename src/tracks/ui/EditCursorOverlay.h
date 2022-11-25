/**********************************************************************

Audacity: A Digital Audio Editor

EditCursorOverlay.h

Paul Licameli split from TrackPanel.cpp

**********************************************************************/

#ifndef __AUDACITY_EDIT_CURSOR_OVERLAY__
#define __AUDACITY_EDIT_CURSOR_OVERLAY__

#include <memory>
#include "ClientData.h" // to inherit
#include "../../widgets/Overlay.h" // to inherit

class SaucedacityProject;

class EditCursorOverlay final
   : public Overlay
   , public ClientData::Base
{
public:
   explicit
   EditCursorOverlay(SaucedacityProject *project, bool isMaster = true);

private:
   [[nodiscard]] unsigned SequenceNumber() const override;
   std::pair<wxRect, bool> DoGetRectangle(wxSize size) override;
   void Draw(OverlayPanel &panel, wxDC &dc) override;

   SaucedacityProject *mProject;
   bool mIsMaster;
   std::shared_ptr<EditCursorOverlay> mPartner;

   int mLastCursorX;
   double mCursorTime;
   int mNewCursorX;
};

#endif
