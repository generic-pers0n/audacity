/**********************************************************************

  Audacity: A Digital Audio Editor

  Journal.h

  Paul Licameli

**********************************************************************/

#ifndef __AUDACITY_JOURNAL__
#define __AUDACITY_JOURNAL__

#include <initializer_list>

// Tenacity libraries
#include <lib-exceptions/TenacityException.h>
#include <lib-strings/Identifier.h>

class wxArrayString;
class wxArrayStringEx;
class wxString;


// Whether the journalling feature is shown to the end user
#undef END_USER_JOURNALLING

namespace Journal
{
   //\brief Whether recording is enabled; but recording will happen only if this
   // was true at application start up
   bool RecordEnabled();

   //\brief Change the enablement of recording and store in preferences
   //\return whether successful
   bool SetRecordEnabled(bool value);

   //\brief Whether actually replaying.
   // IsRecording() && IsReplaying() is possible
   bool IsReplaying();

   //\brief Set the played back journal file at start up
   void SetInputFileName( const wxString &path );

   //\brief Initialize playback if a file name has been set, and initialize
   // output if recording is enabled.
   // Must be called after wxWidgets initializes.
   // Application initialization is late enough.
   // @param dataDir the output journal.txt will be in this directory, and the
   // input file, if it was relative, is made absolute with respect to it
   // @return true if successful
   bool Begin( const FilePath &dataDir );

   //\brief Consume next line from the input journal (skipping blank lines and
   // comments) and tokenize it.
   // Throws SyncException if no next line or not replaying
   wxArrayStringEx GetTokens();

   //\brief if playing back and commands remain, may execute one.
   // May throw SyncException if playing back but none remain, or if other error
   // conditions are encountered.
   // Returns true if any command was dispatched
   bool Dispatch();

   //\brief If recording, output the strings; if playing back, require
   // identical strings.  None of them may contain newlines
   void Sync( const wxString &string );
   void Sync( const wxArrayString &strings );
   void Sync( std::initializer_list< const wxString > strings );

   //\brief Get the value that the application will return to the command line
   int GetExitCode();

   //\brief thrown when playback of a journal doesn't match the recording
   class SyncException : public TenacityException {
   public:
      SyncException();
      ~SyncException() override;

      // The delayed handler action forces the program to quit gracefully,
      // so that the test playback failure is prompty reported.  This is
      // unlike other AudacityExceptions that roll back the project and
      // continue.
      void DelayedHandlerAction() override;
   };
}

#endif
