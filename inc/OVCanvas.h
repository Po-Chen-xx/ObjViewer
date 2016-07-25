#pragma once

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#define wxUSE_GUI 1

#include <fstream>
#include <queue>
#include "wx/glcanvas.h"
#include "ObjViewer.h"
#include "glm/glm.h"
#include "OVCommon.h"

namespace ov
{

class ObjViewer;
class PenPoseTracker;

// The canvas window
class OVCanvas : public wxGLCanvas
{
public:
    OVCanvas(ObjViewer *objViewer,
             wxWindowID id = wxID_ANY,
             wxPoint pos = wxDefaultPosition,
             wxSize size = wxDefaultSize, long style = 0,
             wxString name = wxT("OVCanvas"));

    ~OVCanvas();

    static int FrameWidth;
    static int FrameHeight;
    static double PlaneNear;
    static double PlaneFar;

    void setRenderMode(int renderMode);
    bool setGlmModel(std::string objModelFile);
    bool setBackgroundImamge(std::string imageFile);
    cv::Mat printScreen();
    void resetMatrix();
    void setIsNewModel(bool isNewModel) { _isNewModel = isNewModel; }

protected:
    void onMouse(wxMouseEvent& evt);
    void onMouseWheel(wxMouseEvent& evt);
    void onIdle(wxIdleEvent& evt);
    void onPaint(wxPaintEvent& evt);
    void onSize(wxSizeEvent& evt);

private:
    // OpenGL functions
    void oglInit();
    void drawBackground();
    void drawModel();

    // Foreground and background objects
    ObjViewer*   _objViewer;
    wxGLContext* _oglContext;
    GLMmodel*    _glmModel;
    cv::Mat      _backgroundImage;
    GLuint       _textureId;

    // Selection of source and destination
    int  _renderMode;
    bool _isNewModel;

    // Offset transformation coefficients
    std::vector<double> _offsetRotation;
    std::vector<double> _offsetTranslation;
    double              _offsetScale;

    // For OpenGL rendering
    double _modelViewMatrix[16];
    double _projectionMatrix[16];
    Mat3   _R;
    Vec3   _t;

    // For trackball
    Vec2 _mousePos;
};

} // namespace ov