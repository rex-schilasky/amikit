/////////////////////////////////////////////////////////////////////////
//
// FLOWERPOT.CPP            Rex Schilasky
//
/////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"
#include "wx/cmdline.h"
#include "wx/dir.h"
#include "wx/filename.h"
#include "wx/fileconf.h"
#include "wx/utils.h"
#include "wx/arrstr.h"
#include "wx/tokenzr.h"

#include "wx/msw/registry.h"

#include "main.h"

#include "../gfx/icon_amikit.xpm"

#include "../gfx/background.xpm"
#include "../gfx/background_x.xpm"

#include "../gfx/about_1.xpm"
#include "../gfx/configure_1.xpm"
#include "../gfx/donate_1.xpm"
#include "../gfx/launch_1.xpm"
#include "../gfx/exit_1.xpm"

#include "../gfx/about_2.xpm"
#include "../gfx/configure_2.xpm"
#include "../gfx/donate_2.xpm"
#include "../gfx/launch_2.xpm"
#include "../gfx/exit_2.xpm"


// ============================================================================
// Defines
// ============================================================================

#define AppTitle        "FlowerPot"
#define VerString       "version 10.0"
#define MaxThemes         32
#define MaxButtons         5

#define Button_PosX        2
#define Button_PosY      104
#define Button_HSpace     10
#define Button_VSpace     15


// ============================================================================
// Constants
// ============================================================================
enum
{
  Menu_About  = wxID_ABOUT,
  Menu_Themes = 2000,
  Menu_Exit   = wxID_EXIT,
};


// ============================================================================
// Globals
// ============================================================================
wxFileConfig* pFConf       = nullptr;
wxLogStderr*  pLog         = nullptr;
wxString      base_dir     = "";
wxString      runflowerpot = "";
wxString      configwinuae = "";
wxString      winuae_dir   = "";
wxString      winuae_exe   = "";
wxString      winuae_ver   = "";
wxString      winuae_check = "no";
wxString      theme        = "theme\\default";
wxString      website      = "";
wxString      manual       = "";
wxString      rundonate    = "";
wxString      contact      = "";
wxString      afonlinefile = "";
wxString      winpath      = "";
wxArrayString forbsysnames;
wxBitmap      bgbitmap;
wxString      themes[MaxThemes];

// ============================================================================
// Help Functions
// ============================================================================

wxString GetShellOpen(wxString file)
{
  wxLogMessage(wxString("GetShellOpen: ") + "FileName = " +  file);
  wxString ext = "";
  int index = file.Find('.',1);
  if(index >= 0) ext = file.Mid(index);
  wxLogMessage(wxString("GetShellOpen: ") + "FileName = " +  file);

  wxString app    = "";
  wxString regStr = "";
  wxRegKey regKey;

  if(!ext.IsEmpty())
  {
    regStr = wxString("HKEY_CLASSES_ROOT\\") + ext;
    wxLogMessage(wxString("GetShellOpen: ") + "ExtRegKey = " +  regStr);
    regKey.SetName(regStr);
    if(regKey.Exists())
    {
      app = regKey.QueryDefaultValue();
      wxLogMessage(wxString("GetShellOpen: ") + "AppViaExt = " +  app);
    }
  }
  else
  {
    app = file;
    wxLogMessage(wxString("GetShellOpen: ") + "AppNoExt = " +  app);
  }
  if(app.IsEmpty()) return("");
  
  wxString command = "";
  regStr = wxString("HKEY_CLASSES_ROOT\\") + app + wxString("\\shell\\open\\command\\");
  regKey.SetName(regStr);
  wxLogMessage(wxString("GetShellOpen: ") + "AppRegKey = " +  regStr);
  if(regKey.Exists())
  {
    command = regKey.QueryDefaultValue();
    wxLogMessage(wxString("GetShellOpen: ") + "Command = " +  regStr);
  }
  return(command);
}

