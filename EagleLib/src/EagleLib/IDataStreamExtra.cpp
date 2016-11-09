#include "EagleLib/DataStream.hpp"
#include "EagleLib/Nodes/Node.h"
#include <cereal/types/vector.hpp>
#include <boost/filesystem.hpp>
#include "MetaObject/IO/Serializer.hpp"
#include "MetaObject/IO/Policy.hpp"
#include <EagleLib/IO/memory.hpp>

using namespace EagleLib;
template<typename AR> 
void DataStream::load(AR& ar)
{
    this->_load_parameters<AR>(ar, mo::_counter_<_DS_N_ - 1>());
    for(auto & node : top_level_nodes)
    {
        node->SetDataStream(this);
    }
    this->_load_parent<AR>(ar);
}

template<typename AR>
void DataStream::save(AR& ar) const
{
    this->_save_parameters<AR>(ar, mo::_counter_<_DS_N_ - 1>());
    this->_save_parent<AR>(ar);
}

IDataStream::Ptr IDataStream::Load(const std::string& config_file)
{
    rcc::shared_ptr<DataStream> stream = rcc::shared_ptr<DataStream>::Create();
    if (stream->LoadStream(config_file))
        return stream;
    return Ptr();
}
bool DataStream::LoadStream(const std::string& filename)
{
    StopThread();
    this->top_level_nodes.clear();
    if (!boost::filesystem::exists(filename))
    {
        LOG(warning) << "Stream config file doesn't exist: " << filename;
        return false;
    }
    std::string ext = boost::filesystem::extension(filename);
    if (ext == ".bin")
    {
        std::ifstream ifs(filename, std::ios::binary);
        cereal::BinaryInputArchive ar(ifs);
        ar(*this);
        return true;
    }
    else if (ext == ".json")
    {
        std::ifstream ifs(filename, std::ios::binary);
        cereal::JSONInputArchive ar(ifs);
        try
        {
            ar(*this);
        }
        catch (cereal::Exception&e)
        {

        }

        return true;
    }
    else if (ext == ".xml")
    {
        std::ifstream ifs(filename, std::ios::binary);
        cereal::XMLInputArchive ar(ifs);
        ar(*this);
        return true;
    }

    return false;
}

bool DataStream::SaveStream(const std::string& filename)
{
    if (boost::filesystem::exists(filename))
    {
        LOG(warning) << "Overwriting existing stream config file: " << filename;
    }
    std::string ext = boost::filesystem::extension(filename);
    if (ext == ".bin")
    {
        std::ofstream ofs(filename, std::ios::binary);
        cereal::BinaryOutputArchive ar(ofs);
        ar(*this);
        return true;
    }
    else if (ext == ".json")
    {
        std::ofstream ofs(filename, std::ios::binary);
        cereal::JSONOutputArchive ar(ofs);
        ar(*this);
        return true;
    }
    else if (ext == ".xml")
    {
        std::ofstream ofs(filename, std::ios::binary);
        cereal::XMLOutputArchive ar(ofs);
        ar(*this);
        return true;
    }

    return false;
}