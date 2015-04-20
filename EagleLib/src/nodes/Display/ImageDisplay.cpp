#include "nodes/Display/ImageDisplay.h"

using namespace EagleLib;

NODE_DEFAULT_CONSTRUCTOR_IMPL(QtImageDisplay)
NODE_DEFAULT_CONSTRUCTOR_IMPL(OGLImageDisplay)

QtImageDisplay::QtImageDisplay(boost::function<void(cv::Mat)> cpuCallback_)
{
    cpuDisplayCallback = cpuCallback_;
}
QtImageDisplay::QtImageDisplay(boost::function<void (cv::cuda::GpuMat)> gpuCallback_)
{
    gpuDisplayCallback = gpuCallback_;
}
void QtImageDisplay::Init(bool firstInit)
{
    if(firstInit)
    {
        updateParameter("Name", std::string(), Parameter::Control, "Set name for window");
    }
}

cv::cuda::GpuMat
QtImageDisplay::doProcess(cv::cuda::GpuMat& img, cv::cuda::Stream stream)
{
    if(img.channels() != 1 && img.channels() != 3)
    {
        log(Warning, "Image has " + boost::lexical_cast<std::string>(img.channels()) + " channels! Cannot display!");
        return img;
    }
    if(img.empty())
        return img;
    if(gpuDisplayCallback)
    {
        gpuDisplayCallback(img);
        return img;
    }
    cv::Mat h_img;
    img.download(h_img);
    if(cpuDisplayCallback)
    {
        cpuDisplayCallback(h_img);
        return img;
    }
    std::string name = getParameter<std::string>(0)->data;
    if(name.size() == 0)
    {
        name = fullTreeName;
    }
    try
    {
        cv::imshow(name, h_img);
    }catch(cv::Exception &err)
    {
        log(Warning, err.what());
    }
    parameters[0]->changed = false;
    return img;
}


OGLImageDisplay::OGLImageDisplay(boost::function<void(cv::cuda::GpuMat)> gpuCallback_)
{
    gpuDisplayCallback = gpuCallback_;
}

void OGLImageDisplay::Init(bool firstInit)
{
    if(firstInit)
    {
        updateParameter("Default Name", std::string("Default Name"), Parameter::Control, "Set name for window");
        cv::namedWindow("Default Name", cv::WINDOW_OPENGL);
        prevName = "Default Name";
    }
}

cv::cuda::GpuMat OGLImageDisplay::doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream stream)
{
    if(parameters[0]->changed)
    {
        cv::destroyWindow(prevName);
        prevName = getParameter<std::string>(0)->data;
        parameters[0]->changed = false;
        cv::namedWindow(prevName, cv::WINDOW_OPENGL);
    }
    if(gpuDisplayCallback)
    {
        gpuDisplayCallback(img);
        return img;
    }
    cv::imshow(prevName, img);
    return img;
}
