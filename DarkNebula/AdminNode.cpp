#include "AdminNode.h"

#include <cassert>
#include <functional>
#include <zmq.h>
#include <sstream>
#include <thread>

dn::AdminNode::AdminNode(uint16_t receivePort, uint16_t sendPort):
	simState_(SimNop),
	pubSocket_(nullptr),
	subSocket_(nullptr),
	socketContext_(zmq_ctx_new()),
	listenThread_(nullptr),
	listenStop_(false),
	listenStopped_(false),
	buffer(nullptr)
{
	setBufferSize(4 * 1024 * 1024);
	setReceivePort(receivePort);
	setSendPort(sendPort);
}

dn::AdminNode::~AdminNode()
{
	stopListen();
	zmq_close(pubSocket_);
	zmq_close(subSocket_);
	zmq_ctx_destroy(socketContext_);
	delete[] buffer;
}

void dn::AdminNode::setReceivePort(uint16_t port)
{
	if (port == receivePort_ || simState_ == SimRun || simState_ == SimPause)
		return;
	stopListen();
	receivePort_ = port;
	if(subSocket_)
	{
		zmq_close(subSocket_);
	}
	subSocket_ = zmq_socket(socketContext_, ZMQ_SUB);
	std::stringstream ss;
	ss << "tcp://*:" << receivePort_;
	assert(zmq_bind(subSocket_, ss.str().c_str()) == 0);
	startListen();
}

void dn::AdminNode::setSendPort(uint16_t port)
{
	if (port == sendPort_ || simState_ == SimRun || simState_ == SimPause)
		return;
	stopListen();
	sendPort_ = port;
	if(pubSocket_)
	{
		zmq_close(pubSocket_);
	}
	pubSocket_ = zmq_socket(socketContext_, ZMQ_PUB);
	std::stringstream ss;
	ss << "tcp://*:" << sendPort_;
	assert(zmq_bind(pubSocket_, ss.str().c_str()) == 0);
	startListen();
}

void dn::AdminNode::setBufferSize(size_t bytes)
{
	if(bytes == bufferSize_)
		return;
	stopListen();
	delete[] buffer;
	buffer = new char[bytes];
	bufferSize_ = bytes;
	startListen();
}

void dn::AdminNode::startListen()
{
	if(listenThread_ != nullptr)
	{
		stopListen();
	}
	listenThread_ = new std::thread([this] { listen(); });
	listenThread_->detach();
}

void dn::AdminNode::stopListen()
{
	listenStop_ = true;
	while(!listenStopped_)
	{
		std::this_thread::yield();
	}
	delete listenThread_;
	listenThread_ = nullptr;
	listenStop_ = false;
}

void dn::AdminNode::listen()
{
	listenStopped_ = false;
	zmq_pollitem_t pollitem{ subSocket_,0,ZMQ_POLLIN };
	while(!listenStop_)
	{
		auto item = zmq_poll(&pollitem, 1, 0);
		if(item >= 0)
		{
			// TODO
		}
	}
	listenStopped_ = true;
}
