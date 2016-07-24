// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#define wxUSE_GUI 1

#include "wx/glcanvas.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iomanip>
#include "ObjViewer.h"
#include "glm/glm.h"
#include "OVCanvas.h"
#include "OVUtil.h"
#include "OVCommon.h"

namespace ov
{

#ifndef GL_BGR
    #define GL_BGR GL_BGR_EXT
#endif

#ifndef GL_BGRA
    #define GL_BGRA GL_BGRA_EXT
#endif

int OVCanvas::FrameWidth = 800;
int OVCanvas::FrameHeight = 600;
double OVCanvas::PlaneNear = 0.01;
double OVCanvas::PlaneFar = 100;

OVCanvas::OVCanvas(ObjViewer *objViewer,
                   wxWindowID id,
                   wxPoint pos,
                   wxSize size,
                   long style,
                   wxString name)
    : wxGLCanvas(objViewer, id, NULL, pos, size, style | wxFULL_REPAINT_ON_RESIZE, name)
{
    _objViewer = objViewer;

    // Offset transformation coefficients
    _offsetRotation = { 180, 0, 0 };
    _offsetTranslation = { 0, 0, 7 };
    _offsetScale = 1;

    _oglContext = NULL;
    _glmModel = NULL;
    _renderMode = RENDER_SOLID;
    resetMatrix();

    Connect(ID_ANY, wxEVT_PAINT, wxPaintEventHandler(OVCanvas::onPaint));
    Connect(ID_ANY, wxEVT_SIZE, wxSizeEventHandler(OVCanvas::onSize));
    Connect(ID_ANY, wxEVT_IDLE, wxIdleEventHandler(OVCanvas::onIdle));

    // Explicitly create a new rendering context instance for this canvas.
    _oglContext = new wxGLContext(this);
    
    oglInit();
}

OVCanvas::~OVCanvas()
{
    if (_oglContext) delete _oglContext;
    if (_glmModel) delete _glmModel;
}

bool
OVCanvas::setGlmModel(std::string modelFile)
{
    if (_glmModel) delete _glmModel;
    _glmModel = glmReadOBJ(modelFile.c_str());
    if (!_glmModel)
    {
        wxString msg = "Failed to open \"" + modelFile + "\"...";
        wxMessageBox(msg, wxT("Error"), wxICON_ERROR);
        return false;
    }
    glmUnitize(_glmModel);
    glmFacetNormals(_glmModel);
    glmVertexNormals(_glmModel, 90.0);
    return true;
}

bool
OVCanvas::setBackgroundImamge(std::string imageFile)
{
    cv::Mat cameraImage = cv::imread(imageFile, CV_LOAD_IMAGE_COLOR);
    if (cameraImage.empty())
    {
        wxString msg = "Cannot open \"" + imageFile + "\".\n";
        wxMessageBox(msg, wxT("Error"), wxICON_ERROR);
        return false;
    }
    else
    {
        _backgroundImage = cameraImage;
        FrameWidth = _backgroundImage.cols;
        FrameHeight = _backgroundImage.rows;
    }

    // After getting the projection matrix, we do resize one time
    SetClientSize(wxSize(OVCanvas::FrameWidth, OVCanvas::FrameHeight));
    SetMinClientSize(wxSize(OVCanvas::FrameWidth, OVCanvas::FrameHeight));
    onSize(wxSizeEvent());

    return true;
}

void
OVCanvas::setRenderMode(int renderMode)
{
    _renderMode = renderMode;
    Refresh();
}

void
OVCanvas::onIdle(wxIdleEvent& WXUNUSED(evt))
{
    // Nothing to do
}

void
OVCanvas::onPaint(wxPaintEvent& WXUNUSED(evt))
{
    SetCurrent(*_oglContext);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    drawBackground();
    glDisable(GL_TEXTURE_2D);

    // Render the foreground target
    glClear(GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glMultMatrixd(_modelViewMatrix);
    glTranslatef(_offsetTranslation[0], _offsetTranslation[1], _offsetTranslation[2]);
    glRotatef(_offsetRotation[2], 0, 0, 1);
    glRotatef(_offsetRotation[1], 0, 1, 0);
    glRotatef(_offsetRotation[0], 1, 0, 0);
    glScalef(_offsetScale, _offsetScale, _offsetScale);

    // Lighting
    const GLfloat a[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    const GLfloat d[] = { 0.5f, 0.5f, 0.5f, 0.5f };
    const GLfloat s[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const GLfloat p0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat p1[] = { 1.0f, -1.0f, 1.0f, 1.0f };
    const GLfloat p2[] = { -1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat p3[] = { -1.0f, -1.0f, 1.0f, 1.0f };
    const GLfloat p4[] = { 1.0f, 1.0f, -1.0f, 1.0f };
    const GLfloat p5[] = { 1.0f, -1.0f, -1.0f, 1.0f };
    const GLfloat p6[] = { -1.0f, 1.0f, -1.0f, 1.0f };
    const GLfloat p7[] = { -1.0f, -1.0f, -1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, a);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, d);
    glLightfv(GL_LIGHT0, GL_SPECULAR, s);
    glLightfv(GL_LIGHT0, GL_POSITION, p0);
    glLightfv(GL_LIGHT1, GL_AMBIENT, a);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, d);
    glLightfv(GL_LIGHT1, GL_SPECULAR, s);
    glLightfv(GL_LIGHT1, GL_POSITION, p1);
    glLightfv(GL_LIGHT2, GL_AMBIENT, a);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, d);
    glLightfv(GL_LIGHT2, GL_SPECULAR, s);
    glLightfv(GL_LIGHT2, GL_POSITION, p2);
    glLightfv(GL_LIGHT3, GL_AMBIENT, a);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, d);
    glLightfv(GL_LIGHT3, GL_SPECULAR, s);
    glLightfv(GL_LIGHT3, GL_POSITION, p3);
    glLightfv(GL_LIGHT4, GL_AMBIENT, a);
    glLightfv(GL_LIGHT4, GL_DIFFUSE, d);
    glLightfv(GL_LIGHT4, GL_SPECULAR, s);
    glLightfv(GL_LIGHT4, GL_POSITION, p4);
    glLightfv(GL_LIGHT5, GL_AMBIENT, a);
    glLightfv(GL_LIGHT5, GL_DIFFUSE, d);
    glLightfv(GL_LIGHT5, GL_SPECULAR, s);
    glLightfv(GL_LIGHT5, GL_POSITION, p5);
    glLightfv(GL_LIGHT6, GL_AMBIENT, a);
    glLightfv(GL_LIGHT6, GL_DIFFUSE, d);
    glLightfv(GL_LIGHT6, GL_SPECULAR, s);
    glLightfv(GL_LIGHT6, GL_POSITION, p6);
    glLightfv(GL_LIGHT7, GL_AMBIENT, a);
    glLightfv(GL_LIGHT7, GL_DIFFUSE, d);
    glLightfv(GL_LIGHT7, GL_SPECULAR, s);
    glLightfv(GL_LIGHT7, GL_POSITION, p7);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_LIGHT4);
    glEnable(GL_LIGHT5);
    glEnable(GL_LIGHT6);
    glEnable(GL_LIGHT7);
    glEnable(GL_LIGHTING);

