#pragma once

#include <Eigen/Dense>
#include <opencv2/core/eigen.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <AtlBase.h>
#include "ObjViewer.h"
#include "OVCommon.h"

namespace ov
{

#define TRACKBALLSIZE 0.8

std::string
GetFileName(std::string s);

Mat3
Trackball(const Vec2& prePos, const Vec2& curPos);

double
GetZValueFrom2DPoint(double r, const Vec2& pos);

Vec3
FromRotationMatirxToAxisAngle(const Mat3& R);

Mat3
FromAxisAngleToRotationMatrix(const Vec3& r);

} // namespace ov