void ShowURL(wxString url)
{
  wxLaunchDefaultBrowser(url);
}
   
// ============================================================================
// AkApp
// ============================================================================

IMPLEMENT_APP(AkApp)

BEGIN_EVENT_TABLE(AkFrame, wxFrame)
END_EVENT_TABLE()

bool AkApp::OnInit(void)
{
  wxCmdLineEntryDesc cmdLineDesc[] =
  {
    { wxCMD_LINE_SWITCH, "s", "silent", "silent mode" },
    { wxCMD_LINE_NONE }
  };
  wxCmdLineParser parser(argc, argv);
  parser.SetDesc(cmdLineDesc);
  bool silent_mode = false;
  if(!parser.Parse())
  {
    silent_mode = parser.Found("silent");
  }

  // Bitmap Handler
  wxImage::AddHandler(new wxPNGHandler);

  // Base Dir
  base_dir = wxGetCwd();
  wchar_t fname[_MAX_PATH];
  if(GetModuleFileName(NULL, fname, sizeof(fname)))
  {
    wxFileName fn1(fname);
    base_dir = fn1.GetPath();
  }

  // Read configuration
  wxFileName fn2(base_dir + "/" + "flowerpot.ini");
  pFConf = new wxFileConfig("FlowerPot", AppTitle, fn2.GetFullPath());
  pFConf->Read("RunFlowerPot",  &runflowerpot);
  pFConf->Read("ConfigWinUAE",  &configwinuae);
  pFConf->Read("WinUAEDir",     &winuae_dir);
  pFConf->Read("WinUAEVer",     &winuae_ver);
  pFConf->Read("WinUAECheck",   &winuae_check);
  wxString tmp_theme;
  pFConf->Read("Theme",         &tmp_theme);
  if (!tmp_theme.empty() && wxFileExists(base_dir + "/" + tmp_theme + "/" + "background.png"))
  {
    theme = tmp_theme;
  }
  pFConf->Read("WebSite",       &website);
  pFConf->Read("Manual",        &manual);
  pFConf->Read("RunDonate",     &rundonate);
  pFConf->Read("Contact",       &contact);
  pFConf->Read("AFOnlineFile",  &afonlinefile);
  pFConf->Read("WinPath",       &winpath);
  
  // Create list of forbidden sys names
  wxString forbsysnames_str = "";
  pFConf->Read("ForbSysNames",  &forbsysnames_str);
  wxStringTokenizer tkz(forbsysnames_str, wxT(" \t\r\n"));
  while(tkz.HasMoreTokens())
  {
    wxString token = tkz.GetNextToken();
    token.Replace(":", "");
    forbsysnames.Add(token);
  }

  // Open log file
  FILE* logfp = fopen((base_dir + "\\" + "flowerpot.log").c_str(), "wt");
  if(logfp)
  {
    pLog = new wxLogStderr(logfp);
    wxLog::SetActiveTarget(pLog);
  }

  // AFOnline Files
  wxString afonlinereg = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Cloanto\\Amiga Forever";
  wxString afiles      = "";
  wxRegKey regKey1;
  regKey1.SetName(afonlinereg);
  if(!regKey1.Exists())
  {
    afonlinereg = "HKEY_CURRENT_USER\\SOFTWARE\\Cloanto\\Amiga Forever";
    regKey1.SetName(afonlinereg);
  }
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
        afiles = "";
        if(regKey1.HasValue("AmigaFiles"))
        {
          regKey1.QueryValue("AmigaFiles", afiles);
        }
      }
    }
  }

  // if not found in registry, check environment variable
  if(afiles.empty())
  {
    wxGetEnv("AMIGAFOREVERDATA", &afiles);
  }

  // complete amiga files path
  if(!afiles.empty())
  {
    if(afiles.Last() == '\\') afiles.RemoveLast();
    if(afiles.Last() == '/')  afiles.RemoveLast();
    wxString drive = afiles[0];
    drive.MakeUpper();
    wchar_t name[64] = {0};
    if(GetVolumeInformation((drive + ":\\").c_str(), name, sizeof(name), NULL, NULL, NULL, NULL, 0))
    {
      if(wcslen(name) > 0)
      {
        drive = name;
      }
      else
      {
        drive = "WinDH_" + drive;
      }
    }
    if(forbsysnames.Index(drive) != wxNOT_FOUND)
    {
      wxString msg  = "The name of your Windows harddisk might be in conflict with FlowerPot.\nPlease rename your harddisk partition before you run FlowerPot.";
      wxString name = "\n\nCurrent name is    \"" + drive + "\"";
      wxMessageDialog dlg(NULL, msg + name, "FlowerPot Error", wxOK);
      dlg.ShowModal();
      return(false);
    }
    afiles = afiles.substr(3);
    afiles = drive + ":" + afiles;
    afiles.Replace("\\", "/");
    afiles.Replace("//", "/");

    wxFileName fn(base_dir + "/" + afonlinefile);
    wxString full_fn = fn.GetFullPath();
    FILE* fp = fopen(full_fn.c_str(), "w");
    if(fp)
    {
      fwrite(afiles.c_str(), 1, afiles.Length(), fp);
      fclose(fp);
      fp = NULL;
    }
  }

  // WinPath
  wxFileName fn(base_dir + "/" + winpath);
  wxString full_fn = fn.GetFullPath();
  FILE* fp = fopen(full_fn.c_str(), "w");
  wxString drive = "";
  if(fp && ::wxGetEnv("SystemDrive", &drive))
  {
    wchar_t name[64] = {0};
    if(GetVolumeInformation((drive + "\\").c_str(), name, sizeof(name), NULL, NULL, NULL, NULL, 0))
    {
      if(wcslen(name) > 0)
      {
        drive = name;
        drive += ":";
      }
      else
      {
        drive = "WinDH_" + drive;
      }
    }
    fwrite(drive.c_str(), 1, drive.Length(), fp);
    fclose(fp);
    fp = NULL;
  }


  // WinUAE FlowerPot inside executable
  wxString winuae_exe_local = winuae_dir + "/winuae.exe";
  wxString winuae_exe_inst  = "";

  // WinUAE check for newer installed version
  bool check_winuae = true;
  if(winuae_check.Len() > 0)
  {
    winuae_check = winuae_check.Lower();
    winuae_check.Trim();
    winuae_check.Trim(true);
    if(winuae_check == "no") check_winuae = false;
  }

  if(check_winuae)
  {
    wxRegKey regKey2;
    regKey2.SetName("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WinUAE");
    if(regKey2.Exists())
    {
      wxString version = "";
      if(regKey2.HasValue("DisplayVersion") && regKey2.QueryValue("DisplayVersion", version))
      {
        int index = version.Find(' ');
        if(index > 0) version = version.Mid(index+1);
        index = version.Find(' ');
        if(index > 0) version = version.Left(index);
        index = version.Find('.', 1);
        if(index > 0) version = version.Left(index);
        double version_d = 0.0;
        version.ToDouble(&version_d);
        double winuae_ver_d = 0.0;
        winuae_ver.ToDouble(&winuae_ver_d);
        if(version_d > winuae_ver_d)
        {
          if(!(regKey2.HasValue("DisplayIcon") && regKey2.QueryValue("DisplayIcon", winuae_exe_inst)))
          {
            wxString command = GetShellOpen(".uae");
            int index = command.Find(".exe");
            if(index > 0)
            {
              winuae_exe_inst = command.Left(index+4);
            }
          }
        }
      }
    }
  }
  
  // select WinUAE version
  if(wxFileName::FileExists(winuae_exe_inst))
  {
    winuae_exe = winuae_exe_inst;
  }
  else
  {
    winuae_exe = winuae_exe_local;
  }

  if(!silent_mode)
  {
    // Create the main frame window
    AkFrame *frame = new AkFrame(NULL, wxID_ANY, _T(AppTitle), wxPoint(0,0), wxSize(460,516));

    // Make a panel
    frame->CenterOnScreen();
    frame->Show(true);

    frame->SetIcon(wxIcon(icon));

    // Return the main frame window
    SetTopWindow(frame);
    
    return true;
  }
  
  return false;
}


