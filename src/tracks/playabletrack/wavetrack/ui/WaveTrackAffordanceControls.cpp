/*!********************************************************************
*
 Audacity: A Digital Audio Editor

 WaveTrackAffordanceControls.cpp

 Vitaly Sverchinsky

 **********************************************************************/

#include "WaveTrackAffordanceControls.h"

#include <wx/dc.h>

#include "../../../../theme/AllThemeResources.h"
#include "../../../../TrackPanelMouseEvent.h"
#include "../../../../TrackArtist.h"
#include "../../../../TrackPanelDrawingContext.h"
#include "../../../../TrackPanelResizeHandle.h"
#include "../../../../ViewInfo.h"
#include "../../../../WaveTrack.h"
#include "../../../../WaveClip.h"
#include "../../../ui/AffordanceHandle.h"
#include "WaveTrackView.h"//need only ClipParameters

#include "../../../../ProjectHistory.h"
#include "../../../../SelectionState.h"
#include "../../../../RefreshCode.h"

class WaveTrackAffordanceHandle final : public AffordanceHandle
{
public:
    WaveTrackAffordanceHandle(const std::shared_ptr<Track>& track) : AffordanceHandle(track) { }

    static UIHandlePtr HitAnywhere(std::weak_ptr<AffordanceHandle>& holder, const std::shared_ptr<Track>& pTrack)
    {
        auto result = std::static_pointer_cast<AffordanceHandle>(std::make_shared<WaveTrackAffordanceHandle>(pTrack));
        result = AssignUIHandlePtr(holder, result);
        return result;
    }

    UIHandle::Result SelectAt(const TrackPanelMouseEvent& event, SaucedacityProject* pProject) override
    {
        const auto track = std::dynamic_pointer_cast<WaveTrack>(TrackList::Get(*pProject).Lock<Track>(GetTrack()));

        auto& viewInfo = ViewInfo::Get(*pProject);

        auto time = viewInfo.PositionToTime(event.event.m_x, event.rect.x);
        
        WaveClip* const selectedClip = track->GetClipAtTime(time);
        if (selectedClip) {
            viewInfo.selectedRegion.setTimes(selectedClip->GetOffset(), selectedClip->GetEndTime());
        }

        ProjectHistory::Get(*pProject).ModifyState(false);

        return RefreshCode::RefreshAll | RefreshCode::Cancelled;
    }
};

WaveTrackAffordanceControls::WaveTrackAffordanceControls(const std::shared_ptr<Track>& pTrack)
    : CommonTrackCell(pTrack)
{
}

std::vector<UIHandlePtr> WaveTrackAffordanceControls::HitTest(const TrackPanelMouseState& state, const SaucedacityProject* pProject)
{
    mFocusClip.reset();

    std::vector<UIHandlePtr> results;

    auto px = state.state.m_x;
    auto py = state.state.m_y;

    const auto rect = state.rect;

    const auto track = FindTrack();

    auto trackList = track->GetOwner();
    if ((std::abs(rect.GetTop() - py) <= WaveTrackView::kChannelSeparatorThickness / 2) 
        && trackList
        && !track->IsLeader())
    {
        //given that track is not a leader there always should be
        //another track before this one
        auto prev = --trackList->Find(track.get());
        auto result = std::static_pointer_cast<UIHandle>(
            std::make_shared<TrackPanelResizeHandle>((*prev)->shared_from_this(), py)
        );
        result = AssignUIHandlePtr(mResizeHandle, result);
        results.push_back(result);
    }

    const auto waveTrack = std::static_pointer_cast<WaveTrack>(track->SubstitutePendingChangedTrack());

    auto& zoomInfo = ViewInfo::Get(*pProject);
    for (const auto& clip : waveTrack->GetClips())
    {
        auto affordanceRect = ClipParameters::GetClipRect(*clip.get(), zoomInfo, rect);

        if (affordanceRect.Contains(px, py))
        {
            results.push_back(WaveTrackAffordanceHandle::HitAnywhere(mAffordanceHandle, track));
            mFocusClip = clip;
            break;
        }
    }

    return results;
}

void WaveTrackAffordanceControls::Draw(TrackPanelDrawingContext& context, const wxRect& rect, unsigned iPass)
{
    if (iPass == TrackArtist::PassBackground) {
        auto track = FindTrack();
        const auto artist = TrackArtist::Get(context);

        TrackArt::DrawBackgroundWithSelection(context, rect, track.get(), artist->blankSelectedBrush, artist->blankBrush);

        const auto waveTrack = std::static_pointer_cast<WaveTrack>(track->SubstitutePendingChangedTrack());
        const auto& zoomInfo = *artist->pZoomInfo;

        context.dc.SetClippingRegion(rect);

        auto px = context.lastState.m_x;
        auto py = context.lastState.m_y;

        for (const auto& clip : waveTrack->GetClips())
        {
            auto affordanceRect 
                = ClipParameters::GetClipRect(*clip.get(), zoomInfo, rect);
            if (affordanceRect.IsEmpty())
                continue;
            
            auto selected = GetSelectedClip().lock() == clip;
            auto highlight = selected || affordanceRect.Contains(px, py);
            TrackArt::DrawClipAffordance(context.dc, affordanceRect, clip->GetName(), highlight, selected);

        }
        context.dc.DestroyClippingRegion();
    }
}

std::weak_ptr<WaveClip> WaveTrackAffordanceControls::GetSelectedClip() const
{
    if (auto handle = mAffordanceHandle.lock())
    {
        return handle->Clicked() ? mFocusClip : std::weak_ptr<WaveClip>();
    }
    return {};
}
