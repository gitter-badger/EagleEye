#include "EagleLib/SyncedMemory.h"
#include <EagleLib/utilities/GpuMatAllocators.h>
#include <EagleLib/utilities/CudaCallbacks.hpp>
#include <EagleLib/IO/cvMat.hpp>

#include <MetaObject/Logging/Log.hpp>

#include "MetaObject/Parameters/MetaParameter.hpp"
#include "MetaObject/Parameters/UI/Qt/OpenCV.hpp"
#include "MetaObject/Parameters/UI/Qt/Containers.hpp"
#include "MetaObject/Parameters/UI/Qt/TParameterProxy.hpp"
#include "MetaObject/Parameters/Buffers/CircularBuffer.hpp"
#include "MetaObject/Parameters/Buffers/StreamBuffer.hpp"
#include "MetaObject/Parameters/Buffers/map.hpp"
#include "MetaObject/Parameters/IO/CerealPolicy.hpp"
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <boost/lexical_cast.hpp>

INSTANTIATE_META_PARAMETER(EagleLib::SyncedMemory);
INSTANTIATE_META_PARAMETER(std::vector<EagleLib::SyncedMemory>);
INSTANTIATE_META_PARAMETER(cv::Mat);
INSTANTIATE_META_PARAMETER(std::vector<cv::Mat>);

using namespace EagleLib;
SyncedMemory::SyncedMemory():
    _ctx(nullptr)
{
    
}
SyncedMemory::SyncedMemory(const cv::Mat& h_mat):
    _ctx(nullptr)
{
    h_data.resize(1, h_mat);
    d_data.resize(1);
    sync_flags.resize(1, HOST_UPDATED);
}
SyncedMemory::SyncedMemory(const cv::cuda::GpuMat& d_mat):
    _ctx(nullptr)
{
    h_data.resize(1);
    d_data.resize(1, d_mat);
    sync_flags.resize(1, DEVICE_UPDATED);
}
SyncedMemory::SyncedMemory(const cv::Mat& h_mat, const cv::cuda::GpuMat& d_mat):
    _ctx(nullptr)
{
    h_data.resize(1, h_mat);
    d_data.resize(1, d_mat);
    sync_flags.resize(1, SYNCED);
}

SyncedMemory::SyncedMemory(const std::vector<cv::cuda::GpuMat> & d_mats):
    d_data(d_mats),
    _ctx(nullptr)
{
    sync_flags.resize(d_mats.size(), DEVICE_UPDATED);
}

SyncedMemory::SyncedMemory(const std::vector<cv::Mat>& h_mats):
    h_data(h_mats),
    _ctx(nullptr)
{
    sync_flags.resize(h_mats.size(), HOST_UPDATED);
}

SyncedMemory::SyncedMemory(const std::vector<cv::Mat>& h_mat, const std::vector<cv::cuda::GpuMat>& d_mat, SYNC_STATE state):
    h_data(h_mat), d_data(d_mat),
    _ctx(nullptr)
{
    assert(h_data.size() == d_data.size());
    sync_flags.resize(h_data.size(), state);
}

SyncedMemory::SyncedMemory(cv::MatAllocator* cpu_allocator, cv::cuda::GpuMat::Allocator* gpu_allocator):
    h_data(1, cv::Mat()), d_data(1, cv::cuda::GpuMat(gpu_allocator)), sync_flags(1, SYNCED),
    _ctx(nullptr)
{
    h_data[0].allocator = cpu_allocator;
}

const cv::Mat&                
SyncedMemory::GetMat(cv::cuda::Stream& stream, int index)
{
    if(index < 0 || index >= std::max(h_data.size(), d_data.size()))
        THROW(debug) << "Index (" << index << ") out of range [0," << std::max(h_data.size(), d_data.size()) << "]";
    if(sync_flags[index] == DO_NOT_SYNC)
        return h_data[index];
    if (sync_flags[index] == DEVICE_UPDATED)
    {
        d_data[index].download(h_data[index], stream);
        sync_flags[index] = SYNCED;
    }
    return h_data[index];
}

