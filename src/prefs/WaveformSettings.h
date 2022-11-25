/**********************************************************************

Audacity: A Digital Audio Editor

WaveformSettings.h

Paul Licameli

**********************************************************************/

#ifndef __AUDACITY_WAVEFORM_SETTINGS__
#define __AUDACITY_WAVEFORM_SETTINGS__

#include "Prefs.h"

class EnumValueSymbols;

class SAUCEDACITY_DLL_API WaveformSettings : public PrefsListener
{
public:

   // Singleton for settings that are not per-track
   class SAUCEDACITY_DLL_API Globals
   {
   public:
      static Globals &Get();
      void SavePrefs();

   private:
      Globals();
      void LoadPrefs();
   };

   static WaveformSettings &defaults();
   WaveformSettings();
   WaveformSettings(const WaveformSettings &other);
   WaveformSettings& operator= (const WaveformSettings &other);
   ~WaveformSettings() override;

   [[nodiscard]] bool IsDefault() const
   {
      return this == &defaults();
   }

   bool Validate(bool quiet);
   void LoadPrefs();
   void SavePrefs();
   void Update();

   void UpdatePrefs() override;

   void ConvertToEnumeratedDBRange();
   void ConvertToActualDBRange();
   void NextLowerDBRange();
   void NextHigherDBRange();

   typedef int ScaleType;
   enum ScaleTypeValues : int {
      stLinear,
      stLogarithmic,

      stNumScaleTypes,
   };

   static const EnumValueSymbols &GetScaleNames();

   ScaleType scaleType;
   int dBRange;

   // Convenience
   [[nodiscard]] bool isLinear() const { return stLinear == scaleType; }
};
#endif
