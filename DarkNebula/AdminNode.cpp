#include "AdminNode.h"

#include <cassert>
#include <functional>
#include <iostream>
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
	inBuffer_(nullptr),
	outBuffer_(nullptr),
	simSteps_(0),
	curTime_(0),
	simTime_(10),
	simFree_(false),
	simReplay_(false),
	stepTime_(10),
	curStepTime_(0.0),
	simSpeed_(1.0)
{
	setBufferSize(1 * 1024 * 1024);
	setReceivePort(receivePort);
	setSendPort(sendPort);
	timer_.setCallback([this]() {timerEvent(); });
	timer_.setInterval(1);
}

dn::AdminNode::~AdminNode()
{
	stopListen();
	zmq_close(pubSocket_);
	zmq_close(subSocket_);
	zmq_ctx_destroy(socketContext_);
	delete[] inBuffer_;
	delete[] outBuffer_;
}

void dn::AdminNode::setReceivePort(uint16_t port)
{
	if (simState_ != SimNop && (port == receivePort_ || simState_ == SimRun || simState_ == SimPause))
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
	zmq_setsockopt(subSocket_, ZMQ_SUBSCRIBE, REPLY_TOPIC, strlen(REPLY_TOPIC));
	startListen();
}

uint16_t dn::AdminNode::getReceivePort() const
{
	return receivePort_;
}

void dn::AdminNode::setSendPort(uint16_t port)
{
	if (simState_ != SimNop && (port == receivePort_ || simState_ == SimRun || simState_ == SimPause))
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

uint16_t dn::AdminNode::getSendPort() const
{
	return sendPort_;
}

dn::SimState dn::AdminNode::getSimState() const
{
	return simState_;
}

bool dn::AdminNode::isFreeSim() const
{
	return simFree_;
}

bool dn::AdminNode::isReplaySim() const
{
	return simReplay_;
}

size_t dn::AdminNode::getNodeCount() const
{
	return nodeList_.size();
}

std::vector<dn::NodeInfo> dn::AdminNode::getNodeList() const
{
	return nodeList_;
}

size_t dn::AdminNode::getChunkCount() const
{
	return chunkList_.size();
}

std::vector<dn::ChunkInfo> dn::AdminNode::getChunkList() const
{
	return chunkList_;
}

void dn::AdminNode::setBufferSize(size_t bytes)
{
	if(bytes == bufferSize_)
		return;
	assert(bytes >= sizeof CommandHeader);
	stopListen();
	if(inBuffer_)
	{
		zmq_msg_close(inBuffer_);
		delete inBuffer_;
	}
	inBuffer_ = new zmq_msg_t;
	zmq_msg_init_size(inBuffer_, bytes);
	delete[] outBuffer_;
	outBuffer_ = new char[bytes];
	memset(outBuffer_, 0, bytes);
	bufferSize_ = bytes;
	startListen();
}

size_t dn::AdminNode::getBufferSize() const
{
	return bufferSize_;
}

double dn::AdminNode::getCurTime() const
{
	return curTime_;
}

unsigned dn::AdminNode::getCurSteps() const
{
	return simSteps_;
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

void dn::AdminNode::setSimSpeed(double speed)
{
	if(speed <= 0.0)
		return;
	simSpeed_ = speed;
}

void dn::AdminNode::initSim()
{
	if(simState_ != SimNop && simState_ != SimInit && simState_ != SimStop)
		return;
	simState_ = SimInit;
	simReplay_ = true;
	// 把各种信息打到json里发布
	json info;
	auto nodes = json::array();
	auto chunks = json::array();
	for(auto i = 0ull;i < nodeList_.size();++i)
	{
		json obj;
		obj["name"] = nodeList_[i].name;
		obj["id"] = i;
		nodes.push_back(obj);
		// 所有节点都是重放状态才是重放
		simReplay_ = simReplay_ && nodeList_[i].replay;
	}
	for(auto i = 0ull;i < chunkList_.size();++i)
	{
		json obj;
		obj["name"] = chunkList_[i].first;
		obj["path"] = chunkList_[i].second;
		obj["id"] = i;
		chunks.push_back(obj);
	}
	info["nodes"] = nodes;
	info["chunks"] = chunks;
	auto jsonStr = info.dump();
	sendCommand(ALL_NODE, COMMAND_INIT,jsonStr.size(),jsonStr.c_str());
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
		simSteps_ = 0;
		curTime_ = 0.0;
		curStepTime_ = 0.0;
		sendCommand(ALL_NODE, COMMAND_START,sizeof curTime_ + sizeof simSteps_, &simSteps_);
		simState_ = SimRun;
		timer_.start();
	}
}

void dn::AdminNode::pauseSim()
{
	if(simState_ != SimRun)
		return;
	sendCommand(ALL_NODE, COMMAND_PAUSE, sizeof curTime_ + sizeof simSteps_, &simSteps_);
	simState_ = SimPause;
}

void dn::AdminNode::stopSim()
{
	if (simState_ != SimRun && simState_ != SimStep && simState_ != SimPause)
	{
		return;
	}
	sendCommand(ALL_NODE, COMMAND_STOP, sizeof curTime_ + sizeof simSteps_, &simSteps_);
	simState_ = SimStop;
}

void dn::AdminNode::stepForward()
{
	if(simState_ != SimPause && simState_ != SimStep)
		return;
	stepAdvance();
	simState_ = SimStep;
}

void dn::AdminNode::stepBackward()
{
	if (simState_ != SimPause && simState_ != SimStep)
		return;
	if (!simReplay_)	// 只有重放模式可以
		return;
	if (simSteps_ <= 0)
		return;
	--simSteps_;
	curTime_ = static_cast<double>(simSteps_) * stepTime_ / 1000.0;
	sendCommand(ALL_NODE, COMMAND_STEP_BACKWARD, sizeof curTime_ + sizeof simSteps_, &simSteps_);
	simState_ = SimStep;
}

void dn::AdminNode::onInitOver(AdminCallback callback)
{
	initCallback_ = std::move(callback);
}

void dn::AdminNode::onRegister(AdminCallback callback)
{
	registerCallback_ = std::move(callback);
}

void dn::AdminNode::onAdvance(AdminCallback callback)
{
	advanceCallback_ = std::move(callback);
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
	std::unique_lock<std::mutex> lock(listenMutex_);
	delete listenThread_;
	listenThread_ = nullptr;
	listenStop_ = false;
}

void dn::AdminNode::listen()
{
	std::unique_lock<std::mutex> lock(listenMutex_);
	if (!subSocket_)
	{
		return;
	}
	zmq_pollitem_t pollitem{ subSocket_,0,ZMQ_POLLIN };
	while(!listenStop_)
	{
		auto item = zmq_poll(&pollitem, 1, 1);
		if(item > 0)
		{
			auto len = zmq_msg_recv(inBuffer_, subSocket_, ZMQ_DONTWAIT);
			if(len <= 0)
				continue;
			// 第一条应该是topic
			if(inString() != REPLY_TOPIC)
				continue;
			// 再接一次是数据
			len = zmq_msg_recv(inBuffer_, subSocket_, ZMQ_DONTWAIT);
			if(len < sizeof CommandHeader)
				continue;
			auto* header = reinterpret_cast<CommandHeader*>(inBufferData());
			switch (header->code)
			{
			case COMMAND_REG:
				nodeReg(inBufferData(), len);
				break;
			case COMMAND_INIT:
				nodeInit(inBufferData(), len);
				break;
			case COMMAND_STEP_FORWARD:
			case COMMAND_STEP_BACKWARD:
				nodeStep(inBufferData(), len);
				break;
			// TODO
			default:
				break;
			}
		}
	}
}

void dn::AdminNode::sendMsg(void* buffer, size_t len)
{
	zmq_send(pubSocket_, COMMAND_TOPIC, strlen(COMMAND_TOPIC), ZMQ_SNDMORE);
	zmq_send(pubSocket_, buffer, len, 0);
}

void dn::AdminNode::sendCommand(int id, int code, size_t size, void const* data)
{
	memset(outBuffer_, 0, bufferSize_);
	auto* header = reinterpret_cast<CommandHeader*>(outBuffer_);
	header->ID = id;
	header->code = code;
	header->size = size;
	if(size && data)
	{
		if (size > bufferSize_ - sizeof CommandHeader)
			setBufferSize(size * 2);
		memcpy_s(outBuffer_ + sizeof CommandHeader, size, data, size);
	}
	sendMsg(outBuffer_, size + sizeof CommandHeader);
}

void dn::AdminNode::stepAdvance()
{
	++simSteps_;
	curTime_ = static_cast<double>(simSteps_) * stepTime_ / 1000.0;
	sendCommand(ALL_NODE, COMMAND_STEP_FORWARD, sizeof curTime_ + sizeof simSteps_, &simSteps_);
}

bool dn::AdminNode::checkInit()
{
	for (const auto& it : nodeList_)
		if (!it.init)
			return false;
	return true;
}

bool dn::AdminNode::checkStep()
{
	for (const auto& it : nodeList_)
		if (it.steps != simSteps_ && !it.slow)
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
		nodeList_.emplace_back();
		nodeList_[id].name = name;
	}
	else
	{
		id = nodeMap_[name];
	}
	auto chunks = obj["chunks"];
	nodeList_[id].ip = obj["ip"].get<std::string>();
	nodeList_[id].slow = obj["slow"].get<bool>();
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
	// 回报注册通过
	sendCommand(id, COMMAND_REG, name.size(), name.c_str());
	if (registerCallback_)
		registerCallback_(id);
}