// ============================================================================
// AkBitmapButton
// ============================================================================

AkBitmapButton::AkBitmapButton(wxWindow *parent, wxWindowID id, const wxBitmap& bitmap1, const wxBitmap& bitmap2)
               : wxBitmapButton(parent, id, bitmap1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
  // info
  pBitmap[0] = new wxBitmap(bitmap1);
  pBitmap[1] = new wxBitmap(bitmap2);
}

AkBitmapButton::~AkBitmapButton()
{
  if(pBitmap[0]) delete pBitmap[0];
  if(pBitmap[1]) delete pBitmap[1];
}

bool AkBitmapButton::SetBitmap1(wxBitmap& bitmap)
{
  if(pBitmap[0]) delete pBitmap[0];
  pBitmap[0] = new wxBitmap(bitmap);
  SetBitmapLabel(*pBitmap[0]);
  return(true);
}

bool AkBitmapButton::SetBitmap2(wxBitmap& bitmap)
{
  if(pBitmap[1]) delete pBitmap[1];
  pBitmap[1] = new wxBitmap(bitmap);
  return(true);
}

bool AkBitmapButton::ProcessEvent(wxEvent& event)
{
  if(event.IsKindOf(CLASSINFO(wxMouseEvent)))
  {
    wxMouseEvent* pEvent = (wxMouseEvent*)&event;
    if(pEvent->Entering())
    {
      SetBitmapLabel(*pBitmap[1]);
    }
    if(pEvent->Leaving())
    {
      SetBitmapLabel(*pBitmap[0]);
    }
  }
  return(wxWindow::ProcessEvent(event));
}


