#include <tdmonitor/cluster.h>
#include <tdmonitor/monitor.h>

#include <string>
#include <vector>

int main(int argc, char *argv[])
{
    std::vector<std::string> nodes;
    nodes.push_back("127.0.0.1:31410");

    TDM::Cluster cluster(nodes, 0, "0.0.0.0:31410");

    TDM::Monitor monitor(cluster, "producent");

    monitor.create();

    monitor.lock();
    monitor.unlock();

    return 0;
}
