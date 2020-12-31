#include "AdminNode.h"

#include <cassert>
#include <functional>
#include <zmq.h>
#include <sstream>
#include <thread>
#include <nlohmann/json.hpp>
using nlohmann::json;

dn::AdminNode::AdminNode(uint16_t receivePort, uint16_t sendPort):
	receivePort_(receivePort),
	sendPort_(sendPort),
	simState_(SimNop),
	pubSocket_(nullptr),
	subSocket_(nullptr),
	socketContext_(zmq_ctx_new()),
	listenThread_(nullptr),
	listenStop_(false),
	listenStopped_(false),
	inBuffer(nullptr),
	outBuffer(nullptr),
	curTime_(0),
	simTime_(10),
	simSteps_(0),
	simFree_(false),
	simReplay_(false),
	stepTime_(10)
{
	setBufferSize(1 * 1024 * 1024);
	setReceivePort(receivePort);
	setSendPort(sendPort);
	timer_.setCallback([this]() {timerEvent(); });
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

void dn::AdminNode::clear()
{
	if(simState_ != SimNop && simState_ != SimInit && simState_ != SimStop)
		return;
	nodeList_.clear();
	chunkList_.clear();
	nodeMap_.clear();
	chunkMap_.clear();
}

void dn::AdminNode::setFreeSim(bool en)
{
	if (simState_ > SimStop)
		return;
	simFree_ = en;
}

void dn::AdminNode::setSimTime(double time)
{
	if (simState_ > SimStop)
		return;
	simTime_ = time;
}

void dn::AdminNode::setStepTime(unsigned ms)
{
	if (simState_ > SimStop)
		return;
	stepTime_ = ms;
	timer_.setInterval(ms);
}

void dn::AdminNode::setRecord(bool enable, char const* name)
{
	if (simState_ > SimStop)
		return;
	//TODO
}

void dn::AdminNode::setReply(int node, bool enable, char const* name)
{
	if (simState_ > SimStop)
		return;
	//TODO
}

void dn::AdminNode::initSim()
{
	if(simState_ != SimNop && simState_ != SimInit && simState_ != SimStop)
		return;
	sendCommand(ALL_NODE, COMMAND_INIT);
	simState_ = SimInit;
	simReplay_ = true;
	// 所有节点都是重放状态才是重放
	for(const auto& it : nodeList_)
	{
		simReplay_ = simReplay_ && it.replay;
	}
	curTime_ = 0.0;
	simSteps_ = 0;
}

void dn::AdminNode::startSim()
{
	// 初始化、暂停、单步才能启动
	if(simState_ != SimPause && simState_ != SimInit && simState_ != SimStep)
		return;
	if (checkInit())
	{
		sendCommand(ALL_NODE, COMMAND_START);
		simState_ = SimRun;
	}
}

void dn::AdminNode::pauseSim()
{
	if(simState_ != SimRun)
		return;
	sendCommand(ALL_NODE, COMMAND_PAUSE);
	simState_ = SimPause;
}

void dn::AdminNode::stopSim()
{
	if (simState_ != SimRun && simState_ != SimStep && simState_ != SimPause)
	{
		return;
	}
	sendCommand(ALL_NODE, COMMAND_STOP);
	simState_ = SimStop;
}

void dn::AdminNode::stepForward()
{
	if(simState_ != SimPause && simState_ != SimStep)
		return;
	sendCommand(ALL_NODE, COMMAND_STEP_FORWARD);
	simState_ = SimStep;
}

void dn::AdminNode::stepBackward()
{
	if (simState_ != SimPause && simState_ != SimStep)
		return;
	if(!simReplay_)	// 只有重放模式可以
		return;
	sendCommand(ALL_NODE, COMMAND_STEP_BACKWARD);
	simState_ = SimStep;
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
			auto len = zmq_recv(subSocket_, inBuffer, bufferSize_, ZMQ_DONTWAIT);
			if(len <= 0)
				continue;
			// 第一条应该是topic
			if(strcmp(inBuffer,REPLY_TOPIC) != 0)
				continue;
			// 再接一次是数据
			len = zmq_recv(subSocket_, inBuffer, bufferSize_, ZMQ_DONTWAIT);
			if(len < sizeof CommandHeader)
				continue;
			auto* header = reinterpret_cast<CommandHeader*>(inBuffer);
			switch (header->code)
			{
			case COMMAND_REG:
				nodeReg(inBuffer, len);
				break;
			case COMMAND_INIT:
				nodeInit(inBuffer, len);
				break;
			case COMMAND_STEP_FORWARD:
			case COMMAND_STEP_BACKWARD:
				nodeStep(inBuffer, len);
				break;
			// TODO
			default:
				break;
			}
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

bool dn::AdminNode::checkInit()
{
	for (const auto& it : nodeList_)
		if (!it.init)
			return false;
	return true;
}

void dn::AdminNode::nodeReg(char* buffer, int len)
{
	auto* header = reinterpret_cast<CommandHeader*>(buffer);
	auto obj = json::parse(std::string(buffer + sizeof CommandHeader, header->size));
	auto name = obj["name"].get<std::string>();
	int id;
	// 查找or新建
	if(nodeMap_.count(name) < 1)
	{
		nodeMap_.insert(std::make_pair(name, id = nodeMap_.size()));
		nodeList_.emplace_back(name);
	}
	else
	{
		id = nodeMap_[name];
	}
	auto chunks = obj["chunks"];
	nodeList_[id].ip = obj["ip"].get<std::string>();
	nodeList_[id].chunks.clear();
	if(chunks.is_array())
	{
		for(const auto& it : chunks)
		{
			auto chunk = it["name"].get<std::string>();
			int chunkID;
			// 查找or新建
			if(chunkMap_.count(chunk) < 1)
			{
				chunkMap_.insert(std::make_pair(chunk, chunkID = chunkMap_.size()));
				chunkList_.emplace_back(chunk, "");
			}
			else
			{
				chunkID = chunkMap_[chunk];
			}
			auto own = it["own"].get<bool>();
			if(own)
			{
				auto port = it["port"].get<int>();
				std::stringstream ss;
				ss << "tcp://" << nodeList_[id].ip << ":" << port;
				chunkList_[chunkID].second = ss.str();
			}
			nodeList_[id].chunks.emplace_back(chunkID, own);
		}
	}
}

void dn::AdminNode::nodeInit(char* buffer, int len)
{
}

void dn::AdminNode::nodeStep(char* buffer, int len)
{
}

void dn::AdminNode::timerEvent()
{
	// TODO
}
