/////////////////////////////////////////////////////////////////////////
//
// RHELP.H              Rex Schilasky
//
/////////////////////////////////////////////////////////////////////////

#ifndef rhelp_h_included
#define rhelp_h_included

#include "wx/wx.h"
#include "wx/filename.h"
#include "wx/msw/registry.h"

wxString ReadAFOnlineRomPath();
wxString ReadAFOnlineDllPath();
bool DoCopyFile(wxString file1, wxString file2, bool overwrite = false);

#endif /* rhelp_h_included */
