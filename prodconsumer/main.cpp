#include <tdmonitor/types.h>
#include <tdmonitor/cluster.h>
#include <tdmonitor/monitor.h>

#include <string>
#include <vector>
#include <memory>
#include <iostream>

class ConsumerProducer {
public:
    ConsumerProducer(TDM::Cluster &cluster, const std::string &name, int toProduce) :
        m_stop(false), m_toProduce(toProduce), m_num(0), m_monitor(cluster, name)
    {
        m_monitor.property("messages", m_messages);
        m_monitor.property("seqid", m_num);
        m_condFree = m_monitor.condition("free");
        m_condData = m_monitor.condition("data");
        m_monitor.create();
    }

    ~ConsumerProducer()
    {
        m_stop = true;

        if (m_consumerThread.joinable())
            m_consumerThread.join();

        if (m_producerThread.joinable())
            m_producerThread.join();
    }

    void consume()
    {
        m_monitor.lock();

        while (m_messages.size() == 0) {
            std::cout << "Konsument czeka" << std::endl;
            m_condData->wait();
        }

        std::cout << "Konsument: SeqID " << m_num++ << std::endl;

        std::cout << "Skonsumowano: " << m_messages.back() << std::endl;
        m_messages.pop_back();

        m_condFree->signal();

        m_monitor.unlock();
    }

    void produce()
    {
        m_monitor.lock();

        while (m_messages.size() == 10) {
            std::cout << "Producent czeka" << std::endl;
            m_condFree->wait();
        }

        std::cout << "Konsument: SeqID " << m_num++ << std::endl;

        std::cout << "Wyprodukowano: " << m_toProduce << std::endl;
        m_messages.push_back(m_toProduce);

        m_condData->signal();

        m_monitor.unlock();
    }

    void startThreads()
    {
        m_producerThread = std::thread([this]() {
            while (!m_stop) {
                produce();
            }
        });

        m_consumerThread = std::thread([this]() {
            while (!m_stop) {
                consume();
            }
        });
    }

private:
    volatile bool m_stop;
    std::thread m_producerThread;
    std::thread m_consumerThread;
    int m_toProduce;
    std::vector<int> m_messages;
    int m_num;
    TDM::Monitor m_monitor;
    std::unique_ptr<TDM::MonitorConditionVariable> m_condFree;
    std::unique_ptr<TDM::MonitorConditionVariable> m_condData;
};

int main(int argc, char *argv[])
{
    std::vector<std::string> nodes;

    std::string listenAddress;
    uint32_t nodeId;

    if (argc < 4) {
        std::cout << "program adres_nasłuchujący node_id node0 nodeAddr1 nodeAddr2 nodeAddr3 nodeAddr4" << std::endl;
        return 0;
    }

    listenAddress = argv[1];
    nodeId = std::stoul(argv[2]);

    for (int i = 3; i < argc; ++i) {
        nodes.push_back(argv[i]);
    }

    if (nodeId > nodes.size() - 1) {
        std::cout << "nodeId za duży";
        return 0;
    }

    TDM::Cluster *cluster = new TDM::Cluster(nodes, nodeId, listenAddress);

    ConsumerProducer *worker = new ConsumerProducer(*cluster, "consumer-producer", nodeId);
    ConsumerProducer *worker2 = new ConsumerProducer(*cluster, "drugi-monitor", nodes.size() + nodeId);

    worker->startThreads();
    worker2->startThreads();

    while (1) {
        // ...
    }

    delete worker;
    delete worker2;
    delete cluster;

    return 0;
}
