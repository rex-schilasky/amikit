/////////////////////////////////////////////////////////////////////////
//
// RFIND.CPP            Rex Schilasky
//
/////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"
#include "wx/dir.h"
#include "wx/dirdlg.h"
#include "wx/filename.h"
#include "wx/fileconf.h"
#include "wx/utils.h"
#include "wx/msw/registry.h"

#include "../gfx/Icon.xpm"

#include "rhelp.h"
#include "rfind.h"

extern "C"
{
#include "rmemory.h"
}


// ============================================================================
// Defines
// ============================================================================

#define AppTitle        "AmiKit RomFind 1.6.4"
#define KickStart_Ver   3
#define KickStart_Rev   1


// ============================================================================
// Globals
// ============================================================================
wxLogStderr*  pLog         = NULL;
wxString      base_dir     = "";
wxString      start_dir    = "";
wxString      afdll_dir    = "";
wxString      kick_dir     = "";
wxString      kick_cloanto = "kick33180;kick34005;kick37175;kick39106;kick40068";

wxArrayString kick_list;
wxArrayString cloanto_list;


// ============================================================================
// RFApp
// ============================================================================

IMPLEMENT_APP(RFApp)

BEGIN_EVENT_TABLE(RFFrame, wxFrame)
END_EVENT_TABLE()

bool RFApp::OnInit(void)
{
  // Base Dir
  base_dir = wxGetCwd();
  char fname[_MAX_PATH];
  if(GetModuleFileNameA(NULL, fname, sizeof(fname)))
  {
    wxFileName fn1(fname);
    base_dir = fn1.GetPath();
  }

  // Read configuration
  wxFileName fn(base_dir + "/" + "romfind.ini");
  wxFileConfig fconf("RomFind", AppTitle, fn.GetFullPath());
  fconf.Read("WHDLoadKickPath", &kick_dir);

  FILE* logfp = fopen((base_dir + "\\" + "romfind.log").c_str(), "wt");
  if(logfp)
  {
    pLog = new wxLogStderr(logfp);
    wxLog::SetActiveTarget(pLog);
  }

  // Read AFOnLinePathes
  start_dir = ReadAFOnlineRomPath();
#ifdef _DEBUG
  //start_dir = "D:\\Sources\\AmiKit\\Roms";
#endif
  afdll_dir = ReadAFOnlineDllPath();

  // Create KickPath
  wxFileName kfn(kick_dir);
  kfn.MakeAbsolute(base_dir);
  kick_dir = kfn.GetFullPath();
  wxDir kdir(kick_dir);
  if(wxDir::Exists(kick_dir))
  {
    kdir.GetAllFiles(kick_dir, &kick_list, "kick*.rtb", wxDIR_FILES);
  }

  // Create Cloanto KickList
  if(!kick_cloanto.IsEmpty())
  {
    while(!kick_cloanto.IsEmpty())
    {
      int index = kick_cloanto.Find(';');
      if(index != wxNOT_FOUND)
      {
        wxString kick = kick_cloanto.Left(index);
        kick_cloanto = kick_cloanto.Mid(index+1);
        kick.Trim();
        kick.Trim(true);
        cloanto_list.Add(kick);
      }
      else
      {
        wxString kick = kick_cloanto;
        kick = kick.Trim();
        kick = kick.Trim(true);
        cloanto_list.Add(kick);
        break;
      }
    }
  }

  // Create the main frame window
  RFFrame* frame = new RFFrame(NULL, wxID_ANY, _T(AppTitle), wxPoint(0,0), wxSize(640,300));

  // Make a panel
  frame->CenterOnScreen();
  frame->Show(true);

  frame->SetIcon(wxIcon(Icon));

  // Return the main frame window
  SetTopWindow(frame);

  return true;
}


// ============================================================================
// RFThread
// ============================================================================
RFThread::RFThread(wxString startdir, wxString dlldir, int mode, wxPanel* pPanel /* = NULL */)
         : wxThread()
{
  m_startdir = startdir;
  m_dlldir   = dlldir;
  m_mode     = mode;
  m_panel    = pPanel;
  m_sleep    = false;
  m_stop     = false;
}

void RFThread::OnExit()
{
}

void *RFThread::Entry()
{
  if(m_startdir == "")
  {
    unsigned char drive[] = "c";
    while(drive[0] <= 'z')
    {
      wxString dn = drive;
      dn += ":\\";
      RomSearch(dn);
      drive[0]++;
    }
  }
  else
  {
    RomSearch(m_startdir);
  }

  if(m_panel) ((RFPanel*)m_panel)->NothingFound();

  while(1)
  {
    if(!TestDestroy())
    {
      wxMilliSleep(10);
    }
    else
    {
      break;
    }
  }

  return NULL;
}

