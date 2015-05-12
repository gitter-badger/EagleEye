#pragma once

#include <nodes/Node.h>
#include "CudaUtils.hpp"

namespace EagleLib
{
    class SetDevice: public Node
    {
        bool firstRun;
    public:
        SetDevice();
        virtual bool SkipEmpty() const;
        virtual void Init(bool firstInit);
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream& stream = cv::cuda::Stream::Null());
    };

    class StreamDispatcher: public Node
    {
        ConstBuffer<cv::cuda::Stream> streams;
    public:
        StreamDispatcher();
        virtual bool SkipEmpty() const;
        virtual void Init(bool firstInit);
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream& stream = cv::cuda::Stream::Null());
    };


}
