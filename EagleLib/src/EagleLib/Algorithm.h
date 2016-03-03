#pragma once

#include "ParameteredObject.h"

#include <opencv2/core/cuda.hpp>
#include <EagleLib/rcc/shared_ptr.hpp>
namespace EagleLib
{
    class EAGLE_EXPORTS Algorithm : public TInterface<IID_Algorithm, EagleLib::ParameteredIObject>
    {
        std::vector<shared_ptr<Algorithm>> child_algorithms;
    public:
        virtual std::vector<std::shared_ptr<Parameters::Parameter>> GetParameters() = 0;
    };
}