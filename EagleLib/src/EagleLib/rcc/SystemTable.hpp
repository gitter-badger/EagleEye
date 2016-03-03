#pragma once

#include <parameters/LokiTypeInfo.h>
#include <map>
namespace EagleLib
{
	class EventHandler;
}
namespace Freenect
{
	class Freenect;
}
struct SystemTable
{
	SystemTable();

	EagleLib::EventHandler* eventHandler;
	Freenect::Freenect* freenect;
    // These are per stream singletons
	std::map<Loki::TypeInfo, std::map<int,void*>> singletons;
    // These are global single instance singletons
    std::map<Loki::TypeInfo, void*> g_singletons;

	template<typename T> T* GetSingleton(int stream_id = 0)
	{
        // Check global singletons first regardless of stream_id
        auto g_itr = g_singletons.find(Loki::TypeInfo(typeid(T)));
        if(g_itr != g_singletons.end())
        {
            return static_cast<T*>(g_itr->second);
        }
		auto& stream_map = singletons[Loki::TypeInfo(typeid(T))];
        auto itr = stream_map.find(stream_id);
		if (itr != stream_map.end())
		{
			return static_cast<T*>(itr->second);
		}
        return nullptr;
	}
	template<typename T> void SetSingleton(T* singleton, int stream_id = 0)
	{
        if(stream_id == -1)
        {
            g_singletons[Loki::TypeInfo(typeid(T))] = singleton;
            return;
        }
        singletons[Loki::TypeInfo(typeid(T))][stream_id] = singleton;
		/*auto itr = singletons.find(Loki::TypeInfo(typeid(T)));
		if (itr == singletons.end())
		{
			singletons[Loki::TypeInfo(typeid(T))] = static_cast<void*>(singleton);
		}*/
	}

};