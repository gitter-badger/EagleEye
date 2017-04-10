#pragma once

#include "MetaObject/Detail/HelperMacros.hpp"
#include "MetaObject/MetaObjectInfo.hpp"
#include "IRosMessageReader.hpp"

namespace mo
{
template<class Type>
struct MetaObjectInfoImpl<Type, ros::MessageReaderInfo>: public ros::MessageReaderInfo
{
    int CanHandleTopic(const std::string& type) const
    {
        return Type::CanHandleTopic(type);
    }
    void ListTopics(std::vector<std::string>& topics) const
    {
        Type::ListTopics(topics);
    }
};
}