bool RFThread::RomSearch(const wxString& startdir)
{
  if(TestDestroy())            return(false);
  if(!wxDir::Exists(startdir)) return(false);

  wxDir dir1(startdir);
  if(!dir1.IsOpened()) return(false);

  if(m_panel) ((RFPanel*)m_panel)->CurrentDir(startdir);

  wxArrayString files;
  int roms_found = dir1.GetAllFiles(startdir, &files, "*.rom", wxDIR_FILES);
  roms_found    += dir1.GetAllFiles(startdir, &files, "*.143", wxDIR_FILES);
  files.Sort();
  if(roms_found > 0)
  {
    // WHDLoad KickRom ?
    if(m_mode & RFMODE_WHDLOAD)
    {
      if(!kick_list.IsEmpty())
      {
        for(unsigned int i = 0; i < files.GetCount() && !m_stop; i++)
        {
          if(TestDestroy()) return(false);
          wxString kickfile = files[i];
          FILE* fp = fopen(kickfile.c_str(), "rb");
          if(fp)
          {
            struct romdata* pRomData = scan_single_rom(fp, startdir.c_str(), m_dlldir.c_str());
            if(pRomData)
            {
              if(pRomData->type == ROMTYPE_KICK)
              {
                if(pRomData->cloanto)
                {
                  wxString model = pRomData->model;
                  if(model.Last() == '+') model.RemoveLast();
                  wxString kick_name = "kick" + wxString::Format("%02d", pRomData->subver) + wxString::Format("%03d", pRomData->subrev);
                  if(cloanto_list.Index(kick_name, false) != wxNOT_FOUND)
                  {
                    wxString model = pRomData->model;
                    if(model.Last() == '+') model.RemoveLast();
                    wxString rtb_name = kick_dir + "\\" + "kick" + wxString::Format("%02d", pRomData->subver) + wxString::Format("%03d", pRomData->subrev) + "." + model;
                    wxString rom_name = rtb_name;
                    rtb_name += ".rtb";
                    bool copied = false;
                    if(kick_list.Index(rtb_name, false) != wxNOT_FOUND)
                    {
                      copied |= DoCopyFile(kickfile, rom_name);
                      wxLogMessage(wxString("COPY ROM WHDLoad (Cloanto): ") + kickfile + " -> " +  rom_name);
                    }
                    else
                    {
                      wxLogMessage(wxString("DID NOT COPY ROM WHDLoad (Cloanto): ") + kickfile + " -> " +  rom_name + " (RTB File not found: " + rtb_name);
                    }
                    if(copied)
                    {
                      wxFileName src_fn(startdir);
                      wxString src_path = src_fn.GetFullPath() + "\\" + "rom.key";
                      wxFileName tgt_fn(kick_dir);
                      wxString tgt_path = tgt_fn.GetFullPath() + "\\" + "rom.key";

                      DoCopyFile(src_path, tgt_path);
                      wxLogMessage(wxString("COPY ROM.KEY WHDLoad (Cloanto): ") + src_path + " -> " +  tgt_path);
                    }
                  }
                }
                else
                {
                  wxString model = pRomData->model;
                  if(model.Last() == '+') model.RemoveLast();
                  wxString rtb_name      = kick_dir + "\\" + "kick" + wxString::Format("%02d", pRomData->subver) + wxString::Format("%03d", pRomData->subrev) + "." + model;
                  wxString rtb_name_beta = rtb_name + ".BETA";
                  wxString rom_name      = rtb_name;
                  wxString rom_name_beta = rtb_name_beta;
                  rtb_name      += ".rtb";
                  rtb_name_beta += ".rtb";
                  if(kick_list.Index(rtb_name, false) != wxNOT_FOUND)
                  {
                    DoCopyFile(kickfile, rom_name);
                    wxLogMessage(wxString("COPY ROM WHDLoad (Regular): ") + kickfile + " -> " +  rom_name);
                  }
                  if(kick_list.Index(rtb_name_beta, false) != wxNOT_FOUND)
                  {
                    DoCopyFile(kickfile, rom_name_beta);
                    wxLogMessage(wxString("COPY ROM WHDLoad (Regular): ") + kickfile + " -> " +  rom_name_beta);
                  }
                }
              }
            }
            fclose(fp);
          }
        }
      }
    }

    // AmiKit KickRom ?
    if(m_mode & RFMODE_AMIKIT)
    {
      if(m_panel)
      {
        for(unsigned int i = 0; i < files.GetCount() && !m_stop; i++)
        {
          if(TestDestroy()) return(false);
          wxString kickfile = files[i];
          FILE* fp = fopen(kickfile.c_str(), "rb");
          if(fp)
          {
            struct romdata* pRomData = scan_single_rom(fp, startdir.c_str(), m_dlldir.c_str());
            if(pRomData)
            {
              if((pRomData->ver >= KickStart_Ver) && (pRomData->rev >= KickStart_Rev) && (pRomData->type == ROMTYPE_KICK))
              {
                wxString model = pRomData->model;
                if((model == "A1200") || (model == "A4000"))
                {
                  if(((RFPanel*)m_panel)->FileFound(kickfile, pRomData))
                  {
                    m_sleep = true;
                    while(m_sleep && !m_stop)
                    {
                      if(TestDestroy()) return(false);
                      wxThread::Sleep(100);
                    }
                  }
                }
              }
            }
            fclose(fp);
          }
        }
      }
    }
  }

  wxDir dir2(startdir);
  if(!dir2.IsOpened()) return(false);
  wxString subdir;
  bool cont = dir2.GetFirst(&subdir, "", wxDIR_DIRS);
  while(cont)
  {
    if(TestDestroy()) return(false);
    wxFileName fn(startdir, subdir);
    if(RomSearch(fn.GetFullPath()))
    {
      return(true);
    }
    cont = dir2.GetNext(&subdir);
  }

  return(false);
}


