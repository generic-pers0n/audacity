/*!********************************************************************
*
 Audacity: A Digital Audio Editor

 WaveTrackAffordanceControls.cpp

 Vitaly Sverchinsky

 **********************************************************************/

#include "WaveTrackAffordanceControls.h"

#include <wx/dc.h>
#include <wx/frame.h>

#include "../../../../theme/AllThemeResources.h"
#include "../../../../TrackPanelMouseEvent.h"
#include "../../../../TrackArtist.h"
#include "../../../../TrackPanelDrawingContext.h"
#include "../../../../TrackPanelResizeHandle.h"
#include "../../../../ViewInfo.h"
#include "../../../../WaveTrack.h"
#include "../../../../WaveClip.h"
#include "../../../../UndoManager.h"
#include "../../../../shuttle/ShuttleGui.h"
#include "../../../../Project.h"
#include "../../../../ProjectWindow.h"
#include "../../../../commands/AudacityCommand.h"
#include "../../../ui/AffordanceHandle.h"
#include "../../../ui/TextEditHelper.h"
#include "WaveTrackView.h"//need only ClipParameters

#include "../../../../ProjectHistory.h"
#include "../../../../SelectionState.h"
#include "../../../../RefreshCode.h"
#include "../../../../theme/Theme.h"
#include "../../../../images/Cursors.h"
#include "../../../../HitTestResult.h"

class WaveTrackAffordanceHandle final : public AffordanceHandle
{
    std::shared_ptr<WaveClip> mTarget;
public:
    WaveTrackAffordanceHandle(const std::shared_ptr<Track>& track, const std::shared_ptr<WaveClip>& target) 
        : AffordanceHandle(track), mTarget(target)
    { }

    Result Click(const TrackPanelMouseEvent& event, SaucedacityProject* project) override
    {
        if (event.event.LeftDClick())
        {
            auto& viewInfo = ViewInfo::Get(*project);
            auto affordanceRect = ClipParameters::GetClipRect(*mTarget.get(), viewInfo, event.rect);
            if (affordanceRect.Contains(event.event.GetPosition()))
            {
                if (auto affordanceControl = std::dynamic_pointer_cast<WaveTrackAffordanceControls>(event.pCell))
                {
                    return affordanceControl->StartEditClipName(project) ? 
                        (RefreshCode::RefreshCell | RefreshCode::Cancelled) : RefreshCode::RefreshNone;
                }
            }
            return RefreshCode::RefreshNone;
        }
        else
            return AffordanceHandle::Click(event, project);
    }

    UIHandle::Result SelectAt(const TrackPanelMouseEvent& event, SaucedacityProject* project) override
    {
        auto& viewInfo = ViewInfo::Get(*project);
        viewInfo.selectedRegion.setTimes(mTarget->GetOffset(), mTarget->GetEndTime());
        
        ProjectHistory::Get(*project).ModifyState(false);
        
        return RefreshCode::RefreshAll | RefreshCode::Cancelled;
    }
};

class SetWaveClipNameCommand : public AudacityCommand
{
public:
    static const ComponentInterfaceSymbol Symbol;

    ComponentInterfaceSymbol GetSymbol() override
    {
        return Symbol;
    }
    void PopulateOrExchange(ShuttleGui& S) override
    {
        S.AddSpace(0, 5);

        S.StartMultiColumn(2, wxALIGN_CENTER);
        {
            S.TieTextBox(XXO("Name:"), mName, 60);
        }
        S.EndMultiColumn();
    }
public:
    wxString mName;
};

const ComponentInterfaceSymbol SetWaveClipNameCommand::Symbol
{ XO("Set Wave Clip Name") };

//Handle which is used to send mouse events to TextEditHelper
class WaveClipTitleEditHandle final : public UIHandle
{
    std::shared_ptr<TextEditHelper> mHelper;
public:

    WaveClipTitleEditHandle(const std::shared_ptr<TextEditHelper>& helper)
        : mHelper(helper)
    { }

    Result Click(const TrackPanelMouseEvent& event, SaucedacityProject* project) override
    {
        if (mHelper->OnClick(event.event, project))
            return RefreshCode::RefreshCell;
        return RefreshCode::RefreshNone;
    }

    Result Drag(const TrackPanelMouseEvent& event, SaucedacityProject* project) override
    {
        if (mHelper->OnDrag(event.event, project))
            return RefreshCode::RefreshCell;
        return RefreshCode::RefreshNone;
    }

    HitTestPreview Preview(const TrackPanelMouseState& state, SaucedacityProject* pProject) override
    {
        static auto ibeamCursor =
            ::MakeCursor(wxCURSOR_IBEAM, IBeamCursorXpm, 17, 16);
        return {
           XO("Click and drag to select text"),
           ibeamCursor.get()
        };
    }

    Result Release(const TrackPanelMouseEvent& event, SaucedacityProject* project, wxWindow*) override
    {
        if (mHelper->OnRelease(event.event, project))
            return RefreshCode::RefreshCell;
        return RefreshCode::RefreshNone;
    }

