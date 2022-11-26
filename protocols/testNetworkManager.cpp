/*
 * =====================================================================================
 *
 *       Filename:  testNetworkManager.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/13/2019 11:37:29 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  陈焱 (Lee),
 *   Organization:
 *
 * =====================================================================================
 */
#include <string>
#include <exception>

#include "protocol/NetworkManager.h"

class Receiver : public protocols::ReceiverBase
{
public:
    Receiver() {
    }
    virtual ~Receiver() {
    }

private:
    void receive(tinyxml2::XMLDocument * xmlDocument, protocols::SenderBase * sender) {
    }

    void receive(char * buffer, size_t bufferSize, protocols::SenderBase * sender) {
    }

    void receive(Json::Value * rootValue, protocols::SenderBase * sender) {
    }

    void destroySender(protocols::SenderBase * sender) {
    }

    void connected(protocols::SenderBase * sender) {
    }

    void disconnected(protocols::SenderBase * sender) {
    }

    void replyUpper(protocols::SenderBase * sender, bool result, void * upperPointer) {
    }
};

int main(int argc, char *argv[])
{
    std::string configPath("/home/chenyan/projects/project1/protocols/configuration.xml");
    try
    {
        protocols::NetworkManager * network = new protocols::NetworkManager(configPath);
        Receiver * receiver = new Receiver();
        network->preStart(receiver);
        network->startUdpServer();
        network->startTcpServer();
        network->startUdpClient();
        network->startTcpClient();

        // network->stopUdpServer();
        // network->stopTcpServer();
        // network->stopUdpClient();
        // network->stopTcpClient();
        network->stopAll();
        printf("%s:%d\n", __FUNCTION__, __LINE__);
        // sleep(60 * 2);
        delete network;
        network = NULL;
        delete receiver;
        receiver = NULL;
    }
    catch(std::exception & ex) {
        printf("%s\n", ex.what());
    }
    return 0;
}
