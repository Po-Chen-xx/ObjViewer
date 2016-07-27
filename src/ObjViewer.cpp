// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#define wxUSE_GUI 1

#include <string>
#include <windows.h>
#include "opencv2/opencv.hpp"
#include <wx/tglbtn.h>
#include <wx/filepicker.h>
#include <wx/wfstream.h>
#include "ObjViewer.h"
#include "OVCanvas.h"
#include "OVUtil.h"

namespace ov
{

// MyFrame constructor
ObjViewer::ObjViewer(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(660, 566))
{
    _renderMode = RENDER_SOLID;
    _lightingOn = true;

    // Data path
#ifdef RESEARCH_HANDTRACKING
    _dataFolder = "../../../data/PenPoseTracker/";
#else
    _dataFolder = "./";
#endif
    _objModelFile = _dataFolder + "model/cube/cube.obj";
    _imageFile = _dataFolder + "image/black.png";

    // Make the "File" menu
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(ID_MENU_OPEN_MODEL, wxT("Open &Model"), "Open .obj model file");
    fileMenu->Append(ID_MENU_OPEN_BACKGROUND_IMAGE, wxT("Open &Background Image"), "Open background image file");
    fileMenu->Append(ID_MENU_SAVE_IMAGE, wxT("S&ave Image"), "Save current frame to image file");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_MENU_EXIT, wxT("E&xit\tEsc"), "Quit this program");
    // Make the "Help" menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(ID_MENU_HELP, wxT("&About\tF1"), "Show about dialog");

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(helpMenu, wxT("&Help"));
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("OBJ Viewer");

    SetIcon(wxICON(IDI_APP));
    wxColour backgroundColor = wxColour(240, 240, 240);
    SetBackgroundColour(backgroundColor);
    _mainSizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(_mainSizer);

    // ==================================== Canvas part ====================================
    _oglCanvas = new OVCanvas(this, ID_CANVAS, wxDefaultPosition, GetClientSize(), wxSUNKEN_BORDER);
    _mainSizer->Add(_oglCanvas, 1, wxEXPAND);
    _oglCanvas->setRenderMode(_renderMode);
    _oglCanvas->setBackgroundImamge(_imageFile);
    _oglCanvas->setForegroundObject(_objModelFile);

    // ==================================== Controller part ====================================
    _controllerSizer = new wxBoxSizer(wxVERTICAL);
    _mainSizer->Add(_controllerSizer, 0, wxEXPAND);
    const wxString render_mode_radio_string[] =
    {
        wxT("Solid"),
        wxT("Wireframe"),
    };
    _renderModeRadioBox = new wxRadioBox(this,
                                         ID_RENDER_MODE_RADIO,
                                         wxT("&Render mode"),
                                         wxDefaultPosition,
                                         wxDefaultSize,
                                         WXSIZEOF(render_mode_radio_string),
                                         render_mode_radio_string,
                                         1,
                                         wxRA_SPECIFY_COLS);
    _controllerSizer->Add(_renderModeRadioBox, 0, wxEXPAND | wxALL, 5);
    _lightingCheckBox = CreateCheckBoxAndAddToSizer(this,
                                                    _controllerSizer,
                                                    wxT("Lighting"),
                                                    ID_LIGHTING);
    _lightingCheckBox->SetValue(true);
    _resetButton = new wxButton(this, ID_RESET, "Reset");
    _controllerSizer->Add(_resetButton, 0, wxEXPAND | wxALL, 5);

    reLayout();
    Show(true);

    Connect(ID_MENU_OPEN_MODEL, wxEVT_MENU, wxCommandEventHandler(ObjViewer::onMenuFileOpenObjModel));
    Connect(ID_MENU_OPEN_BACKGROUND_IMAGE, wxEVT_MENU, wxCommandEventHandler(ObjViewer::onMenuFileOpenBackgroundImage));
    Connect(ID_MENU_SAVE_IMAGE, wxEVT_MENU, wxCommandEventHandler(ObjViewer::onMenuFileSaveImage));
    Connect(ID_MENU_EXIT, wxEVT_MENU, wxCommandEventHandler(ObjViewer::onMenuFileExit));
    Connect(ID_MENU_HELP, wxEVT_MENU, wxCommandEventHandler(ObjViewer::onMenuHelpAbout));
    Connect(ID_RENDER_MODE_RADIO, wxEVT_RADIOBOX, wxCommandEventHandler(ObjViewer::onRenderModeRadio));
    Connect(ID_LIGHTING, wxEVT_CHECKBOX, wxCommandEventHandler(ObjViewer::onLightingCheck));
    Connect(ID_RESET, wxEVT_BUTTON, wxCommandEventHandler(ObjViewer::onReset));
}

