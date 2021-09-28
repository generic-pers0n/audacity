/**********************************************************************

  Audacity: A Digital Audio Editor

  SpectralDataDialog.cpp

  Edward Hui

*******************************************************************//*!

\class SpectralDataDialog
\brief Provides UI for spectral editing and parameters adjustments

*//*******************************************************************/

#ifdef EXPERIMENTAL_BRUSH_TOOL

#include <wx/app.h>
#include <wx/defs.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/frame.h>
#include <wx/imaglist.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "../images/Arrow.xpm"
#include "../images/Empty9x16.xpm"

#include "AudioIO.h"
#include "ClientData.h"
#include "Clipboard.h"
#include "commands/CommandContext.h"
#include "commands/CommandManager.h"
#include "Menus.h"
#include "UndoManager.h"
#include "Prefs.h"
#include "Project.h"
#include "ProjectFileIO.h"
#include "ProjectHistory.h"
#include "ProjectSettings.h"
#include "ProjectWindows.h"
#include "ShuttleGui.h"
#include "SpectralDataManager.h"
#include "TrackPanel.h"

#include "commands/CommandManagerWindowClasses.h"
#include "widgets/AudacityMessageBox.h"
#include "widgets/wxPanelWrapper.h" // to inherit

enum {
   ID_CHECKBOX_SMART = 10000,
   ID_CHECKBOX_OVERTONES,
   ID_SLIDER_BRUSH_SIZE
};

class wxButton;
class wxCheckBox;
class wxListCtrl;
class wxListEvent;
class wxSpinCtrl;
class wxTextCtrl;
class AudacityProject;
class ShuttleGui;
class UndoManager;

class SpectralDataDialog final : public wxDialogWrapper,
      public PrefsListener,
      public ClientData::Base,
      public TopLevelKeystrokeHandlingWindow
{

      public:
         static SpectralDataDialog &Get(AudacityProject &project);
         static SpectralDataDialog *Find(AudacityProject *pProject);

         explicit SpectralDataDialog(AudacityProject &parent);

         void UpdateDisplay(wxEvent &e);

         bool Show( bool show = true ) override;

      private:
         void Populate(ShuttleGui & S);

         void OnAudioIO(wxCommandEvent & evt);
         void DoUpdate();

         void OnCloseWindow(wxCloseEvent &event);
         void OnApply(wxCommandEvent &event);
         void OnBrushSizeSlider(wxCommandEvent &event);
         void OnCheckSmartSelection(wxCommandEvent &event);
         void OnCheckOvertones(wxCommandEvent &event);

         // PrefsListener implementation
         void UpdatePrefs() override;

         AudacityProject   &mProject;
         bool              mAudioIOBusy { false };

      public:
         DECLARE_EVENT_TABLE()
};

class AUDACITY_DLL_API SpectralDataDialogWorker final
      : public ClientData::Base{
public:
   explicit SpectralDataDialogWorker( AudacityProject &project );

   void OnToolChanged(wxCommandEvent &evt);
private:
   AudacityProject &mProject;
};

BEGIN_EVENT_TABLE(SpectralDataDialog, wxDialogWrapper)
   EVT_CHECKBOX(ID_CHECKBOX_SMART, SpectralDataDialog::OnCheckSmartSelection)
   EVT_CHECKBOX(ID_CHECKBOX_OVERTONES, SpectralDataDialog::OnCheckOvertones)
   EVT_CLOSE(SpectralDataDialog::OnCloseWindow)
   EVT_SLIDER(ID_SLIDER_BRUSH_SIZE, SpectralDataDialog::OnBrushSizeSlider)
END_EVENT_TABLE()

#define Title XO("Spectral Data Control Panel")

SpectralDataDialog::SpectralDataDialog(AudacityProject &parent)
   : mProject(parent)
   , wxDialogWrapper(FindProjectFrame( &parent ), wxID_ANY, Title,
                         wxDefaultPosition, wxDefaultSize,
                         wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )

