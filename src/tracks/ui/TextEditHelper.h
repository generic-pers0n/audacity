/*!********************************************************************
*
 Audacity: A Digital Audio Editor

 TextEditHelper.h

 Vitaly Sverchinsky

 **********************************************************************/

#pragma once

#include <wx/string.h>
#include <wx/font.h>

#include "UIHandle.h"
#include <wx/colour.h>

class wxDC;
class wxMouseEvent;

class TenacityProject;
class TextEditDelegate;
class Track;

//Used as a delegate for TextEditHelper
class TextEditDelegate
{
public:

    virtual ~TextEditDelegate();

    virtual void OnTextEditFinished(TenacityProject* project, const wxString& text) = 0;
    virtual void OnTextEditCancelled(TenacityProject* project) = 0;
    virtual void OnTextModified(TenacityProject* project, const wxString& text) = 0;
    virtual void OnTextContextMenu(TenacityProject* project, const wxPoint& position) = 0;

};


//Used as a helper object for drawing, editing
//and text navigation in TrackPanel
class TextEditHelper
{
    wxString mText;
    wxFont mFont;
    wxRect mBBox;

    wxColor mTextSelectionColor;
    wxColor mTextColor;

    bool mRightDragging{ false };
    //Index of the symbol from which text drawing should start,
    //in cases where the whole text cannot fit into provided "view"
    //rectangle. Used both in hit testing and drawing.
    int mOffset{ 0 };
    int mInitialCursorPos{ 0 };
    int mCurrentCursorPos{ 0 };

    // TextEditHelper will send messages about changes to the object
    // that implements TextEditDelegate, if present
    std::weak_ptr<TextEditDelegate> mDelegate;

public:
    static bool IsGoodEditKeyCode(int keyCode);

    TextEditHelper(const std::weak_ptr<TextEditDelegate>& delegate, const wxString& text, const wxFont& font);
   
   ~TextEditHelper()
   {
   }

    void SetTextColor(const wxColor& textColor);
    void SetTextSelectionColor(const wxColor& textSelectionColor);

    void Cancel(TenacityProject* project);
    void Finish(TenacityProject* project);

    std::pair<int, int> GetSelection() const;
    void SetSelection(int from, int to);
    void SelectAll();
    bool IsSelectionEmpty();

    bool CaptureKey(int keyCode, int mods);
    bool OnKeyDown(int keyCode, int mods, TenacityProject* project);
    bool OnChar(int charCode, TenacityProject* project);

    bool OnClick(const wxMouseEvent& event, TenacityProject* project);
    bool OnDrag(const wxMouseEvent& event, TenacityProject* project);
    bool OnRelease(const wxMouseEvent& event, TenacityProject* project);

    void Draw(wxDC& dc, const wxRect& rect);

    bool CutSelectedText(TenacityProject& project);
    bool CopySelectedText(TenacityProject& project);
    bool PasteSelectedText(TenacityProject& project);

    bool GetCharPositionX(int index, int* outX);

    const wxRect& GetBBox() const;

protected:

    bool HandleDragRelease(const wxMouseEvent& event, TenacityProject* project);

    void RemoveSelectedText(TenacityProject* project);
    int FindCursorIndex(const wxPoint& point);
    
};
