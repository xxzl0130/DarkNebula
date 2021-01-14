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
	slowNodeCount_(0),
	stepNodeCount_(0),
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
	stopWorking();
}

void dn::AdminNode::setReceivePort(uint16_t port)
{
	if (simState_ != SimNop && (port == receivePort_ || simState_ == SimRun || simState_ == SimPause))
		return;
	stopWorking();
	receivePort_ = port;
	if(subSocket_)
	{
		zmq_close(subSocket_);
	}
	subSocket_ = zmq_socket(socketContext_, ZMQ_SUB);
	int linger = 0;
	zmq_setsockopt(pubSocket_, ZMQ_LINGER, &linger, sizeof linger);
	std::stringstream ss;
	ss << "tcp://*:" << receivePort_;
	assert(zmq_bind(subSocket_, ss.str().c_str()) == 0);
	zmq_setsockopt(subSocket_, ZMQ_SUBSCRIBE, REPLY_TOPIC, strlen(REPLY_TOPIC));
	startWorking();
}

uint16_t dn::AdminNode::getReceivePort() const
{
	return receivePort_;
}

void dn::AdminNode::setSendPort(uint16_t port)
{
	if (simState_ != SimNop && (port == receivePort_ || simState_ == SimRun || simState_ == SimPause))
		return;
	stopWorking();
	sendPort_ = port;
	if(pubSocket_)
	{
		zmq_close(pubSocket_);
	}
	pubSocket_ = zmq_socket(socketContext_, ZMQ_PUB);
	int linger = 0;
	zmq_setsockopt(pubSocket_, ZMQ_LINGER, &linger, sizeof linger);
	std::stringstream ss;
	ss << "tcp://*:" << sendPort_;
	assert(zmq_bind(pubSocket_, ss.str().c_str()) == 0);
	startWorking();
}

uint16_t dn::AdminNode::getSendPort() const
{
	return sendPort_;
}

size_t dn::AdminNode::getNodeCount() const
{
	return nodeList_.size();
}

const std::vector<dn::NodeInfo>& dn::AdminNode::getNodeList() const
{
	return nodeList_;
}

size_t dn::AdminNode::getChunkCount() const
{
	return chunkList_.size();
}

const std::vector<dn::ChunkInfo>& dn::AdminNode::getChunkList() const
{
	return chunkList_;
}

