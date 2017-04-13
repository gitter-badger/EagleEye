#pragma once
#include <Aquila/Algorithm.h>
#include <MetaObject/IMetaObjectInfo.hpp>
#include "ros/ros.h"
#include "ros/topic.h"
#include "ros/master.h"
#include "RosInterface.hpp"

namespace ros
{
class MessageReaderInfo: public mo::IMetaObjectInfo
{
public:
    virtual int CanHandleTopic(const std::string& type) const = 0;
    virtual void ListTopics(std::vector<std::string>& topics) const = 0;
};

class IMessageReader: public TInterface<ctcrc32("IMessageReader"), aq::Algorithm>
{
public:
    template<class T, int P> static int CanHandleTopic(const std::string& topic)
    {
        ros::master::V_TopicInfo ti;
        if(!ros::master::getTopics(ti))
            return 0;
        for(ros::master::V_TopicInfo::iterator it = ti.begin(); it != ti.end(); ++it)
        {
            if(it->name == topic)
            {
                if(it->datatype == ros::message_traits::DataType<T>::value())
                {
                    return P;
                }
            }
        }
        return 0;
    }
    template<class T> static void ListTopics(std::vector<std::string>& topics)
    {
        aq::RosInterface::Instance();
        ros::master::V_TopicInfo ti;
        if(!ros::master::getTopics(ti))
            return;
        for(ros::master::V_TopicInfo::iterator it = ti.begin(); it != ti.end(); ++it)
        {
            if(it->datatype == ros::message_traits::DataType<T>::value())
            {
                topics.push_back(it->name);
            }
        }
    }


    MO_BEGIN(IMessageReader)
        PARAM(std::string, subscribed_topic, "")
        //MO_SLOT(void, on_subscribed_topic_modified, mo::Context*, mo::IParameter*)
        PARAM_UPDATE_SLOT(subscribed_topic)
    MO_END;
    typedef MessageReaderInfo InterfaceInfo;
    static std::vector<std::string> ListSubscribableTopics();
    static rcc::shared_ptr<IMessageReader> Create(const std::string& topic);
    static int CanLoadTopic(const std::string& topic);
    virtual bool Subscribe(const std::string& topic) = 0;
};

} // namespace ros
