/**********************************************************************

   Audacity - A Digital Audio Editor
   Copyright 1999-2018 Audacity Team
   License: GPL v2 - see LICENSE.txt

   Dominic Mazzoni
   Dan Horgan
   James Crook

**********************************************************************/

#ifndef __SCREENSHOT_COMMAND__
#define __SCREENSHOT_COMMAND__

#include "Command.h"

#include <wx/colour.h> // member variable

class wxWindow;
class wxTopLevelWindow;
class wxCommandEvent;
class wxRect;
class ToolManager;
class CommandOutputTargets;
class TrackPanel;
class AdornedRulerPanel;
class SaucedacityProject;
class CommandContext;

class SAUCEDACITY_DLL_API ScreenshotCommand : public AudacityCommand
{
public:
   enum kBackgrounds
   {
      kBlue,
      kWhite,
      kNone,
      nBackgrounds
   };

   enum kCaptureTypes
   {
      kwindow,
      kfullwindow,
      kwindowplus,
      kfullscreen,
      ktoolbars,
      keffects,
      kscriptables,
      kpreferences,
      kselectionbar,
      kspectralselection,
      ktimer,
      ktools,
      ktransport,
      kmeter,
      kplaymeter,
      krecordmeter,
      kedit,
      kdevice,
      kscrub,
      ktranscription,
      ktrackpanel,
      kruler,
      ktracks,
      kfirsttrack,
      kfirsttwotracks,
      kfirstthreetracks,
      kfirstfourtracks,
      ksecondtrack,
      ktracksplus,
      kfirsttrackplus,
      kalltracks,
      kalltracksplus,
      nCaptureWhats
   };

   static const ComponentInterfaceSymbol Symbol;

   ScreenshotCommand();
   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override { return "Takes screenshots."; };
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;

   // AudacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#screenshot_short_format";}

private:
   int mWhat;
   int mBack;
   wxString mPath;
   bool mbBringToTop;
   bool bHasBackground;
   bool bHasBringToTop;
   friend class ScreenshotBigDialog;

public:
   bool Apply(const CommandContext & context) override;
   void GetDerivedParams();

private:
   // May need to ignore the screenshot dialog
   // Appears not to be used anymore.
   wxWindow *mIgnore;

   bool mBackground;
   wxColour mBackColor;
   wxString mDirToWriteTo;

   wxString mFilePath;
   wxString mFileName;
   int mCaptureMode;

   static wxString MakeFileName(const wxString &path, const wxString &basename);

   static wxRect GetBackgroundRect();

   bool CaptureToolbar(const CommandContext & Context, ToolManager *man, int type, const wxString &name);
   bool CaptureDock(const CommandContext & Context, wxWindow *win, const wxString &fileName);
   void CaptureCommands(const CommandContext & Context, const wxArrayStringEx &Commands  );
   void CaptureEffects(const CommandContext & Context, SaucedacityProject * pProject, const wxString &fileName );
   void CaptureScriptables(const CommandContext & Context, SaucedacityProject * pProject, const wxString &fileName );
   void CapturePreferences(const CommandContext & Context, SaucedacityProject * pProject, const wxString &fileName );
   bool Capture(
      const CommandContext & Context,
      const wxString &basename,
         wxWindow *window, wxRect rect, 
         bool bg = false);
   static wxRect GetWindowRect(wxTopLevelWindow *w);
   wxRect GetFullWindowRect(wxTopLevelWindow *w);
   static wxRect GetScreenRect();
   static wxRect GetPanelRect(TrackPanel * panel);
   static wxRect GetRulerRect(AdornedRulerPanel *ruler);
   static wxRect GetTracksRect(TrackPanel * panel);
   static wxRect GetTrackRect( SaucedacityProject * pProj, TrackPanel * panel,int n);
   wxString WindowFileName(SaucedacityProject * proj, wxTopLevelWindow *w);

public:
   static ScreenshotCommand * mpShooter;
   static void (*mIdleHandler)(wxIdleEvent& event);
   static void SetIdleHandler( SaucedacityProject &project );
   static bool MayCapture( wxDialog * pDlg );

   void CaptureWindowOnIdle( const CommandContext & context, wxWindow * pWin );
   wxTopLevelWindow *GetFrontWindow(SaucedacityProject *project);
};

#endif /* End of include guard: __SCREENSHOT_COMMAND__ */
