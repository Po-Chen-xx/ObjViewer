#include <Eigen/Dense>
#include <opencv2/core/eigen.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <AtlBase.h>
#include <iomanip>
#include "OVUtil.h"
#include "OVCommon.h"

namespace ov
{

const double PI = 3.1415927;
const double ROT_EPS = 1e-15;

std::string
GetFileName(std::string s)
{
    char sep1 = '/', sep2 = '\\';

    size_t i = s.rfind(sep1, s.length());
    size_t j = s.rfind(sep2, s.length());

    if (i != std::string::npos)
        return(s.substr(i + 1, s.length() - i));
    else if (j != std::string::npos)
        return(s.substr(j + 1, s.length() - j));
    else
        return("");
}

// Simulate a trackball by projecting the previous and current points onto
// a virtual sphere, and then compute the related rotation matrix.
// August '88 issue of Siggraph's "Computer Graphics," pp. 121-129
Mat3
Trackball(const Vec2& prePos, const Vec2& curPos)
{
    if (prePos == curPos)
        return Mat3::Identity();
    
    Vec3 p1(prePos(0), prePos(1), GetZValueFrom2DPoint(TRACKBALLSIZE, prePos));
    Vec3 p2(curPos(0), curPos(1), GetZValueFrom2DPoint(TRACKBALLSIZE, curPos));
    Vec3 axis = (p1.cross(p2)).normalized();

    // Rotation amout
    double t = (p1 - p2).norm() / 2.0 / TRACKBALLSIZE;
    t = (t > 1) ? 1 : (t < -1) ? -1 : t;
    double phi = 2.0 * asin(t);
    return FromAxisAngleToRotationMatrix(axis * phi);
}

// Project a 2D point onto a sphere of radius r or a hyperboic sheet
// to get the z value
double
GetZValueFrom2DPoint(double r, const Vec2& pos)
{
    double d = pos.norm();
    if (d < r / sqrt(2))
        return sqrt(r * r - d * d);
    else
        return r * r / 2.0 / d;
}

Vec3
FromRotationMatirxToAxisAngle(const Mat3& R)
{
    double a = acos((R.trace() - 1) / 2);
    Vec3 r;
    if (a < ROT_EPS)
    {
        r << R(2, 1) - R(1, 2), R(0, 2) - R(2, 0), R(1, 0) - R(0, 1);
        r *= 0.5;
    }
    else if (a >(PI - ROT_EPS))
    {
        Mat3 S = 0.5 * (R - Mat3::Identity());
        double b = sqrt(S(0, 0) + 1);
        double c = sqrt(S(1, 1) + 1);
        double d = sqrt(S(2, 2) + 1);
        if (b > ROT_EPS)
        {
            c = S(1, 0) / b;
            d = S(2, 0) / b;
        }
        else if (c > ROT_EPS)
        {
            b = S(0, 1) / c;
            d = S(2, 1) / c;
        }
        else
        {
            b = S(0, 2) / d;
            c = S(1, 2) / d;
        }
        r << b, c, d;
    }
    else
    {
        r << R(2, 1) - R(1, 2), R(0, 2) - R(2, 0), R(1, 0) - R(0, 1);
        r *= (a / 2 / sin(a));
    }
    return r;
}

Mat3
FromAxisAngleToRotationMatrix(const Vec3& r)
{
    double rx, ry, rz;
    rx = r(0);
    ry = r(1);
    rz = r(2);
    Mat3 I = Mat::Identity(3, 3);
    Mat3 W = (Mat3() << 0, -rz, ry, rz, 0, -rx, -ry, rx, 0).finished();
    Mat3 W2 = W * W;
    const double a = r.norm();
    if (a < ROT_EPS)
        return I + W + 0.5 * W2;
    else
        return I + W * sin(a) / a + W2 * (1 - cos(a)) / (a * a);
}

} // namespace ov
