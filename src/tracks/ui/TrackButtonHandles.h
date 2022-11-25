/**********************************************************************

Audacity: A Digital Audio Editor

WavelTrackButtonHandles.h

Paul Licameli split from TrackPanel.cpp

**********************************************************************/

#ifndef __AUDACITY_TRACK_BUTTON_HANDLES__
#define __AUDACITY_TRACK_BUTTON_HANDLES__

#include "../ui/ButtonHandle.h"

class wxMouseState;

class MinimizeButtonHandle final : public ButtonHandle
{
   MinimizeButtonHandle(const MinimizeButtonHandle&) = delete;

protected:
   Result CommitChanges
      (const wxMouseEvent &event, SaucedacityProject *pProject, wxWindow *pParent)
      override;

   TranslatableString Tip(
      const wxMouseState &state, SaucedacityProject &) const override;

public:
   explicit MinimizeButtonHandle
      ( const std::shared_ptr<Track> &pTrack, const wxRect &rect );

   MinimizeButtonHandle &operator=(const MinimizeButtonHandle&) = default;

   ~MinimizeButtonHandle() override;

   static UIHandlePtr HitTest
      (std::weak_ptr<MinimizeButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect, TrackPanelCell *pCell);
};

////////////////////////////////////////////////////////////////////////////////
class SelectButtonHandle final : public ButtonHandle
{
   SelectButtonHandle(const SelectButtonHandle&) = delete;

protected:
   Result CommitChanges
      (const wxMouseEvent &event, SaucedacityProject *pProject, wxWindow *pParent)
      override;

   TranslatableString Tip(
      const wxMouseState &state, SaucedacityProject &) const override;

public:
   explicit SelectButtonHandle
      ( const std::shared_ptr<Track> &pTrack, const wxRect &rect );

   SelectButtonHandle &operator=(const SelectButtonHandle&) = default;

   ~SelectButtonHandle() override;

   static UIHandlePtr HitTest
      (std::weak_ptr<SelectButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect, TrackPanelCell *pCell);
};

////////////////////////////////////////////////////////////////////////////////
class CloseButtonHandle final : public ButtonHandle
{
   CloseButtonHandle(const CloseButtonHandle&) = delete;

protected:
   Result CommitChanges
      (const wxMouseEvent &event, SaucedacityProject *pProject, wxWindow *pParent)
      override;

   TranslatableString Tip(
      const wxMouseState &state, SaucedacityProject &project) const override;

   bool StopsOnKeystroke () override { return true; }
   
public:
   explicit CloseButtonHandle
      ( const std::shared_ptr<Track> &pTrack, const wxRect &rect );

   CloseButtonHandle &operator=(const CloseButtonHandle&) = default;

   ~CloseButtonHandle() override;

   static UIHandlePtr HitTest
      (std::weak_ptr<CloseButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect, TrackPanelCell *pCell);
};

////////////////////////////////////////////////////////////////////////////////

class MenuButtonHandle final : public ButtonHandle
{
   MenuButtonHandle(const MenuButtonHandle&) = delete;

protected:
   Result CommitChanges
      (const wxMouseEvent &event, SaucedacityProject *pProject, wxWindow *pParent)
      override;

   TranslatableString Tip(
      const wxMouseState &state, SaucedacityProject &) const override;

public:
   explicit MenuButtonHandle
      ( const std::shared_ptr<TrackPanelCell> &pCell,
        const std::shared_ptr<Track> &pTrack, const wxRect &rect );

   MenuButtonHandle &operator=(const MenuButtonHandle&) = default;

   ~MenuButtonHandle() override;

   static UIHandlePtr HitTest
      (std::weak_ptr<MenuButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect,
       const std::shared_ptr<TrackPanelCell> &pCell);

private:
   std::weak_ptr<TrackPanelCell> mpCell;
};

#endif