// ============================================================================
// RFPanel
// ============================================================================

const int ID_BUTTON_DirSelB = 101;
const int ID_BUTTON_DirSelE = 102;

const int ID_BUTTON_Start   = 110;
const int ID_BUTTON_Use     = 111;
const int ID_BUTTON_Cancel  = 112;

const int RFT_INIT          =  -1;
const int RFT_STOP          =   0;
const int RFT_SLEEP         =   1;
const int RFT_RUN           =   2;


BEGIN_EVENT_TABLE(RFPanel, wxPanel)
EVT_PAINT  (                   RFPanel::OnPaint)
EVT_BUTTON (ID_BUTTON_DirSelB, RFPanel::OnButtonDirSel)
EVT_BUTTON (ID_BUTTON_Start,   RFPanel::OnButtonStart)
EVT_BUTTON (ID_BUTTON_Use,     RFPanel::OnButtonUse)
EVT_BUTTON (ID_BUTTON_Cancel,  RFPanel::OnButtonCancel)
END_EVENT_TABLE()

RFPanel::RFPanel(wxFrame *frame, int x, int y, int w, int h )
       : wxPanel(frame, wxID_ANY, wxPoint(x, y), wxSize(w, h) )
{
  m_frame      = frame;
  m_wh_thread  = NULL;
  m_ak_thread  = NULL;
  m_romdata    = NULL;
  m_state      = RFT_INIT;

  m_sizer10   = new wxBoxSizer (wxHORIZONTAL);
  
  m_sizer111  = new wxBoxSizer (wxVERTICAL);
  m_sizer1111 = new wxBoxSizer (wxHORIZONTAL);
  m_sizer1112 = new wxBoxSizer (wxHORIZONTAL);
  m_sizer1113 = new wxBoxSizer (wxHORIZONTAL);
  m_sizer1114 = new wxBoxSizer (wxHORIZONTAL);
  m_sizer11   = new wxBoxSizer (wxHORIZONTAL);

  m_sizer12   = new wxBoxSizer (wxHORIZONTAL);
  m_sizer1    = new wxBoxSizer (wxVERTICAL);
  
  m_sizer10->Add(8,0);
  m_sizer11->Add(8,0);
  m_sizer12->Add(6,0);
  m_sizer1->Add(0,8);

  wxStaticText* SEntry0 = new wxStaticText(this, -1, "This program searches your harddisk for a suitable ROM file (AmiKit needs ROM v3.1 file).\nPlease press the \"Search\" button. If correct file is found, press the \"Install\" button.", wxDefaultPosition, wxSize(620,40));
  m_sizer10->Add(SEntry0, 0, wxALL, 4);

  m_sizer1111->Add(0,4);
  wxStaticText* SEntry1 = new wxStaticText(this, -1, "Start Directory: ", wxDefaultPosition, wxSize(124,24));
  m_sizer1111->Add(SEntry1, 0, wxALL, 4);
  m_sizer1111->Add(4,0);
  m_sizer111->Add(m_sizer1111);

  wxStaticText* SEntry2 = new wxStaticText(this, -1, "Current Directory: ", wxDefaultPosition, wxSize(124,24));
  m_sizer1112->Add(SEntry2, 0, wxALL, 4);
  m_sizer1112->Add(4,0);
  m_sizer111->Add(m_sizer1112);

  wxStaticText* SEntry3 = new wxStaticText(this, -1, "Rom Filename: ", wxDefaultPosition, wxSize(124,24));
  m_sizer1113->Add(SEntry3, 0, wxALL, 4);
  m_sizer1113->Add(4,0);
  m_sizer111->Add(m_sizer1113);

  wxStaticText* SEntry4 = new wxStaticText(this, -1, "Rom Description: ", wxDefaultPosition, wxSize(124,24));
  m_sizer1114->Add(SEntry4, 0, wxALL, 4);
  m_sizer1114->Add(4,0);
  m_sizer111->Add(m_sizer1114);

  m_TEntry1 = new wxTextCtrl(this, ID_BUTTON_DirSelE, start_dir, wxDefaultPosition, wxSize(400,24));
  m_sizer1111->Add(m_TEntry1, 0, wxALL, 4);
  if(start_dir == "")
  {
    m_TEntry1->Disable();
  }

  m_TEntry2 = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(400,24), wxTE_READONLY);
  m_sizer1112->Add(m_TEntry2, 0, wxALL, 4);

  m_TEntry3 = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(400,24), wxTE_READONLY);
  m_sizer1113->Add(m_TEntry3, 0, wxALL, 4);

  m_TEntry4 = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(400,24), wxTE_READONLY | wxTE_CENTRE);
  m_sizer1114->Add(m_TEntry4, 0, wxALL, 4);

  m_ButtonD = new wxButton(this, ID_BUTTON_DirSelB, "...", wxDefaultPosition, wxSize(60,24));
  m_sizer1111->Add(6,0);
  m_sizer1111->Add(m_ButtonD, 0, wxALL, 4);

  m_sizer12->Add(136,0);
  m_ButtonS = new wxButton(this, ID_BUTTON_Start, "Search", wxDefaultPosition, wxSize(124,28));
  m_sizer12->Add(m_ButtonS, 0, wxALL, 4);
  m_ButtonS->SetFocus();
  m_ButtonS->SetDefault();
  //SetDefaultItem(m_ButtonS);

  m_ButtonU = new wxButton(this, ID_BUTTON_Use, "Install", wxDefaultPosition, wxSize(124,28));
  m_ButtonU->Disable();
  m_sizer12->Add(m_ButtonU, 0, wxALL, 4);

  m_sizer12->Add(40,0);
  m_ButtonC = new wxButton(this, ID_BUTTON_Cancel, "Cancel", wxDefaultPosition, wxSize(100,28));
  m_sizer12->Add(m_ButtonC, 0, wxALL, 4);

  // Sizers
  m_sizer11->Add(m_sizer111);

  m_sizer1->Add(m_sizer10);
  m_sizer1->Add(0,16);
  m_sizer1->Add(m_sizer11);
  m_sizer1->Add(0,16);
  m_sizer1->Add(m_sizer12);

  SetSizer(m_sizer1);
}