{
   SetName();
   //------------------------- Main section --------------------
   // Construct the GUI.
   ShuttleGui S(this, eIsCreating);
   Populate(S);

   wxTheApp->Bind(EVT_AUDIOIO_PLAYBACK,
                  &SpectralDataDialog::OnAudioIO,
                  this);

   wxTheApp->Bind(EVT_AUDIOIO_CAPTURE,
                  &SpectralDataDialog::OnAudioIO,
                  this);

   Clipboard::Get().Bind(
         EVT_CLIPBOARD_CHANGE, &SpectralDataDialog::UpdateDisplay, this);
   parent.Bind(EVT_UNDO_PUSHED, &SpectralDataDialog::UpdateDisplay, this);
   parent.Bind(EVT_UNDO_MODIFIED, &SpectralDataDialog::UpdateDisplay, this);
   parent.Bind(EVT_UNDO_OR_REDO, &SpectralDataDialog::UpdateDisplay, this);
   parent.Bind(EVT_UNDO_RESET, &SpectralDataDialog::UpdateDisplay, this);
   parent.Bind(EVT_UNDO_PURGE, &SpectralDataDialog::UpdateDisplay, this);
}

static const AttachedWindows::RegisteredFactory key{
      []( AudacityProject &project ){
         return safenew SpectralDataDialog( project );
      }
};

void SpectralDataDialog::Populate(ShuttleGui & S)
{
   auto imageList = std::make_unique<wxImageList>(9, 16);
   imageList->Add(wxIcon(empty9x16_xpm));
   imageList->Add(wxIcon(arrow_xpm));

   S.StartVerticalLay(true);
   {
      S.StartStatic(XO("Options"), 1);
      {
         S.Id(ID_CHECKBOX_OVERTONES)
               .AddCheckBox(XXO("Enable overtones selection"), false),
         S.Id(ID_CHECKBOX_SMART)
               .AddCheckBox(XXO("Enable smart selection"), false);
      }
      S.EndStatic();

      S.StartStatic(XO("Brush radius"), 1);
      {
         S.Id(ID_SLIDER_BRUSH_SIZE)
               .Style(wxSL_HORIZONTAL)
               .Name(XO("Custom brush size"))
               .AddSlider( {}, 5, 10, 1);
      }
      S.EndStatic();
   }
   S.EndVerticalLay();
   // ----------------------- End of main section --------------

   Layout();
   Fit();
   SetMinSize(GetSize());
}

void SpectralDataDialog::OnAudioIO(wxCommandEvent& evt)
{
   evt.Skip();

   if (evt.GetInt() != 0)
      mAudioIOBusy = true;
   else
      mAudioIOBusy = false;
}

void SpectralDataDialog::UpdateDisplay(wxEvent& e)
{
   e.Skip();
   if(IsShown())
      DoUpdate();
}

bool SpectralDataDialog::Show( bool show )
{
   if ( show && !IsShown())
      DoUpdate();
   auto result = wxDialogWrapper::Show( show );
   CommandManager::Get( mProject ).UpdateCheckmarks( mProject );
   return result;
}

void SpectralDataDialog::DoUpdate()
{
}

void SpectralDataDialog::OnCloseWindow(wxCloseEvent & WXUNUSED(event))
{
   this->Show(false);
}

// PrefsListener implementation
void SpectralDataDialog::UpdatePrefs()
{
   bool shown = IsShown();
   if (shown) {
      Show(false);
   }

   SetSizer(nullptr);
   DestroyChildren();

   SetTitle(Title);
   ShuttleGui S(this, eIsCreating);
   Populate(S);

   if (shown) {
      Show(true);
   }
}

static const AudacityProject::AttachedObjects::RegisteredFactory sSpectralWorkerKey{
   []( AudacityProject &project ){
      return std::make_shared< SpectralDataDialogWorker >( project );
   }
};