void dn::AdminNode::setBufferSize(size_t bytes)
{
	stopWorking();
	Node::setBufferSize(bytes);
	startWorking();
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

void dn::AdminNode::setSimEndTime(double time)
{
	if (simState_ > SimStop)
		return;
	simTime_ = time;
}

double dn::AdminNode::getSimEndTime() const
{
	return simTime_;
}

void dn::AdminNode::setStepTime(unsigned ms)
{
	if (simState_ > SimStop)
		return;
	stepTime_ = ms;
}

void dn::AdminNode::setRecord(bool enable, const std::string& name)
{
	if (simState_ > SimStop)
		return;
	if(enable)
	{
		replayState_ = Recording;
		recordName_ = name;
	}
	else
	{
		replayState_ = ReplayNop;
	}
}

void dn::AdminNode::setReplay(bool enable, const std::string& name)
{
	if (simState_ > SimStop)
		return;
	if (enable)
	{
		replayState_ = Replaying;
		recordName_ = name;
	}
	else
	{
		replayState_ = ReplayNop;
	}
}

void dn::AdminNode::setSimSpeed(double speed)
{
	if(speed <= 0.0)
		return;
	simSpeed_ = speed;
}

double dn::AdminNode::getSimSpeed() const
{
	return simSpeed_;
}

void dn::AdminNode::speedUp()
{
	setSimSpeed(simSpeed_ + 0.05);
}

void dn::AdminNode::speedDown()
{
	setSimSpeed(simSpeed_ - 0.05);
}

void dn::AdminNode::initSim()
{
	if(simState_ != SimNop && simState_ != SimInit && simState_ != SimStop)
		return;
	simState_ = SimInit;
	curTime_ = 0.0;
	simSteps_ = 0;
	slowNodeCount_ = 0;
	stepNodeCount_ = 0;
	// 把各种信息打到json里发布
	json info;
	json nodes;
	json chunks;
	for(auto i = 0ull;i < nodeList_.size();++i)
	{
		json obj;
		obj["name"] = nodeList_[i].name;
		obj["id"] = i;
		nodes[nodeList_[i].name] = obj;
		nodeList_[i].init = false;
		if (nodeList_[i].slow)
			++slowNodeCount_;
	}
	for(auto i = 0ull;i < chunkList_.size();++i)
	{
		json obj;
		obj["name"] = chunkList_[i].first;
		obj["path"] = chunkList_[i].second;
		obj["id"] = i;
		chunks[chunkList_[i].first] = obj;
	}
	info["nodes"] = nodes;
	info["chunks"] = chunks;
	info["simTime"] = simTime_;
	info["free"] = simFree_;
	info["replay"] = replayState_;
	info["step"] = stepTime_;
	info["record"] = recordName_;
	auto jsonStr = info.dump();
	sendCommand(ALL_NODE, COMMAND_INIT,jsonStr.size(),jsonStr.c_str());
}

void dn::AdminNode::startSim()
{
	// 初始化、暂停、单步才能启动
	if(simState_ != SimPause && simState_ != SimInit && simState_ != SimStep)
		return;
	if (checkInit())
	{
		if(simState_ != SimPause)
		{
			// 从暂停恢复的时候时间不清零
			curTime_ = 0.0;
			simSteps_ = 0;
		}
		curStepTime_ = 0.0;
		stepNodeCount_ = nodeList_.size() - slowNodeCount_;
		sendCommand(ALL_NODE, COMMAND_START,sizeof curTime_, &curTime_);
		simState_ = SimRun;
		if(stepTime_ > 1)
			curStepTime_ = stepTime_ - 1;
		timer_.start();
	}
}

void dn::AdminNode::pauseSim()
{
	if(simState_ != SimRun)
		return;
	sendCommand(ALL_NODE, COMMAND_PAUSE, sizeof curTime_, &curTime_);
	simState_ = SimPause;
	timer_.stop();
}

void dn::AdminNode::stopSim()
{
	if (simState_ != SimRun && simState_ != SimStep && simState_ != SimPause)
	{
		return;
	}
	sendCommand(ALL_NODE, COMMAND_STOP, sizeof curTime_, &curTime_);
	simState_ = SimStop;
	timer_.stop();
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
	if (replayState_ != Replaying)	// 只有重放模式可以
		return;
	if (simSteps_ <= 0)
		return;
	--simSteps_;
	stepNodeCount_ = 0;
	curTime_ = static_cast<double>(simSteps_) * stepTime_ / 1000.0;
	sendCommand(ALL_NODE, COMMAND_STEP_BACKWARD, sizeof curTime_, &curTime_);
	simState_ = SimStep;
}

void dn::AdminNode::searchNode()
{
	if (simState_ > SimStop)
		return;
	sendCommand(ALL_NODE, COMMAND_REG);
}

void dn::AdminNode::setInitOverCallback(AdminCallback callback)
{
	initCallback_ = std::move(callback);
}

void dn::AdminNode::setRegisterCallback(AdminCallback callback)
{
	registerCallback_ = std::move(callback);
}

void dn::AdminNode::setAdvanceCallback(AdminCallback callback)
{
	advanceCallback_ = std::move(callback);
}

void dn::AdminNode::working()
{
	std::unique_lock<std::mutex> lock(workMutex_);
	if (!subSocket_)
	{
		return;
	}
	zmq_pollitem_t pollitem{ subSocket_,0,ZMQ_POLLIN };
	while(!workStop_)
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
	zmq_send(pubSocket_, COMMAND_TOPIC, COMMAND_TOPIC_LEN, ZMQ_SNDMORE);
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
	stepNodeCount_ = 0;
	curTime_ = static_cast<double>(simSteps_) * stepTime_ / 1000.0;
	sendCommand(ALL_NODE, COMMAND_STEP_FORWARD, sizeof curTime_, &curTime_);
	if (!simFree_ && curTime_ >= simTime_)
		stopSim();
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
	return stepNodeCount_ + slowNodeCount_ >= nodeList_.size();
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
	uint16_t code = ERR_NOP;
	if(header->size)
	{
		code = *reinterpret_cast<uint16_t*>(inData());
	}
	if(header->ID < nodeList_.size())
	{
		nodeList_[header->ID].init = code == ERR_NOP;
		nodeList_[header->ID].errorCode = code;
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
		step = *reinterpret_cast<int*>(inData());
	}
	if (header->ID >= 0 && header->ID < nodeList_.size() && nodeList_[header->ID].steps != step)
	{
		stepNodeCount_++;
		nodeList_[header->ID].steps = step;
		if (advanceCallback_)
		{
			advanceCallback_(header->ID);
			// 所有节点都仿真了
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