    // Semitransparent effect 
    glEnable(GL_BLEND);
    // Remove back faces
    glEnable(GL_CULL_FACE); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (_renderMode == RENDER_SOLID)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawModel();
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    glFlush();
    SwapBuffers();
}

void
OVCanvas::onSize(wxSizeEvent& WXUNUSED(evt))
{
    if (!IsShownOnScreen())
        return;

    int w, h;
    GetClientSize(&w, &h);

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMultMatrixd(_projectionMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Refresh();
}

void
OVCanvas::oglInit()
{
    SetCurrent(*_oglContext);

    // OpenGL initialization
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &_textureId);
}

void OVCanvas::drawBackground()
{
    SetCurrent(*_oglContext);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // set texture filter to linear - we do not build mipmaps for speed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // create the texture from OpenCV image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FrameWidth, FrameHeight, 0, GL_BGR,
        GL_UNSIGNED_BYTE, _backgroundImage.data);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // draw the quad textured with the camera image
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex2f(-1, -1);
    glTexCoord2f(0, 0);
    glVertex2f(-1, 1);
    glTexCoord2f(1, 0);
    glVertex2f(1, 1);
    glTexCoord2f(1, 1);
    glVertex2f(1, -1);
    glEnd();

    // reset the projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void
OVCanvas::drawModel()
{
    SetCurrent(*_oglContext);
    GLuint drawMode = (_glmModel->texcoords) ? GLM_SMOOTH | GLM_TEXTURE | GLM_MATERIAL : GLM_SMOOTH | GLM_COLOR;
    glmDraw(_glmModel, drawMode);
}

