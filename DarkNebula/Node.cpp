#include "Node.h"

#include <cassert>
#include <zmq.h>

dn::Node::Node():
	pubSocket_(nullptr),
	subSocket_(nullptr),
	socketContext_(zmq_ctx_new()),
	workThread_(nullptr),
	workStop_(false),
	outBuffer_(nullptr),
	inBuffer_(nullptr),
	bufferSize_(0),
	simTime_(10),
	simFree_(false),
	stepTime_(10),
	simState_(SimNop),
	replayState_(ReplayNop)
{
	setBufferSize(1 * 1024 * 1024);
}

dn::Node::~Node()
{
	stopWorking();
	if (pubSocket_)
	{
		zmq_close(pubSocket_);
	}
	if (subSocket_)
	{
		zmq_close(subSocket_);
	}
	delete[] outBuffer_;
	if (inBuffer_)
		zmq_msg_close(inBuffer_);
	delete inBuffer_;
	zmq_ctx_term(socketContext_);
}

void dn::Node::setBufferSize(size_t bytes)
{
	if (bytes <= 2 * sizeof CommandHeader)
		bytes = 2 * sizeof CommandHeader;
	if (bytes == bufferSize_)
		return;
	bufferSize_ = bytes;
	if (inBuffer_)
	{
		zmq_msg_close(inBuffer_);
		delete inBuffer_;
	}
	inBuffer_ = new struct zmq_msg_t;
	zmq_msg_init_size(inBuffer_, bytes);
	delete[] outBuffer_;
	outBuffer_ = new char[bytes];
}

size_t dn::Node::getBufferSize()
{
	return bufferSize_;
}

uint32_t dn::Node::getSimStep() const
{
	return simSteps_;
}

double dn::Node::getSimTime() const
{
	return curTime_;
}

dn::Node::SimState dn::Node::getSimState() const
{
	return simState_;
}

bool dn::Node::isFreeSim() const
{
	return simFree_;
}

bool dn::Node::isReplaySim() const
{
	return replayState_ == Replaying;
}

double dn::Node::getCurTime() const
{
	return curTime_;
}

unsigned dn::Node::getCurSteps() const
{
	return simSteps_;
}

std::string dn::Node::getRecordName() const
{
	return recordName_;
}

void dn::Node::startWorking()
{
	if (workThread_)
	{
		stopWorking();
		delete workThread_;
	}
	workThread_ = new std::thread([this]() {working(); });
	workThread_->detach();
}

void dn::Node::stopWorking()
{
	workStop_ = true;
	// ¼ÓËøµÈ´ý
	std::unique_lock<std::mutex> lock(workMutex_);
	delete workThread_;
	workThread_ = nullptr;
	workStop_ = false;
}

char* dn::Node::inBufferData() const
{
	assert(inBuffer_ != nullptr);
	return static_cast<char*>(zmq_msg_data(inBuffer_));
}

std::string dn::Node::inString() const
{
	assert(inBuffer_ != nullptr);
	return std::string(inBufferData(), zmq_msg_size(inBuffer_));
}

dn::CommandHeader* dn::Node::inHeader() const
{
	assert(inBuffer_ != nullptr);
	return static_cast<CommandHeader*>(zmq_msg_data(inBuffer_));
}

char* dn::Node::inData() const
{
	return inBufferData() + sizeof CommandHeader;
}

int dn::Node::recvMsg(void* socket)
{
	return zmq_msg_recv(inBuffer_, socket, ZMQ_DONTWAIT);
}
