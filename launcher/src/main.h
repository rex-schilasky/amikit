/////////////////////////////////////////////////////////////////////////
//
// MAIN.H              Rex Schilasky
//
/////////////////////////////////////////////////////////////////////////

#pragma once

// ============================================================================
// AkApp
// ============================================================================

class AkApp: public wxApp
{
public:
  AkApp(void){};
  bool OnInit(void);
};


// ============================================================================
// AkBitmapButton
// ============================================================================

class AkBitmapButton: public wxBitmapButton
{
public:
  AkBitmapButton(wxWindow *parent, wxWindowID id, const wxBitmap& bitmap1, const wxBitmap& bitmap2);
  ~AkBitmapButton();

  bool SetBitmap1(wxBitmap& bitmap);
  bool SetBitmap2(wxBitmap& bitmap);

protected:
  // system
  bool ProcessEvent(wxEvent& event);

  // info
  wxBitmap* pBitmap[2];
};


// ============================================================================
// AkPanel
// ============================================================================

class AkPanel: public wxPanel
{
public:
  AkPanel(wxFrame *frame, int x, int y, int w, int h);
  virtual ~AkPanel();
  
protected:
  void OnPaint(wxPaintEvent& event);
  void OnLeftUp(wxMouseEvent& event);
  void OnContextMenu(wxContextMenuEvent& event);
  void OnButtonAK(wxCommandEvent& event);
  void OnButtonConfUAE(wxCommandEvent& event);
  void OnButtonDoc(wxCommandEvent& event);
  void OnButtonDonate(wxCommandEvent& event);
  void OnButtonExit(wxCommandEvent& event);

  void OnMenuAbout(wxCommandEvent& event);
  void OnMenuThemes(wxCommandEvent& event);
  void OnMenuExit(wxCommandEvent& event);

  bool LoadTheme();
  bool LoadButton(int pos, wxString name);
  wxMenu* CreateThemesMenu();
  void ShowContextMenu(const wxPoint& pos);
  wxString GetRunCommand();
  wxString GetCfgCommand();
  wxString GetDonateCommand();

  wxFrame          *m_frame;
  wxBoxSizer       *m_sizer1;
  wxBoxSizer       *m_sizer2;
  wxBoxSizer       *m_sizer3;
  wxBoxSizer       *m_sizer4;
  AkBitmapButton   *m_Button[5];
  
private:
  DECLARE_EVENT_TABLE()
};


// ============================================================================
// AkFrame
// ============================================================================

class AkFrame: public wxFrame
{
public:
  AkFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size);
  
private:
  AkPanel *m_panel;
  
  DECLARE_EVENT_TABLE()
};