ObjViewer::~ObjViewer()
{
    if (_oglCanvas) delete _oglCanvas;
}

void
ObjViewer::onMenuFileOpenObjModel(wxCommandEvent& evt)
{
    std::string objModelFile = wxFileSelector(wxT("Choose OBJ Model"), _dataFolder + "model", wxT(""), wxT(""),
        wxT("Wavefront Files (*.obj)|*.obj|All files (*.*)|*.*"),
        wxFD_OPEN);

    if (objModelFile != "")
    {
        SetStatusText("Loading the new model file...");
        if (_oglCanvas->setForegroundObject(objModelFile))
        {
            _oglCanvas->setIsNewFile(true);
            _oglCanvas->resetMatrix();
            _objModelFile = objModelFile;
        }
        SetStatusText(GetFileName(_objModelFile));
    }
}

void
ObjViewer::onMenuFileOpenBackgroundImage(wxCommandEvent& evt)
{
    std::string imageFile = wxFileSelector(wxT("Choose Image"), _dataFolder + "image", wxT(""), wxT(""),
        wxT("Image Files (*.bmp;*.pbm;*.pgm;*.ppm;*.sr;*.ras;*.jpeg;*.jpg;*.jpe;*.jp2;*.tiff;*.tif;*.png)| \
             *.bmp;*.pbm;*.pgm;*.ppm;*.sr;*.ras;*.jpeg;*.jpg;*.jpe;*.jp2;*.tiff;*.tif;*.png|All files (*.*)|*.*"),
        wxFD_OPEN);

    if (imageFile != "")
    {
        if (_oglCanvas->setBackgroundImamge(imageFile))
        {
            _oglCanvas->setIsNewFile(true);
            _imageFile = imageFile;
            _oglCanvas->resetMatrix();
            reLayout();
        }
    }
}

void
ObjViewer::onMenuFileSaveImage(wxCommandEvent& evt)
{
    wxFileDialog saveFileDialog(this, wxT("Save Image"), _dataFolder + "image", "",
        wxT("Image Files (*.bmp;*.pbm;*.pgm;*.ppm;*.sr;*.ras;*.jpeg;*.jpg;*.jpe;*.jp2;*.tiff;*.tif;*.png)| \
             *.bmp;*.pbm;*.pgm;*.ppm;*.sr;*.ras;*.jpeg;*.jpg;*.jpe;*.jp2;*.tiff;*.tif;*.png|All files (*.*)|*.*"),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...

    // save the current contents in the file;
    // this can be done with e.g. wxWidgets output streams:
    wxFileOutputStream output_stream(saveFileDialog.GetPath());
    if (!output_stream.IsOk())
    {
        wxLogError("Cannot save current contents in file '%s'.", saveFileDialog.GetPath());
        return;
    }

    std::string filename = saveFileDialog.GetPath();
    cv::Mat image = _oglCanvas->printScreen();
    imwrite(filename, image);
}

void 
ObjViewer::onMenuFileExit(wxCommandEvent& WXUNUSED(evt))
{
    // true is to force the frame to close
    Close(true);
}

void
ObjViewer::onMenuHelpAbout(wxCommandEvent& WXUNUSED(evt))
{
    wxString msg = "Mouse left button: rotation\n"
        + std::string("Mouse middle button: Move up, down, left, and right\n")
        + std::string("Mouse right button: Move forward and backward\n");
    wxMessageBox(msg);
}

void
ObjViewer::onRenderModeRadio(wxCommandEvent& WXUNUSED(evt))
{
    int renderMode = _renderModeRadioBox->GetSelection();
    if (renderMode != _renderMode)
    {
        _renderMode = renderMode;
        _oglCanvas->setRenderMode(renderMode);
    }
}

void
ObjViewer::onLightingCheck(wxCommandEvent& WXUNUSED(evt))
{
    _lightingOn = _lightingCheckBox->GetValue();
    _oglCanvas->setLightingOn(_lightingOn);
}

void
ObjViewer::onReset(wxCommandEvent& WXUNUSED(evt))
{
    _oglCanvas->resetMatrix();
}

void
ObjViewer::reLayout()
{
    int w, h;
    SetMinSize(wxSize(-1, -1));
    Fit();
    GetSize(&w, &h);
    SetMinSize(wxSize(w, h));
    Centre();
}

} // namespace ov