    Result Cancel(SaucedacityProject* project) override
    {
        if (mHelper)
        {
            mHelper->Cancel(project);
            mHelper.reset();
        }
        return RefreshCode::RefreshAll;
    }
};

WaveTrackAffordanceControls::WaveTrackAffordanceControls(const std::shared_ptr<Track>& pTrack)
    : CommonTrackCell(pTrack), mClipNameFont(wxFont(wxFontInfo()))
{
    if (auto trackList = pTrack->GetOwner())
    {
        trackList->Bind(EVT_TRACKLIST_SELECTION_CHANGE,
            &WaveTrackAffordanceControls::OnTrackChanged,
            this);
    }
}

std::vector<UIHandlePtr> WaveTrackAffordanceControls::HitTest(const TrackPanelMouseState& state, const SaucedacityProject* pProject)
{
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
        results.push_back(
            AssignUIHandlePtr(
                mResizeHandle, 
                std::make_shared<TrackPanelResizeHandle>((*prev)->shared_from_this(), py)
            )
        );
    }

    if (mTextEditHelper)
    {
        if(mTextEditHelper->GetBBox().Contains(px, py))
            results.push_back(
                AssignUIHandlePtr(
                    mTitleEditHandle,
                    std::make_shared<WaveClipTitleEditHandle>(mTextEditHelper)
                )
            );
    }
    else
    {
        const auto waveTrack = std::static_pointer_cast<WaveTrack>(track->SubstitutePendingChangedTrack());
        std::shared_ptr<WaveClip> focusClip;
        auto& zoomInfo = ViewInfo::Get(*pProject);
        for (const auto& clip : waveTrack->GetClips())
        {
            auto affordanceRect = ClipParameters::GetClipRect(*clip.get(), zoomInfo, state.rect);
            if (affordanceRect.Contains(px, py))
            {
                results.push_back(
                    AssignUIHandlePtr(
                        mAffordanceHandle,
                        std::make_shared<WaveTrackAffordanceHandle>(track, clip)
                    )
                );
                focusClip = clip;
                break;
            }
        }
        mFocusClip = focusClip;
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

        context.dc.SetTextBackground(wxTransparentColor);
        context.dc.SetTextForeground(theTheme.Colour(clrClipNameText));
        context.dc.SetFont(mClipNameFont);

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
            if (mTextEditHelper && mFocusClip.lock() == clip)
            {
                TrackArt::DrawClipAffordance(context.dc, affordanceRect, wxEmptyString, highlight, selected);
                mTextEditHelper->Draw(context.dc, TrackArt::GetAffordanceTitleRect(affordanceRect));
            }
            else
               TrackArt::DrawClipAffordance(context.dc, affordanceRect, clip->GetName(), highlight, selected);

        }
        context.dc.DestroyClippingRegion();
    }
}

bool WaveTrackAffordanceControls::StartEditClipName(SaucedacityProject* project)
{
    if (auto lock = mFocusClip.lock())
    {
        auto clip = lock.get();

        bool useDialog{ false };
        gPrefs->Read(wxT("/GUI/DialogForNameNewLabel"), &useDialog, false);

        if (useDialog)
        {
            SetWaveClipNameCommand Command;
            auto oldName = clip->GetName();
            Command.mName = oldName;
            auto result = Command.PromptUser(&GetProjectFrame(*project));
            if (result && Command.mName != oldName)
            {
                clip->SetName(Command.mName);
                ProjectHistory::Get(*project).PushState(XO("Modified Clip Name"),
                    XO("Clip Name Edit"), UndoPush::CONSOLIDATE);

                return true;
            }
        }
        else
        {
            mTextEditHelper = MakeTextEditHelper(clip->GetName());
            return true;
        }
    }
    return false;
}

std::weak_ptr<WaveClip> WaveTrackAffordanceControls::GetSelectedClip() const
{
    if (auto handle = mAffordanceHandle.lock())
    {
        return handle->Clicked() ? mFocusClip : std::weak_ptr<WaveClip>();
    }
    return {};
}

unsigned WaveTrackAffordanceControls::CaptureKey(wxKeyEvent& event, ViewInfo& viewInfo, wxWindow* pParent, SaucedacityProject* project)
{
    const auto keyCode = event.GetKeyCode();
    if (!mTextEditHelper && !(keyCode == WXK_RETURN || keyCode == WXK_NUMPAD_ENTER || keyCode == WXK_TAB))
        event.Skip();
    return RefreshCode::RefreshNone;
}