// ============================================================================
// AkPanel
// ============================================================================

const int ID_BUTTON_1 = 101;
const int ID_BUTTON_2 = 102;
const int ID_BUTTON_3 = 103;
const int ID_BUTTON_4 = 104;
const int ID_BUTTON_5 = 105;

BEGIN_EVENT_TABLE(AkPanel, wxPanel)
EVT_PAINT       (                AkPanel::OnPaint)
EVT_LEFT_UP     (                AkPanel::OnLeftUp)
EVT_CONTEXT_MENU(                AkPanel::OnContextMenu)

EVT_BUTTON      (ID_BUTTON_1,    AkPanel::OnButtonAK)
EVT_BUTTON      (ID_BUTTON_2,    AkPanel::OnButtonDoc)
EVT_BUTTON      (ID_BUTTON_3,    AkPanel::OnButtonConfUAE)
EVT_BUTTON      (ID_BUTTON_4,    AkPanel::OnButtonDonate)
EVT_BUTTON      (ID_BUTTON_5,    AkPanel::OnButtonExit)

EVT_MENU        (Menu_About,     AkPanel::OnMenuAbout)
EVT_MENU_RANGE  (Menu_Themes, Menu_Themes+MaxThemes, AkPanel::OnMenuThemes)
EVT_MENU        (Menu_Exit,      AkPanel::OnMenuExit)
END_EVENT_TABLE()

