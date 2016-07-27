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
#include <unordered_map>
#include "wx/glcanvas.h"
#include "ObjViewer.h"
#include "OVCommon.h"
#include "TinyObjLoader.h"

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
    bool setForegroundObject(const std::string& filename, bool isUnitization = true);
    bool setBackgroundImamge(const std::string& filename);
    cv::Mat printScreen();
    void resetMatrix();
    void setIsNewFile(bool isNewFile) { _isNewFile = isNewFile; }
    void setLightingOn(bool lightingOn);

protected:
    void onMouse(wxMouseEvent& evt);
    void onMouseWheel(wxMouseEvent& evt);
    void onIdle(wxIdleEvent& evt);
    void onPaint(wxPaintEvent& evt);
    void onSize(wxSizeEvent& evt);

private:
    // OpenGL functions
    void oglInit();
    void drawBackground(GLuint backgroundImageTextureId);
    void drawForeground(const std::vector<tinyobj::shape_t>& shapes,
                        const std::vector<tinyobj::material_t>& materials,
                        const std::unordered_map<std::string, GLuint>& textureIds);
    void unitize(std::vector<tinyobj::shape_t>& shapes);

    // Widgets
    ObjViewer*   _objViewer;
    wxGLContext* _oglContext;

    // Foreground objects
    std::vector<tinyobj::shape_t>           _shapes;
    std::vector<tinyobj::material_t>        _materials;
    std::unordered_map<std::string, GLuint> _textureIds;

    // Backgroubd objects
    cv::Mat _backgroundImage;
    GLuint  _backgroundImageTextureId;

    // Selections
    int  _renderMode;
    bool _isNewFile;
    bool _lightingOn;

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