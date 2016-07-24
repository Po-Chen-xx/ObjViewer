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
    void forceRender(const Mat3& R, const Vec3& t);
    cv::Mat printScreen();
    void resetMatrix();

protected:
    void onIdle(wxIdleEvent& evt);
    void onPaint(wxPaintEvent& evt);
    void onSize(wxSizeEvent& evt);
    void OnMouse(wxMouseEvent& event);

private:
    // OpenGL functions
    void oglInit();
    void drawBackground();
    void drawModel();

    // Foreground and background objects
    ObjViewer*      _objViewer;
    wxGLContext*    _oglContext;
    GLMmodel*       _glmModel;
    cv::Mat         _backgroundImage;
    GLuint          _textureId;

    // Selection of source and destination
    int  _renderMode;

    // Offset transformation coefficients
    std::vector<double> _offsetRotation;
    std::vector<double> _offsetTranslation;
    double              _offsetScale;

    // For OpenGL rendering
    double _modelViewMatrix[16];
    double _projectionMatrix[16];

    // For trackball
    double _mouseX;
    double _mouseY;
    double _quat[4];
};

} // namespace ov