AkPanel::AkPanel(wxFrame *frame, int x, int y, int w, int h )
       : wxPanel(frame, wxID_ANY, wxPoint(x, y), wxSize(w, h) )
{
  m_frame  = frame;

  m_sizer1 = new wxBoxSizer(wxVERTICAL);
  m_sizer2 = new wxBoxSizer(wxHORIZONTAL);
  m_sizer3 = new wxBoxSizer(wxHORIZONTAL);
  m_sizer4 = new wxBoxSizer(wxHORIZONTAL);
  m_sizer1->Add(1, Button_PosY);
  m_sizer2->Add(Button_PosX, 1);
  m_sizer3->Add(Button_PosX, 1);
  m_sizer4->Add(Button_PosX, 1);

  // Button (launch)
  wxBitmap bitmap11 = wxBitmap(launch_1);
  wxBitmap bitmap12 = wxBitmap(launch_2);
  m_Button[0] = new AkBitmapButton(this, ID_BUTTON_1, bitmap11, bitmap12);
  m_sizer2->Add(m_Button[0], 0, wxLEFT | wxRIGHT, Button_HSpace);
#ifdef _DEBUG
  m_Button[0]->SetToolTip(GetRunCommand());
#endif

  // Button (about)
  wxBitmap bitmap21 = wxBitmap(about_1);
  wxBitmap bitmap22 = wxBitmap(about_2);
  m_Button[1] = new AkBitmapButton(this, ID_BUTTON_2, bitmap21, bitmap22);

  // Button (configure)
  wxBitmap bitmap31 = wxBitmap(configure_1);
  wxBitmap bitmap32 = wxBitmap(configure_2);
  m_Button[2] = new AkBitmapButton(this, ID_BUTTON_3, bitmap31, bitmap32);
#ifdef _DEBUG
  m_Button[2]->SetToolTip(GetCfgCommand());
#endif

  // Button (donante)
  wxBitmap bitmap41 = wxBitmap(donate_1);
  wxBitmap bitmap42 = wxBitmap(donate_2);
  m_Button[3] = new AkBitmapButton(this, ID_BUTTON_4, bitmap41, bitmap42);

  // Button (exit)
  wxBitmap bitmap51 = wxBitmap(exit_1);
  wxBitmap bitmap52 = wxBitmap(exit_2);
  m_Button[4] = new AkBitmapButton(this, ID_BUTTON_5, bitmap51, bitmap52);

  m_sizer3->Add(m_Button[2], 0, wxLEFT | wxRIGHT, Button_HSpace);
  m_sizer3->Add(m_Button[3], 0, wxLEFT | wxRIGHT, Button_HSpace);
  m_sizer4->Add(m_Button[1], 0, wxLEFT | wxRIGHT, Button_HSpace);
  m_sizer4->Add(m_Button[4], 0, wxLEFT | wxRIGHT, Button_HSpace);

  // Sizer2 > Sizer1
  m_sizer1->Add(1, Button_VSpace);
  m_sizer1->Add(m_sizer2);
  m_sizer1->Add(1, Button_VSpace);
  m_sizer1->Add(m_sizer3);
  m_sizer1->Add(1, Button_VSpace);
  m_sizer1->Add(m_sizer4);

  // Read theme
  LoadTheme();

  SetSizer(m_sizer1);
}

AkPanel::~AkPanel()
{
}

void AkPanel::OnPaint(wxPaintEvent& WXUNUSED(event))
{
  wxPaintDC dc(this);
  if(bgbitmap.Ok())
  {
    dc.DrawBitmap(bgbitmap, 0, 0, false);
  }
  else
  {
    wxBitmap bitmap(background);
    dc.DrawBitmap(bitmap, 0, 0, false);
  }
}

void ShowTitleText(wxFrame* frame, wxString text, int sleep)
{
  wxString Title = wxString(AppTitle);
  size_t tlen = text.Len();
  size_t slen = 45 - tlen/2;
  wxString Space = "";
  while(slen--) Space = Space+ " ";
  frame->SetTitle(Title + Space + text);
  wxMilliSleep(sleep);
  for(size_t x = 0; x < tlen/2+1; x++)
  {
    Space = Space + " ";
    text = text.Mid(1);
    text = text.Mid(0, text.Len()-1);
    frame->SetTitle(Title + Space + text);
    wxMilliSleep(10);
  }
  frame->SetTitle(Title);
}


