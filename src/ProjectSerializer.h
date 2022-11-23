/**********************************************************************

   Audacity: A Digital Audio Editor
   Audacity(R) is copyright (c) 1999-2010 Audacity Team.
   License: GPL v2.  See License.txt.

   ProjectSerializer.h

*******************************************************************/

#ifndef __AUDACITY_PROJECTSERIALIZER__
#define __AUDACITY_PROJECTSERIALIZER__

// Tenacity libraries
#include <lib-strings/Identifier.h>
#include <lib-xml/XMLTagHandler.h>

#include "MemoryStream.h" // member variables
#include <wx/mstream.h>

#include <unordered_set>
#include <unordered_map>

#include "SampleBlock.h"

///
/// ProjectSerializer
///

using NameMap = std::unordered_map<wxString, unsigned short>;
using IdMap = std::unordered_map<unsigned short, wxString>;

// This class's overrides do NOT throw TenacityException.
class TENACITY_DLL_API ProjectSerializer final : public XMLWriter
{
public:

   static TranslatableString FailureMessage( const FilePath &filePath );

   ProjectSerializer(size_t allocSize = 1024 * 1024);
   virtual ~ProjectSerializer();

   void StartTag(const wxString & name) override;
   void EndTag(const wxString & name) override;

   void WriteAttr(const wxString & name, const wxString &value) override;
   void WriteAttr(const wxString & name, const wxChar *value) override;

   void WriteAttr(const wxString & name, int value) override;
   void WriteAttr(const wxString & name, bool value) override;
   void WriteAttr(const wxString & name, long value) override;
   void WriteAttr(const wxString & name, long long value) override;
   void WriteAttr(const wxString & name, size_t value) override;
   void WriteAttr(const wxString & name, float value, int digits = -1) override;
   void WriteAttr(const wxString & name, double value, int digits = -1) override;

   void WriteData(const wxString & value) override;
   void Write(const wxString & data) override;

   // Non-override functions
   void WriteSubTree(const ProjectSerializer & value);

   const MemoryStream& GetDict() const;
   const MemoryStream& GetData() const;

   bool IsEmpty() const;
   bool DictChanged() const;

   // Returns empty string if decoding fails
   static wxString Decode(const wxMemoryBuffer &buffer);

private:
   void WriteName(const wxString & name);

private:
   MemoryStream mBuffer;
   bool mDictChanged;

   static NameMap mNames;
   static MemoryStream mDict;
};

#endif
