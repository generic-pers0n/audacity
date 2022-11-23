/**********************************************************************

Audacity: A Digital Audio Editor

ProjectWindowBase.cpp

Paul Licameli split from ProjectWindow.cpp

**********************************************************************/

#include "ProjectWindowBase.h"

#include "Project.h"

ProjectWindowBase::ProjectWindowBase(wxWindow * parent, wxWindowID id,
                                 const wxPoint & pos,
                                 const wxSize & size, TenacityProject &project)
   : wxFrame(parent, id, _TS("Tenacity"), pos, size)
   , mProject{ project }
{
   project.SetFrame( this );

   // Ensure a unique name of this window for journalling purposes
   SetName(
      wxString::Format( L"AudacityProject %d", project.GetProjectNumber() ) );
};

ProjectWindowBase::~ProjectWindowBase()
{
}

namespace {

ProjectWindowBase *FindProjectWindow( wxWindow *pWindow )
{
   while ( pWindow && pWindow->GetParent() )
      pWindow = pWindow->GetParent();
   return dynamic_cast< ProjectWindowBase* >( pWindow );
}

}

TenacityProject *FindProjectFromWindow( wxWindow *pWindow )
{
   auto pProjectWindow = FindProjectWindow( pWindow );
   return pProjectWindow ? &pProjectWindow->GetProject() : nullptr;
}

const TenacityProject *FindProjectFromWindow( const wxWindow *pWindow )
{
   return FindProjectFromWindow( const_cast< wxWindow* >( pWindow ) );
}

