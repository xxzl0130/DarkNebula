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
	inBuffer(nullptr),
	outBuffer(nullptr)
{
	setBufferSize(1 * 1024 * 1024);
	setReceivePort(receivePort);
	setSendPort(sendPort);
}

dn::AdminNode::~AdminNode()
{
	stopListen();
	zmq_close(pubSocket_);
	zmq_close(subSocket_);
	zmq_ctx_destroy(socketContext_);
	delete[] inBuffer;
	delete[] outBuffer;
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
	assert(bytes >= sizeof CommandHeader);
	stopListen();
	delete[] inBuffer;
	inBuffer = new char[bytes];
	memset(inBuffer, 0, bytes);
	delete[] outBuffer;
	outBuffer = new char[bytes];
	memset(outBuffer, 0, bytes);
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

void dn::AdminNode::sendMsg(void* buffer, size_t len)
{
	zmq_send(pubSocket_, COMMAND_TOPIC, strlen(COMMAND_TOPIC), 0);
	zmq_send(pubSocket_, buffer, len, 0);
}

void dn::AdminNode::sendCommand(int id, int code, size_t size, void const* data)
{
	memset(outBuffer, 0, bufferSize_);
	auto* header = reinterpret_cast<CommandHeader*>(outBuffer);
	header->ID = id;
	header->code = code;
	header->size = size;
	if(size && data)
	{
		assert(size <= bufferSize_ - sizeof CommandHeader);
		memcpy_s(outBuffer + sizeof CommandHeader, size, data, size);
	}
	sendMsg(outBuffer, size + sizeof CommandHeader);
}