RFPanel::~RFPanel()
{
  if(m_wh_thread) m_wh_thread->Stop();
  m_wh_thread = NULL;
  if(m_ak_thread) m_ak_thread->Stop();
  m_ak_thread = NULL;
}

void RFPanel::CurrentDir(wxString DirName)
{
  m_TEntry2->SetValue(DirName);
}

bool RFPanel::FileFound(wxString FileName, struct romdata* pRomData)
{
  m_romdata = pRomData;
  wxString RomName = pRomData->name;
  m_TEntry3->SetValue(FileName);
  m_TEntry4->SetValue(RomName);

  m_ButtonS->SetLabel("Keep Searching");
  m_state = RFT_SLEEP;
  m_ButtonU->Enable();

  return(true);
}

void RFPanel::NothingFound()
{
  m_ak_thread = NULL;
  m_state = RFT_INIT;
  m_TEntry2->SetValue("");
  m_TEntry3->SetValue("");
  m_TEntry4->SetValue("No ROM files found.");
  m_ButtonS->SetLabel("Start");
  m_ButtonU->Disable();
}

void RFPanel::OnPaint(wxPaintEvent& WXUNUSED(event))
{
  wxPaintDC dc(this);
}

void RFPanel::OnButtonDirSel(wxCommandEvent& WXUNUSED(event))
{
  wxDirDialog dir_dlg(this, "Choose a start directory for RomFind", m_TEntry1->GetValue(), 0);
  int ret = dir_dlg.ShowModal();
  if(ret == wxID_OK)
  {
    m_TEntry1->Disable();
    m_TEntry1->SetValue("");
    m_TEntry1->Enable();
    m_TEntry1->SetValue(dir_dlg.GetPath());
    if(m_ak_thread)
    {
      m_wh_thread->Stop();
      m_wh_thread = NULL;
      m_ak_thread->Stop();
      m_ak_thread = NULL;
      m_state = RFT_INIT;
    }
    m_TEntry3->SetValue("");
    m_TEntry4->SetValue("");
    m_ButtonU->Disable();
  }
}

