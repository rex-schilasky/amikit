/////////////////////////////////////////////////////////////////////////
//
// RHELP.CPP            Rex Schilasky
//
/////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"
#include "wx/dir.h"

#include "rhelp.h"

// use registry or environment variables to detect AF pathes
#define USE_REGISTRY 0


wxString ReadAFOnlineRomPath()
{
  wxString ret_path = "";

#if USE_REGISTRY
  wxString afonlinereg = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Cloanto\\Amiga Forever";
  wxRegKey regKey1;
  regKey1.SetName(afonlinereg);
  if(regKey1.Exists())
  {
    long         k = 0;
    wxString     sSubkey;
    wxStringList SubKeys;
    regKey1.GetFirstKey(sSubkey,k);
    if(sSubkey[0] == '2') SubKeys.Add(sSubkey);
    while(regKey1.GetNextKey(sSubkey,k))
    {
      if(sSubkey[0] == '2') SubKeys.Add(sSubkey);
    }
    SubKeys.Sort();
    if(SubKeys.GetCount() > 0)
    {
      wxString LastAFVersion = SubKeys.GetLast()->GetData();
      regKey1.SetName(afonlinereg + "\\" + LastAFVersion);
      if(regKey1.Exists())
      {
        wxString afiles = "";
        if(regKey1.HasValue("AmigaFiles") && regKey1.QueryValue("AmigaFiles", afiles))
        {
          if(afiles.Last() != '\\') afiles += '\\';
          // this is the valid path for AF2009 and newer
          wxString afiles1 = afiles + "Shared\\rom";
          // this is the valid path AF2008 and older
          wxString afiles2 = afiles + "System\\rom";
          if(wxDir::Exists(afiles1)) ret_path = afiles1;
          if(wxDir::Exists(afiles2)) ret_path = afiles2;
        }
      }
    }
  }
#else
  wxString afiles = ::getenv("AMIGAFOREVERDATA");
  if(!afiles.IsEmpty())
  {
    if(afiles.Last() != '\\') afiles += '\\';
    // this is the valid path for AF2009 and newer
    wxString afiles1 = afiles + "Shared\\rom";
    // this is the valid path AF2008 and older
    wxString afiles2 = afiles + "System\\rom";
    if(wxDir::Exists(afiles1)) ret_path = afiles1;
    if(wxDir::Exists(afiles2)) ret_path = afiles2;
  }
#endif

  return(ret_path);
}

wxString ReadAFOnlineDllPath()
{
  wxString ret_path = "";

#if USE_REGISTRY
  wxString afonlinereg = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Cloanto\\Amiga Forever";
  wxRegKey regKey1;
  regKey1.SetName(afonlinereg);
  if(regKey1.Exists())
  {
    long         k = 0;
    wxString     sSubkey;
    wxStringList SubKeys;
    regKey1.GetFirstKey(sSubkey,k);
    if(sSubkey[0] == '2') SubKeys.Add(sSubkey);
    while(regKey1.GetNextKey(sSubkey,k))
    {
      if(sSubkey[0] == '2') SubKeys.Add(sSubkey);
    }
    SubKeys.Sort();
    if(SubKeys.GetCount() > 0)
    {
      wxString LastAFVersion = SubKeys.GetLast()->GetData();
      regKey1.SetName(afonlinereg + "\\" + LastAFVersion);
      if(regKey1.Exists())
      {
        wxString afiles = "";
        if(regKey1.HasValue("Path") && regKey1.QueryValue("Path", afiles))
        {
          if(afiles.Last() != '\\') afiles += '\\';
          afiles = afiles + "Player";
          ret_path = afiles;
        }
      }
    }
  }
#else
  wxString afiles = ::getenv("AMIGAFOREVERROOT");
  if(!afiles.IsEmpty())
  {
    if(afiles.Last() != '\\') afiles += '\\';
    afiles = afiles + "Player";
    ret_path = afiles;
  }
#endif

  return(ret_path);
}

bool DoCopyFile(wxString file1, wxString file2, bool overwrite /* = false */)
{
  if(!wxFileName::FileExists(file1)) return(false);
  if(!overwrite)
  {
    if(wxFileName::FileExists(file2))  return(false);
  }
  wxCopyFile(file1, file2, true);
  return(true);
}