AttachedWindows::RegisteredFactory sSpectralDataDialogKey{
      []( AudacityProject &parent ) -> wxWeakRef< wxWindow > {
         return safenew SpectralDataDialog( parent );
      }
};

SpectralDataDialog &SpectralDataDialog::Get( AudacityProject &project )
{
   // Ensure existence of the dialog
   return static_cast<SpectralDataDialog&>(
      GetAttachedWindows(project).Get( sSpectralDataDialogKey ) );
}

SpectralDataDialog *SpectralDataDialog::Find( AudacityProject *pProject )
{
   // Return a pointer to the dialog only if it exists
   return pProject
      ? static_cast<SpectralDataDialog*>(
         GetAttachedWindows(*pProject).Find( sSpectralDataDialogKey ) )
      : nullptr;
}

// Current workflow for spectral editing dialog:
// 1. ProjectSettings change event listened by Worker::OnToolChanged()
// 2. In Worker::OnToolChanged(), get Dialog from AttachedWindows and invoke Show()
// 3. Dialog::OnApply() listens to the apply button in Dialog, which calculates and applies the effect
SpectralDataDialogWorker::SpectralDataDialogWorker(AudacityProject &project)
   : mProject{ project }
{
   project.Bind(EVT_PROJECT_SETTINGS_CHANGE, &SpectralDataDialogWorker::OnToolChanged, this);
}

void SpectralDataDialogWorker::OnToolChanged(wxCommandEvent &evt)
{
   evt.Skip();
   auto &projectSettings = ProjectSettings::Get( mProject );
   if (evt.GetInt() == ProjectSettings::ChangedTool)
   {
      // Find not Get to avoid creating the dialog if not yet done
      if (auto pDialog = SpectralDataDialog::Find(&mProject) )
         pDialog->Show(projectSettings.GetTool() == ToolCodes::brushTool);
   }
}

void SpectralDataDialog::OnBrushSizeSlider(wxCommandEvent &event) {
   auto &projectSettings = ProjectSettings::Get( mProject );
   projectSettings.SetBrushRadius(event.GetInt());
}

void SpectralDataDialog::OnCheckSmartSelection(wxCommandEvent &event){
   int isSelected = event.GetInt();
   wxASSERT(isSelected == 0 || isSelected == 1);
   auto &projectSettings = ProjectSettings::Get( mProject );
   projectSettings.SetSmartSelection(isSelected);
}

void SpectralDataDialog::OnCheckOvertones(wxCommandEvent &event){
   int isSelected = event.GetInt();
   wxASSERT(isSelected == 0 || isSelected == 1);
   auto &projectSettings = ProjectSettings::Get( mProject );
   projectSettings.SetOvertones(isSelected);
}

namespace {
struct Handler : CommandHandlerObject {
   void OnSpectralEditingPanel(const CommandContext &context)
   {
      auto &project = context.project;
      auto &dialog = SpectralDataDialog::Get(project);
      dialog.Show( !dialog.IsShown() );
   }
};

CommandHandlerObject &findCommandHandler(AudacityProject &) {
   // Handler is not stateful.  Doesn't need a factory registered with
   // AudacityProject.
   static Handler instance;
   return instance;
}

using namespace MenuTable;
MenuTable::AttachedItem sAttachment{
   wxT("View/Other/Toolbars/Toolbars/Other"),
   ( FinderScope{ findCommandHandler },
      Command( wxT("ShowSpectralSelectionPanel"),
         XXO("Spectra&l Selection Panel"),
         &Handler::OnSpectralEditingPanel,
         AlwaysEnabledFlag,
         CommandManager::Options{}
            .CheckTest( [](AudacityProject &project) {
               // Find not Get to avoid creating the dialog if not yet done
               auto pDialog = SpectralDataDialog::Find(&project);
               return pDialog && pDialog->IsShown();
            } ) )
   )
};

}

#endif
