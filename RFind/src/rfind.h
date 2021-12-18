/////////////////////////////////////////////////////////////////////////
//
// RFIND.H              Rex Schilasky
//
/////////////////////////////////////////////////////////////////////////

#ifndef rfind_h_included
#define rfind_h_included


// ============================================================================
// RFApp
// ============================================================================

class RFApp: public wxApp
{
public:
  RFApp(void){};
  bool OnInit(void);
};


// ============================================================================
// RFThread
// ============================================================================
#define RFMODE_WHDLOAD  1
#define RFMODE_AMIKIT   2
#define RFMODE_ALL      RFMODE_WHDLOAD | RFMODE_AMIKIT

class RFThread : public wxThread
{
public:
  RFThread(wxString startdir, wxString dlldir, int mode, wxPanel* pPanel = NULL);
  virtual void *Entry();
  virtual void OnExit();

  void Continue() { m_sleep = false; };
  void Stop() { m_sleep = false; m_stop = true; };

protected:
  bool RomSearch(wxString& startdir);

  wxString m_startdir;
  wxString m_dlldir;
  int      m_mode;
  wxPanel *m_panel;
  bool     m_sleep;
  bool     m_stop;
};


// ============================================================================
// RFPanel
// ============================================================================

class RFPanel: public wxPanel
{
public:
  RFPanel(wxFrame *frame, int x, int y, int w, int h);
  virtual ~RFPanel();

  void CurrentDir(wxString DirName);
  bool FileFound(wxString FileName, struct romdata* pRomData);
  void NothingFound();

protected:
  void OnPaint(wxPaintEvent& event);
  void OnButtonDirSel(wxCommandEvent& event);
  void OnButtonStart(wxCommandEvent& event);
  void OnButtonUse(wxCommandEvent& event);
  void OnButtonCancel(wxCommandEvent& event);
  
  wxFrame       *m_frame;
  wxBoxSizer    *m_sizer111;
  wxBoxSizer    *m_sizer1111;
  wxBoxSizer    *m_sizer1112;
  wxBoxSizer    *m_sizer1113;
  wxBoxSizer    *m_sizer1114;
  wxBoxSizer    *m_sizer112;
  wxBoxSizer    *m_sizer10;
  wxBoxSizer    *m_sizer11;
  wxBoxSizer    *m_sizer12;
  wxBoxSizer    *m_sizer1;

  wxTextCtrl    *m_TEntry1;
  wxTextCtrl    *m_TEntry2;
  wxTextCtrl    *m_TEntry3;
  wxTextCtrl    *m_TEntry4;
  wxButton      *m_ButtonD;
  wxButton      *m_ButtonS;
  wxButton      *m_ButtonU;
  wxButton      *m_ButtonC;

  RFThread      *m_wh_thread;
  RFThread      *m_ak_thread;
  romdata       *m_romdata;
  int            m_state;
  
private:
  DECLARE_EVENT_TABLE()
};


// ============================================================================
// RFFrame
// ============================================================================

class RFFrame: public wxFrame
{
public:
  RFFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size);
  virtual ~RFFrame();

private:
  RFPanel *m_panel;

  DECLARE_EVENT_TABLE()
};

#endif /* rfind_h_included */