cv::Mat&                    
SyncedMemory::GetMatMutable(cv::cuda::Stream& stream, int index)
{
    if (index < 0 || index >= std::max(h_data.size(), d_data.size()))
        THROW(debug) << "Index (" << index << ") out of range [0," << std::max(h_data.size(), d_data.size()) << "]";
    if(sync_flags[index] == DO_NOT_SYNC)
        return h_data[index];
    if (sync_flags[index] == DEVICE_UPDATED)
        d_data[index].download(h_data[index], stream);
    sync_flags[index] = HOST_UPDATED;
    return h_data[index];
}

const cv::cuda::GpuMat&        
SyncedMemory::GetGpuMat(cv::cuda::Stream& stream, int index)
{
    if (index < 0 || index >= std::max(h_data.size(), d_data.size()))
        THROW(debug) << "Index (" << index << ") out of range [0," << std::max(h_data.size(), d_data.size()) << "]";
    if (sync_flags[index] == DO_NOT_SYNC)
        return d_data[index];
    if (sync_flags[index] == HOST_UPDATED)
    {
        d_data[index].upload(h_data[index], stream);
        sync_flags[index] = SYNCED;
    }
    
    return d_data[index];
}

cv::cuda::GpuMat&            
SyncedMemory::GetGpuMatMutable(cv::cuda::Stream& stream, int index)
{
    if (index < 0 || index >= std::max(h_data.size(), d_data.size()))
        THROW(debug) << "Index (" << index << ") out of range [0," << std::max(h_data.size(), d_data.size()) << "]";
    if (sync_flags[index] == DO_NOT_SYNC)
        return d_data[index];
    if (sync_flags[index] == HOST_UPDATED)
        d_data[index].upload(h_data[index], stream);
    sync_flags[index] = DEVICE_UPDATED;
    return d_data[index];
}

const std::vector<cv::Mat>&
SyncedMemory::GetMatVec(cv::cuda::Stream& stream)
{
    if (sync_flags.size() && sync_flags[0] == DO_NOT_SYNC)
        return h_data;
    for (int i = 0; i < sync_flags.size(); ++i)
    {
        if (sync_flags[i] == DEVICE_UPDATED)
            d_data[i].download(h_data[i], stream);
    }
    return h_data;
}

std::vector<cv::Mat>&
SyncedMemory::GetMatVecMutable(cv::cuda::Stream& stream)
{
    if (sync_flags.size() && sync_flags[0] == DO_NOT_SYNC)
        return h_data;
    for (int i = 0; i < sync_flags.size(); ++i)
    {
        if (sync_flags[i] == DEVICE_UPDATED)
            d_data[i].download(h_data[i], stream);
        sync_flags[i] = HOST_UPDATED;
    }
    
    return h_data;
}

const std::vector<cv::cuda::GpuMat>&
SyncedMemory::GetGpuMatVec(cv::cuda::Stream& stream)
{
    if (sync_flags.size() && sync_flags[0] == DO_NOT_SYNC)
        return d_data;
    for (int i = 0; i < sync_flags.size(); ++i)
    {
        if (sync_flags[i] == HOST_UPDATED)
            d_data[i].upload(h_data[i], stream);
    }
    return d_data;
}

std::vector<cv::cuda::GpuMat>&
SyncedMemory::GetGpuMatVecMutable(cv::cuda::Stream& stream)
{
    if (sync_flags.size() && sync_flags[0] == DO_NOT_SYNC)
        return d_data;
    for (int i = 0; i < sync_flags.size(); ++i)
    {
        if (sync_flags[i] == HOST_UPDATED)
            d_data[i].upload(h_data[i], stream);
        sync_flags[i] = DEVICE_UPDATED;
    }
    return d_data;
}
void SyncedMemory::ResizeNumMats(int new_size)
{
    h_data.resize(new_size);
    d_data.resize(new_size);
    sync_flags.resize(new_size);
}

SyncedMemory SyncedMemory::clone(cv::cuda::Stream& stream)
{
    SyncedMemory output;
    output.h_data.resize(h_data.size());
    output.d_data.resize(d_data.size());
    for(int i = 0; i < h_data.size(); ++i)
    {
        output.h_data[i] = h_data[i].clone();
        d_data[i].copyTo(output.d_data[i], stream);
    }
    output.sync_flags = sync_flags;
    return output;
}
int SyncedMemory::GetNumMats() const
{
    return h_data.size();
}
bool SyncedMemory::empty() const
{
    if(h_data.size() && d_data.size())
        return h_data[0].empty() && d_data[0].empty();
    if(h_data.size() && d_data.size() == 0)
        return h_data[0].empty();
    if(d_data.size() && h_data.size() == 0)
        return d_data[0].empty();
    return true;
}
void SyncedMemory::Synchronize(cv::cuda::Stream& stream)
{
    for(int i = 0; i < h_data.size(); ++i)
    {
        if (sync_flags[i] == DO_NOT_SYNC)
            continue;
        if(sync_flags[i] == HOST_UPDATED)
            d_data[i].upload(h_data[i], stream);
        else if(sync_flags[i] == DEVICE_UPDATED)
            d_data[i].download(h_data[i], stream);
        sync_flags[i] = SYNCED;
    }
}


