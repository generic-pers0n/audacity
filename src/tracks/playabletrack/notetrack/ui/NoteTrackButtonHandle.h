/**********************************************************************

Audacity: A Digital Audio Editor

NoteTrackButtonHandle.h

Paul Licameli split from TrackPanel.cpp

**********************************************************************/

#ifndef __AUDACITY_NOTE_TRACK_BUTTON_HANDLE__
#define __AUDACITY_NOTE_TRACK_BUTTON_HANDLE__

class wxMouseState;
class NoteTrack;

#include "../../../../UIHandle.h"

///////////////////////////////////////////////////////////////////////////////
// TODO: do appearance changes as in ButtonHandle, or even, inherit from that
class NoteTrackButtonHandle : public UIHandle
{
   NoteTrackButtonHandle(const NoteTrackButtonHandle&);
   NoteTrackButtonHandle();
   static NoteTrackButtonHandle& Instance();

public:
   explicit NoteTrackButtonHandle
      ( const std::shared_ptr<NoteTrack> &pTrack,
        int channel, const wxRect &rect );

   NoteTrackButtonHandle &operator=(const NoteTrackButtonHandle&) = default;

   ~NoteTrackButtonHandle() override;

   static UIHandlePtr HitTest
      (std::weak_ptr<NoteTrackButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect,
       const std::shared_ptr<NoteTrack> &pTrack);

   [[nodiscard]] std::shared_ptr<NoteTrack> GetTrack() const { return mpTrack.lock(); }
   [[nodiscard]] int GetChannel() const { return mChannel; }

   static UIHandle::Result NeedChangeHighlight
      (const NoteTrackButtonHandle &oldState,
       const NoteTrackButtonHandle &newState);

protected:
   void Enter(bool forward, SaucedacityProject *) override;

   Result Click
      (const TrackPanelMouseEvent &event, SaucedacityProject *pProject) override;

   Result Drag
      (const TrackPanelMouseEvent &event, SaucedacityProject *pProject) override;

   HitTestPreview Preview
      (const TrackPanelMouseState &state, SaucedacityProject *pProject)
      override;

   Result Release
      (const TrackPanelMouseEvent &event, SaucedacityProject *pProject,
       wxWindow *pParent) override;

   Result Cancel(SaucedacityProject *pProject) override;

   std::weak_ptr<NoteTrack> mpTrack;
   int mChannel{ -1 };
   wxRect mRect{};
};

#endif