unsigned WaveTrackAffordanceControls::KeyDown(wxKeyEvent& event, ViewInfo& viewInfo, wxWindow*, SaucedacityProject* project)
{
    auto keyCode = event.GetKeyCode();
    
    if (mTextEditHelper)
    {
        mTextEditHelper->OnKeyDown(keyCode, event.GetModifiers(), project);
        if (!TextEditHelper::IsGoodEditKeyCode(keyCode))
            event.Skip();
    }
    else
    {
        switch (keyCode)
        {
        case WXK_TAB: {
            SelectNextClip(viewInfo, project, event.GetModifiers() != wxMOD_SHIFT);
        } break;
        case WXK_NUMPAD_ENTER:
        case WXK_RETURN:
            StartEditSelectedClipName(viewInfo, project);
            break;
        }
    }
    return RefreshCode::RefreshCell;
}

unsigned WaveTrackAffordanceControls::Char(wxKeyEvent& event, ViewInfo& viewInfo, wxWindow* pParent, SaucedacityProject* project)
{
    if (mTextEditHelper && mTextEditHelper->OnChar(event.GetUnicodeKey(), project))
        return RefreshCode::RefreshCell;
    return RefreshCode::RefreshNone;
}

void WaveTrackAffordanceControls::OnTextEditFinished(SaucedacityProject* project, const wxString& text)
{
    if (auto lock = mFocusClip.lock())
    {
        lock->SetName(text);

        ProjectHistory::Get(*project).PushState(XO("Modified Clip Name"),
            XO("Clip Name Edit"), UndoPush::CONSOLIDATE);
    }
    mTextEditHelper.reset();
}

void WaveTrackAffordanceControls::OnTextEditCancelled(SaucedacityProject* project)
{
    mTextEditHelper.reset();
}

void WaveTrackAffordanceControls::OnTextModified(SaucedacityProject* project, const wxString& text)
{
    //Nothing to do
}

void WaveTrackAffordanceControls::OnTextContextMenu(SaucedacityProject* project, const wxPoint& position)
{
}

void WaveTrackAffordanceControls::OnTrackChanged(TrackListEvent& evt)
{
    if (mTextEditHelper)
    {
        auto trackList = FindTrack()->GetOwner();
        if (trackList)
        {
            mTextEditHelper->Finish(trackList->GetOwner());
        }
        mTextEditHelper.reset();
    }
}



namespace {
    template<typename Iter, typename Comp>
    WaveClip* NextClipLooped(ViewInfo& viewInfo, Iter begin, Iter end, Comp comp)
    {
        auto it = std::find_if(begin, end, [&](WaveClip* clip) {
            return clip->GetStartTime() == viewInfo.selectedRegion.t0() &&
                clip->GetEndTime() == viewInfo.selectedRegion.t1();
        });
        if (it == end)
            it = std::find_if(begin, end, comp);
        else
            it = std::next(it);
        
        if (it == end)
            return *begin;
        return *it;
    }
}


bool WaveTrackAffordanceControls::SelectNextClip(ViewInfo& viewInfo, SaucedacityProject* project, bool forward)
{
    //Iterates through clips in a looped manner
    auto waveTrack = std::dynamic_pointer_cast<WaveTrack>(FindTrack());
    if (!waveTrack)
        return false;
    auto clips = waveTrack->SortedClipArray();
    if (clips.empty())
        return false;

    WaveClip* clip{ };
    if (forward)
    {
        clip = NextClipLooped(viewInfo, clips.begin(), clips.end(), [&](const WaveClip* other) {
            return other->GetStartTime() >= viewInfo.selectedRegion.t1();
        });
    }
    else
    {
        clip = NextClipLooped(viewInfo, clips.rbegin(), clips.rend(), [&](const WaveClip* other) {
            return other->GetStartTime() <= viewInfo.selectedRegion.t0();
        });
    }

    viewInfo.selectedRegion.setTimes(clip->GetOffset(), clip->GetEndTime());
    ProjectHistory::Get(*project).ModifyState(false);
    return true;
}

bool WaveTrackAffordanceControls::StartEditSelectedClipName(ViewInfo& viewInfo, SaucedacityProject* project)
{
    //Attempts to invoke name editing if there is a selected clip
    auto waveTrack = std::dynamic_pointer_cast<WaveTrack>(FindTrack());
    if (!waveTrack)
        return false;
    auto clips = waveTrack->GetClips();

    auto it = std::find_if(clips.begin(), clips.end(), [&](const std::shared_ptr<WaveClip>& clip) {
        return clip->GetStartTime() == viewInfo.selectedRegion.t0() &&
            clip->GetEndTime() == viewInfo.selectedRegion.t1();
        });
    if (it != clips.end())
    {
        mFocusClip = *it;
        return StartEditClipName(project);
    }
    return false;
}

std::shared_ptr<TextEditHelper> WaveTrackAffordanceControls::MakeTextEditHelper(const wxString& text)
{
    auto helper = std::make_shared<TextEditHelper>(shared_from_this(), text, mClipNameFont);
    helper->SetTextColor(theTheme.Colour(clrClipNameText));
    helper->SetTextSelectionColor(theTheme.Colour(clrClipNameTextSelection));
    return helper; 
}

