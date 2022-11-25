/**********************************************************************

  Audacity: A Digital Audio Editor

  GUIPrefs.h

  Brian Gunlogson
  Joshua Haberman
  James Crook

**********************************************************************/

#ifndef __AUDACITY_GUI_PREFS__
#define __AUDACITY_GUI_PREFS__

#include <wx/defs.h>

#include "PrefsPanel.h"

class ChoiceSetting;
class ShuttleGui;

#define GUI_PREFS_PLUGIN_SYMBOL ComponentInterfaceSymbol{ XO("GUI") }

class SAUCEDACITY_DLL_API GUIPrefs final : public PrefsPanel
{
 public:
   GUIPrefs(wxWindow * parent, wxWindowID winid);
   ~GUIPrefs() override;
   ComponentInterfaceSymbol GetSymbol() override;
   TranslatableString GetDescription() override;

   bool Commit() override;
   ManualPageID HelpPageName() override;
   void PopulateOrExchange(ShuttleGui & S) override;

   static void GetRangeChoices(
      TranslatableStrings *pChoices,
      wxArrayStringEx *pCodes,
      int *pDefaultRangeIndex = nullptr
   );

   // If no input language given, defaults to system language.
   // Returns the language actually used which is not lang if lang cannot be found.
   static wxString SetLang( const wxString & lang );

 private:
   void Populate();

   wxArrayStringEx mLangCodes;
   TranslatableStrings mLangNames;

   wxArrayStringEx mRangeCodes;
   TranslatableStrings mRangeChoices;
   int mDefaultRangeIndex;
};

SAUCEDACITY_DLL_API
int ShowClippingPrefsID();
SAUCEDACITY_DLL_API
int ShowTrackNameInWaveformPrefsID();

extern SAUCEDACITY_DLL_API ChoiceSetting
     GUIManualLocation
;

#endif