void
OVCanvas::resetMatrix()
{
    // Get the default camera parameters accordring to the image size
    double fx = Vec2(FrameWidth, FrameHeight).norm();
    double fy = fx;
    double cx = (FrameWidth - 1.) / 2.;
    double cy = (FrameHeight - 1.) / 2.;
    double w = FrameWidth;
    double h = FrameHeight;

    // Set the projection matrix for opengl
    _projectionMatrix[0] = 2 * fx / w;
    _projectionMatrix[1] = 0;
    _projectionMatrix[2] = 0;
    _projectionMatrix[3] = 0;
    _projectionMatrix[4] = 0;
    _projectionMatrix[5] = -2 * fy / h;
    _projectionMatrix[6] = 0;
    _projectionMatrix[7] = 0;
    _projectionMatrix[8] = 2 * (cx / w) - 1;
    _projectionMatrix[9] = 1 - 2 * (cy / h);
    _projectionMatrix[10] = (PlaneFar + PlaneNear) / (PlaneFar - PlaneNear);
    _projectionMatrix[11] = 1;
    _projectionMatrix[12] = 0;
    _projectionMatrix[13] = 0;
    _projectionMatrix[14] = 2 * PlaneFar*PlaneNear / (PlaneNear - PlaneFar);
    _projectionMatrix[15] = 0;

    // Set the extrinsic matrix
    memset(_modelViewMatrix, 0, 16 * sizeof(double));
    _modelViewMatrix[0] = _modelViewMatrix[5] = _modelViewMatrix[10] = _modelViewMatrix[15] = 1;
}

cv::Mat
OVCanvas::printScreen()
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    int x, y, w, h;
    x = vp[0];
    y = vp[1];
    w = vp[2];
    h = vp[3];

    int j;

    unsigned char *bottomup_pixel = (unsigned char *)malloc(w*h * 3 * sizeof(unsigned char));
    unsigned char *topdown_pixel = (unsigned char *)malloc(w*h * 3 * sizeof(unsigned char));

    //Byte alignment (that is, no alignment)
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(x, y, w, h, GL_BGR, GL_UNSIGNED_BYTE, bottomup_pixel);
    for (j = 0; j < h; j++)
        memcpy(&topdown_pixel[j*w * 3], &bottomup_pixel[(h - j - 1)*w * 3], w * 3 * sizeof(unsigned char));

    cv::Mat image = cv::Mat(h, w, CV_8UC3, topdown_pixel);
    return image;
}

void
OVCanvas::forceRender(const Mat3& R, const Vec3& t)
{
    // Fill in modelViewMatrix
    for (int i = 0; i < 3; ++i)
    {
        _modelViewMatrix[12 + i] = t(i, 0);
        for (int j = 0; j < 3; ++j)
            _modelViewMatrix[i * 4 + j] = R(j, i);
    }
    _modelViewMatrix[3] = _modelViewMatrix[7] = _modelViewMatrix[11] = 0;
    _modelViewMatrix[15] = 1;
    onPaint(wxPaintEvent());
}

} // namespace ov