void SyncedMemory::ReleaseGpu(cv::cuda::Stream& stream)
{
    for(int i = 0; i < d_data.size(); ++i)
    {
        if(sync_flags[i] == DEVICE_UPDATED)
            d_data[i].download(h_data[i], stream);
    }
    if(dynamic_cast<DelayedDeallocator*>(cv::cuda::GpuMat::defaultAllocator()))
    {
        EagleLib::cuda::enqueue_callback([this]
        {
            for(int i = 0; i < d_data.size(); ++i)
            {
                d_data[i].release();
            }
        }, stream);
    }else
    {
        for(int i = 0; i < d_data.size(); ++i)
        {
            d_data[i].release();
        }
    }
}

cv::Size SyncedMemory::GetSize() const
{
    if(d_data.empty() || h_data.empty())
        return cv::Size();
    cv::Size output;
    if(d_data[0].empty())
        output = h_data[0].size();
    else
        output = d_data[0].size();
    return output;
}

int SyncedMemory::GetChannels() const
{
    if(d_data.empty() && h_data.empty())
        return 0;
    if(d_data.size() == 1 && !d_data[0].empty())
        return d_data[0].channels();
    if(h_data.size() == 1 && !h_data[0].empty())
        return h_data[0].channels();
    if(d_data.size() > 1 && d_data[0].channels() == 1)
        return d_data.size();
    if(h_data.size() > 1 && h_data[0].channels() == 1)
        return h_data.size();
    return 0;
}

std::vector<int> SyncedMemory::GetShape() const
{
    std::vector<int> output;
    output.push_back(std::max(d_data.size(), h_data.size()));
    if(d_data.empty() && h_data.empty())
        return output;
    if(d_data.empty())
    {
        output.push_back(h_data[0].rows);
        output.push_back(h_data[0].cols);
        output.push_back(h_data[0].channels());
    }else
    {
        if(h_data.empty())
        {
            output.push_back(d_data[0].rows);
            output.push_back(d_data[0].cols);
            output.push_back(d_data[0].channels());
        }else
        {
            output.push_back(std::max(d_data[0].rows, h_data[0].rows));
            output.push_back(std::max(d_data[0].cols, h_data[0].cols));
            output.push_back(std::max(d_data[0].channels(), h_data[0].channels()));
        }
    }
    return output;
}
int SyncedMemory::GetDepth() const
{
    CV_Assert(d_data.size() || h_data.size());
    if(d_data.size())
        return d_data[0].depth();
    return h_data[0].depth();
}

int SyncedMemory::GetType() const
{
    CV_Assert(d_data.size() || h_data.size());
    if (d_data.size())
        return d_data[0].type();
    return h_data[0].type();
}
int SyncedMemory::GetDim(int dim) const
{
    if(dim == 0)
        return d_data.size();
    if(dim == 1 && d_data.size())
        return d_data[0].rows;
    if(dim == 2 && d_data.size())
        return d_data[0].cols;
    if(dim == 3 && d_data.size())
        return d_data[0].channels();
    return 0;
}
SyncedMemory::SYNC_STATE SyncedMemory::GetSyncState(int index) const
{
    CV_Assert(index < sync_flags.size() && index >= 0);
    return sync_flags[index];
}
template<typename A> void SyncedMemory::load(A& ar)
{
    ar(cereal::make_nvp("matrices", h_data));
    sync_flags.resize(h_data.size(), HOST_UPDATED);
    d_data.resize(h_data.size());
}
template<typename A> void SyncedMemory::save(A & ar) const
{
    ar(cereal::make_nvp("matrices", h_data));
}
mo::Context*                           GetContext()
{

}

void                                   SetContext(mo::Context* ctx)
{

}
