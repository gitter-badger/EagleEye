#include "Manager.h"
#include "nodes/Node.h"

int main()
{
    auto node = EagleLib::NodeManager::getInstance().addNode("SerialStack");
    return 0;
}