void dn::AdminNode::nodeInit(char* buffer, int len)
{
	auto* header = reinterpret_cast<CommandHeader*>(buffer);
	bool ok = false;
	if(header->size)
	{
		ok = *reinterpret_cast<bool*>(buffer + sizeof CommandHeader + 1);
	}
	if(header->ID < nodeList_.size())
	{
		nodeList_[header->ID].init = ok;
		if (initCallback_)
		{
			initCallback_(header->ID);
			if(checkInit())
				initCallback_(ALL_NODE);
		}
	}
}

void dn::AdminNode::nodeStep(char* buffer, int len)
{
	auto* header = reinterpret_cast<CommandHeader*>(buffer);
	int step = 0;
	if (header->size)
	{
		step = *reinterpret_cast<int*>(buffer + sizeof CommandHeader + 1);
	}
	if (header->ID < nodeList_.size())
	{
		nodeList_[header->ID].steps = step;
		if (advanceCallback_)
		{
			advanceCallback_(header->ID);
			if (checkStep())
				advanceCallback_(ALL_NODE);
		}
	}
}

void dn::AdminNode::timerEvent()
{
	curStepTime_ += simSpeed_;
	// 达到仿真时间
	if(curStepTime_ >= stepTime_)
	{
		if (!checkStep())
			return;
		curStepTime_ = 0.0;
		stepAdvance();
	}
}

char* dn::AdminNode::inBufferData() const
{
	if (!inBuffer_)
		return nullptr;
	return static_cast<char*>(zmq_msg_data(inBuffer_));
}

std::string dn::AdminNode::inString() const
{
	if (!inBuffer_)
		return "";
	return std::string(inBufferData(), zmq_msg_size(inBuffer_));
}