void RFPanel::OnButtonStart(wxCommandEvent& WXUNUSED(event))
{
  m_TEntry3->SetValue("");
  m_TEntry4->SetValue("");
  m_ButtonU->Disable();

  switch(m_state)
  {
  case RFT_INIT:
    {
      // fire background thread 1 with no interaction
      // to find and copy all WHDLoad roms
      // then continue with state 'RFT_STOP'
      // and start next thread with GUI interaction
      wxString dir = m_TEntry1->GetValue();
      if(m_wh_thread)
      {
        m_wh_thread->Stop();
        m_wh_thread = NULL;
      }
      m_wh_thread = new RFThread(dir, afdll_dir, RFMODE_WHDLOAD);
      m_wh_thread->Create();
      m_wh_thread->Run();
      m_state = RFT_STOP;
    }
  case RFT_STOP:
    {
      wxString dir = m_TEntry1->GetValue();
      if(m_ak_thread)
      {
        m_ak_thread->Stop();
        m_ak_thread = NULL;
      }
      m_ak_thread = new RFThread(dir, afdll_dir, RFMODE_AMIKIT, this);
      m_ak_thread->Create();
      m_ak_thread->Run();
      m_ButtonS->SetLabel("Stop");
      m_state = RFT_RUN;
    }
    break;
  case RFT_SLEEP:
    {
      if(m_ak_thread)
      {
        m_ak_thread->Continue();
        m_ButtonS->SetLabel("Stop");
        m_state = RFT_RUN;
      }
    }
    break;
  case RFT_RUN:
    {
      if(m_ak_thread)
      {
        m_ak_thread->Stop();
        m_ak_thread = NULL;
        m_ButtonS->SetLabel("Search");
        m_TEntry3->SetValue("");
        m_TEntry4->SetValue("");
        m_ButtonU->Disable();
        m_state = RFT_STOP;
      }
    }
    break;
  }
}

void RFPanel::OnButtonUse(wxCommandEvent& WXUNUSED(event))
{
  wxString   src_path = m_TEntry3->GetValue();
  wxFileName src_fn(src_path);

  wxString tgt_name = "kick.rom";

  wxFileName tgt_fn(base_dir, tgt_name);
  wxString   tgt_path = tgt_fn.GetFullPath();

  if(src_fn.FileExists())
  {
    if(src_path.MakeLower() == tgt_path.MakeLower())
    {
      m_frame->Close();
      return;
    }
    
    bool write_it = true;
    if(tgt_fn.FileExists())
    {
      wxMessageDialog dlg(this, wxString("Kick ROM File : ") + "\n\n\"" + tgt_path + "\"\n\nalready exists.\n\nShould I overwrite existing file ?     ", "ROM File exists !", wxOK | wxCANCEL);
      int ret = dlg.ShowModal();
      if(ret == wxID_CANCEL)
      {
        write_it = false;
      }
    }
    if(write_it)
    {
      bool cloanto = false;
      if(m_romdata)
      {
        if(m_romdata->cloanto)
        {
          cloanto = true;
          wxString src_path = src_fn.GetPath() + "\\" + "rom.key";
          wxString tgt_path = tgt_fn.GetPath() + "\\" + "rom.key";
          DoCopyFile(src_path, tgt_path, true);
          wxLogMessage(wxString("COPY ROM.KEY WinUAE (Cloanto): ") + src_path + " -> " +  tgt_path);
        }
      }
      DoCopyFile(src_path, tgt_path, true);
      if(cloanto)
      {
        wxLogMessage(wxString("COPY ROM WinUAE (Cloanto): ") + src_path + " -> " +  tgt_path);
      }
      else
      {
        wxLogMessage(wxString("COPY ROM WinUAE (Regular): ") + src_path + " -> " +  tgt_path);
      }
      m_frame->Close();
      return;
    }
  }
}

void RFPanel::OnButtonCancel(wxCommandEvent& WXUNUSED(event))
{
  m_frame->Close();
}


// ============================================================================
// RFFrame
// ============================================================================

RFFrame::RFFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame(parent, id, title, pos, size, wxSIMPLE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN)
{
  m_panel = new RFPanel(this, 0, 0, 100, 100);
}

RFFrame::~RFFrame()
{
  delete m_panel;
}