void AkPanel::OnLeftUp(wxMouseEvent& event)
{
  wxPoint pos = event.GetPosition();
  wxClientDC cdc(this);

#if 0
  // the kitty nose
  if((pos.x > 50) && (pos.x < 160) && (pos.y > 185) && (pos.y < 300))
  {
    ShowTitleText(m_frame, VerString, 2500);
    ShowTitleText(m_frame, "GUI coded by Rex Schilasky", 2500);
    ShowTitleText(m_frame, "Gfx painted by Kenneth E. Lester, Jr.", 2500);
  }
#endif
  
  // the flowerpot sign
  if((pos.x > 30) && (pos.x < 510) && (pos.y > 20) && (pos.y < 100))
  {
    ShowURL(website);
  }
}

void AkPanel::OnContextMenu(wxContextMenuEvent& event)
{
  wxPoint point = event.GetPosition();
  // If from keyboard
  if (point.x == -1 && point.y == -1) {
    wxSize size = GetSize();
    point.x = size.x / 2;
    point.y = size.y / 2;
  } else {
    point = ScreenToClient(point);
  }
  ShowContextMenu(point);
}

// Run FlowerPot
void AkPanel::OnButtonAK(wxCommandEvent& WXUNUSED(event))
{
  if(::wxExecute(GetRunCommand()) > 0)
  {
    // Close launcher if successful
    ::wxExit();
  }
}

// Configure WinUAE
void AkPanel::OnButtonConfUAE(wxCommandEvent& WXUNUSED(event))
{
  wxExecute(GetCfgCommand());
}

// Read the Manual
void AkPanel::OnButtonDoc(wxCommandEvent& WXUNUSED(event))
{
  wxFileName file_name(base_dir + "/" + manual);
  wxString file = file_name.GetFullPath();
  wxLogMessage(wxString("OnButtonDoc: ") + "FileName = " +  file);
  ShowURL(file);
}

// Make a Donation
void AkPanel::OnButtonDonate(wxCommandEvent& WXUNUSED(event))
{
#if 0
  wxFileName file_name(base_dir + "/" + donation);
  wxString file = file_name.GetFullPath();
  wxLogMessage(wxString("OnButtonDonate: ") + "FileName = " +  file);
  ShowURL(file);
#else
  wxExecute(GetDonateCommand());
#endif
}

// Exit
void AkPanel::OnButtonExit(wxCommandEvent& WXUNUSED(event))
{
  ::wxExit();
}

void AkPanel::OnMenuAbout(wxCommandEvent& event)
{
  (void)wxMessageBox(_T("-------------------------------------------    \n  Coding: Rex Schilasky\n\n  Artwork: Kenneth E. Lester, Jr.\n-------------------------------------------    "),
                     _T("flowerpot launcher ") + wxString("(") + VerString + ")",
                     wxICON_INFORMATION);
}

void AkPanel::OnMenuThemes(wxCommandEvent& event)
{
  int id = event.GetId() - Menu_Themes;
  if(id < MaxThemes)
  {
    wxFileName fpath(theme);
    wxString themedir = fpath.GetPath();
    theme = themedir + "\\" + themes[id];
    pFConf->Write("Theme", theme);
    pFConf->Flush();
    LoadTheme();
  }
}

void AkPanel::OnMenuExit(wxCommandEvent& event)
{
  ::wxExit();
}

bool AkPanel::LoadTheme()
{
  if(theme.Len() > 0)
  {
    // Read background image
    wxFileName fpath(base_dir + "\\" + theme + "\\" + "background.png");
    if(fpath.FileExists())
    {
      wxString fname = fpath.GetFullPath();
      bgbitmap.LoadFile(fname, wxBITMAP_TYPE_ANY);
    }

    // Read buttons
    LoadButton(0, "launch");
    LoadButton(1, "about");
    LoadButton(2, "configure");
    LoadButton(3, "donate");
    LoadButton(4, "exit");

    Refresh(true);
    
    return(true);
  }
  return(false);
}

