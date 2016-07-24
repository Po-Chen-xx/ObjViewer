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

} // namespace ov
