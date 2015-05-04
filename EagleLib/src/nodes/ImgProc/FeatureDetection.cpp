#include "nodes/ImgProc/FeatureDetection.h"
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudaimgproc.hpp>
using namespace EagleLib;

void GoodFeaturesToTrackDetector::Init(bool firstInit)
{
    updateParameter("Feature Detector", cv::cuda::createGoodFeaturesToTrackDetector(CV_8UC1), Parameter::Output);
    updateParameter("Max corners", int(1000), Parameter::Control);
    updateParameter("Quality Level", double(0.01));
    updateParameter("Min Distance", double(0.0), Parameter::Control, "The minimum distance between detected points");
    updateParameter("Block Size", int(3));
    updateParameter("Use harris", false);
    updateParameter("Harris K", double(0.04));
}


cv::cuda::GpuMat
GoodFeaturesToTrackDetector::doProcess(cv::cuda::GpuMat& img, cv::cuda::Stream stream)
{

    if(parameters[1]->changed || parameters[2]->changed ||
       parameters[3]->changed || parameters[4]->changed ||
       parameters[5]->changed || parameters[6]->changed)
    {
        //updateParameter(0,
            cv::cuda::createGoodFeaturesToTrackDetector(CV_8UC1,
                getParameter<int>(1)->data, getParameter<double>(2)->data,
                getParameter<double>(3)->data, getParameter<int>(4)->data,
                getParameter<bool>(5)->data, getParameter<double>(6)->data);//, Parameter::Output);
        log(Status, "Feature Detector updated");
        parameters[1]->changed = false;
        parameters[2]->changed = false;
        parameters[3]->changed = false;
        parameters[4]->changed = false;
        parameters[5]->changed = false;
        parameters[6]->changed = false;
    }

    cv::cuda::GpuMat greyImg;
    if(img.channels() != 1)
    {
        // Internal greyscale conversion
        cv::cuda::cvtColor(img, greyImg, CV_BGR2GRAY,0, stream);
    }else
    {
        greyImg = img;
    }
    auto detectorParam = getParameter<cv::Ptr<cv::cuda::CornersDetector>>("Feature Detector");
    if(detectorParam == nullptr)
    {

        return img;
    }
    cv::Ptr<cv::cuda::CornersDetector> detector = detectorParam->data;
    if(detector == nullptr)
    {
        log(Error, "Detector not built");
        return img;
    }
    cv::cuda::GpuMat detectedCorners;
    detector->detect(greyImg, detectedCorners, cv::cuda::GpuMat(), stream);
    updateParameter("Detected Corners", detectedCorners, Parameter::Output);
    updateParameter("Num corners", detectedCorners.cols, Parameter::State);
    return img;
}
NODE_DEFAULT_CONSTRUCTOR_IMPL(GoodFeaturesToTrackDetector)