bool AkPanel::LoadButton(int pos, wxString name)
{
  if((pos < 0) || (pos > MaxButtons-1)) return(false);

  bool ret_state = false;
  for(int bstate = 0; bstate < 2; bstate++)
  {
    // Button 1 (run_flowerpot)
    wxBitmap bitmap;
    wxFileName fpath(base_dir + "\\" + theme + "\\" + name + wxString::Format("_%d", bstate+1) + ".png");
    if(fpath.FileExists())
    {
      wxString fname = fpath.GetFullPath();
      if(bitmap.LoadFile(fname, wxBITMAP_TYPE_ANY))
      {
        if(bstate == 0) m_Button[pos]->SetBitmap1(bitmap);
        if(bstate == 1) m_Button[pos]->SetBitmap2(bitmap);
        ret_state = true;
      }
    }
  }
  return(ret_state);
}

wxMenu* AkPanel::CreateThemesMenu()
{
  wxFileName fpath(base_dir + "\\" + theme);
  wxString themedir = fpath.GetPath();
  
  wxDir dir1(themedir);
  wxDir dir2(themedir);
  if(!dir1.IsOpened()) return(nullptr);

  wxMenu* menu = new wxMenu;

  wxString subdir;
  int      ndir = 0;
  bool cont = dir1.GetFirst(&subdir);
  while(cont && ndir < MaxThemes)
  {
    if(dir2.HasSubDirs(subdir))
    {
      wxFileName bgfile(themedir + "\\" + subdir + "\\" + "Background.png");
      if(bgfile.FileExists())
      {
        if((themedir + "\\" + subdir) == fpath.GetFullPath())
        {
          menu->AppendRadioItem(Menu_Themes+ndir, subdir);
          menu->Check(Menu_Themes+ndir, TRUE);
        }
        else
        {
          menu->AppendRadioItem(Menu_Themes+ndir, subdir);
        }
        themes[ndir] = subdir;
        ndir++;
      }
    }
    cont = dir1.GetNext(&subdir);
  }
  return menu;
}

void AkPanel::ShowContextMenu(const wxPoint& pos)
{
  wxMenu menu;
  menu.Append(Menu_About,  _T("&About"));
  menu.Append(Menu_Themes, _T("&Theme"), CreateThemesMenu());
  menu.Append(Menu_Exit,   _T("&Exit"));
  PopupMenu(&menu, pos.x, pos.y);
}

wxString AkPanel::GetRunCommand()
{
  wxString command = winuae_exe;
  wxFileName file_name(base_dir + "/" + winuae_dir);
  wxString dir = file_name.GetFullPath();
  command = command + " -datapath " + "\"" + dir + "\" " + runflowerpot;
  return(command);
}

wxString AkPanel::GetCfgCommand()
{
  wxString command = winuae_exe;
  wxFileName file_name(base_dir + "/" + winuae_dir);
  wxString dir = file_name.GetFullPath();
  command = command + " -datapath " + "\"" + dir + "\" " + configwinuae;
  return(command);
}

wxString AkPanel::GetDonateCommand()
{
  wxString command = winuae_exe;
  wxFileName file_name(base_dir + "/" + winuae_dir);
  wxString dir = file_name.GetFullPath();
  command = command + " -datapath " + "\"" + dir + "\" " + rundonate;
  return(command);
}


// ============================================================================
// AkFrame
// ============================================================================

AkFrame::AkFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame(parent, id, title, pos, size, 
           wxFRAME_SHAPED
         | wxSIMPLE_BORDER
      // | wxFRAME_NO_TASKBAR
      // | wxSTAY_ON_TOP
         )
{
  m_panel = new AkPanel(this, 0, 0, size.x, size.y);
  SetSize(size.x-14, size.y-43);
}
