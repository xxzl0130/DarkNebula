#include "AdminNode.h"

dn::AdminNode::AdminNode(uint16_t receivePort, uint16_t sendPort)
{
	setReceivePort(receivePort);
	setSendPort(sendPort);
}

void dn::AdminNode::setReceivePort(uint16_t port)
{
	// TODO
	receivePort_ = port;
}

void dn::AdminNode::setSendPort(uint16_t port)
{
	// TODO
	sendPort_ = port;
}
