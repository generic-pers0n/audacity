/*!********************************************************************
*
 Audacity: A Digital Audio Editor

 WaveTrackAffordanceControls.h

 Vitaly Sverchinsky

 **********************************************************************/

#pragma once

#include <wx/font.h>
#include "../../../ui/CommonTrackPanelCell.h"

class AffordanceHandle;
class WaveClip;

class SAUCEDACITY_DLL_API WaveTrackAffordanceControls : public CommonTrackCell
{
    std::weak_ptr<WaveClip> mFocusClip;
    std::weak_ptr<AffordanceHandle> mAffordanceHandle;
    std::weak_ptr<UIHandle> mResizeHandle;
    wxFont mClipNameFont;

public:
    WaveTrackAffordanceControls(const std::shared_ptr<Track>& pTrack);

    std::vector<UIHandlePtr> HitTest(const TrackPanelMouseState& state, const SaucedacityProject* pProject) override;

    void Draw(TrackPanelDrawingContext& context, const wxRect& rect, unsigned iPass) override;

    std::weak_ptr<WaveClip> GetSelectedClip() const;
};
