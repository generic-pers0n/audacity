/**********************************************************************

  Audacity: A Digital Audio Editor

  TracksBehaviorsPrefs.h

  Brian Gunlogson
  Joshua Haberman
  James Crook
  Steve Daulton

**********************************************************************/

#ifndef __AUDACITY_TRACKSBEHAVIORS_PREFS__
#define __AUDACITY_TRACKSBEHAVIORS_PREFS__

#include "PrefsPanel.h"

class ChoiceSetting;
class ShuttleGui;

#define TRACKS_BEHAVIORS_PREFS_PLUGIN_SYMBOL ComponentInterfaceSymbol{ XO("Tracks Behaviors") }

class SAUCEDACITY_DLL_API TracksBehaviorsPrefs final : public PrefsPanel
{
 public:
   TracksBehaviorsPrefs(wxWindow * parent, wxWindowID winid);
   ~TracksBehaviorsPrefs() override;
   ComponentInterfaceSymbol GetSymbol() override;
   TranslatableString GetDescription() override;

   bool Commit() override;
   ManualPageID HelpPageName() override;

   static const wxChar *ScrollingPreferenceKey();
   static inline bool ScrollingPreferenceDefault() { return false; }

 private:
   void Populate();
   void PopulateOrExchange(ShuttleGui & S) override;
};

extern SAUCEDACITY_DLL_API ChoiceSetting TracksBehaviorsSolo;

SAUCEDACITY_DLL_API bool GetEditClipsCanMove();

#endif
