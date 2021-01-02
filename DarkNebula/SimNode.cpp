#include "SimNode.h"
#include <zmq.h>
#include <memory>
#include <sstream>
#include <nlohmann/json.hpp>
using nlohmann::json;

dn::SimNode::SimNode(const std::string& nodeName, const std::string& nodeIP, uint16_t chunkPort, bool slowNode,
	const std::string& adminIP, uint16_t adminRecvPort, uint16_t adminSendPort):
	simSteps_(0),
	simTime_(0),
	slowNode_(false),
	running_(false),
	comStop_(false),
	workThread_(nullptr),
	socketContext_(zmq_ctx_new()),
	pubSocket_(nullptr),
	subSocket_(nullptr)
{
	setNodeName(nodeName);
	setNodeIP(nodeIP);
	setChunkPort(chunkPort);
	setAdminIP(adminIP);
	setAdminRecvPort(adminRecvPort);
	setAdminSendPort(adminSendPort);
}

dn::SimNode::~SimNode()
{
	stop();
	zmq_close(pubSocket_);
	zmq_close(subSocket_);
	zmq_ctx_destroy(socketContext_);
}

void dn::SimNode::setNodeName(const std::string& name)
{
	if(running_)
		return;
	nodeName_ = name;
}

void dn::SimNode::setNodeIP(const std::string& IP)
{
	if (running_)
		return;
	nodeIP_ = IP;
}

void dn::SimNode::setChunkPort(uint16_t port)
{
	if (running_)
		return;
	chunkPort_ = port;
}

void dn::SimNode::setSlowNode(bool slow)
{
	if (running_)
		return;
	slowNode_ = slow;
}

void dn::SimNode::setAdminIP(const std::string& IP)
{
	if (running_)
		return;
	adminIP_ = IP;
}

void dn::SimNode::setAdminRecvPort(uint16_t port)
{
	if (running_)
		return;
	adminRecvPort_ = port;
}

void dn::SimNode::setAdminSendPort(uint16_t port)
{
	if (running_)
		return;
	adminSendPort_ = port;
}

bool dn::SimNode::regIn()
{
	if(running_)
		return true;
	initSocket();
	if (!sendReg())
		return false;
	if(workThread_)
	{
		stop();
		delete workThread_;
	}
	running_ = true;
	workThread_ = new std::thread([this]() {communicate(); });
	workThread_->detach();
	return true;
}

void dn::SimNode::setInitCallback(SimEventCallback callback)
{
	initCallback_ = std::move(callback);
}

void dn::SimNode::setStartCallback(SimEventCallback callback)
{
	startCallback_ = std::move(callback);
}

void dn::SimNode::setPauseCallback(SimEventCallback callback)
{
	pauseCallback_ = std::move(callback);
}

void dn::SimNode::setStopCallback(SimEventCallback callback)
{
	stopCallback_ = std::move(callback);
}

void dn::SimNode::setSimStepCallback(SimStepCallback callback)
{
	simStepCallback_ = std::move(callback);
}

void dn::SimNode::setReplayStepCallback(SimStepCallback callback)
{
	replayStepCallback_ = std::move(callback);
}

uint32_t dn::SimNode::getSimStep() const
{
	return simSteps_;
}

double dn::SimNode::getSimTime() const
{
	return simTime_;
}

void dn::SimNode::working()
{
	std::unique_lock<std::mutex> lock(workMutex_);

	// 初始化监听列表
	std::unique_ptr<zmq_pollitem_t[]> pollItems(new zmq_pollitem_t[chunks_.size() + 1]);
	pollItems[0].socket = subSocket_;
	pollItems[0].fd = 0;
	pollItems[0].events = ZMQ_POLLIN;
	for (auto i = 0;i < chunks_.size();++i)
	{
		auto& chunk = chunks_[i];
		if (chunk.socket)
			zmq_close(chunk.socket);
		if(chunk.own) // pub
		{
			chunk.socket = zmq_socket(socketContext_, ZMQ_PUB);
			std::stringstream ss;
			ss << "tcp://" << nodeIP_ << ":" << chunk.port;
			zmq_bind(chunk.socket, ss.str().c_str());
		}
		else
		{
			chunk.socket = zmq_socket(socketContext_, ZMQ_SUB);
		}
		pollItems[i + 1].socket = chunk.socket;
		pollItems[i + 1].fd = 0;
		pollItems[i + 1].events = ZMQ_POLLIN;
	}
	
}

void dn::SimNode::stop()
{
	comStop_ = true;
	// 加锁等待
	std::unique_lock<std::mutex> lock(workMutex_);
	delete workThread_;
	workThread_ = nullptr;
	comStop_ = false;
}

bool dn::SimNode::sendReg()
{
	zmq_pollitem_t pollitem{ subSocket_,0,ZMQ_POLLIN };
	json info;
	info["name"] = nodeName_;
	info["ip"] = nodeIP_;
	info["slow"] = slowNode_;
	auto chunksJson = json::array();
	auto port = chunkPort_;
	for(auto& it : chunks_)
	{
		json obj;
		obj["name"] = it.name;
		obj["own"] = it.own;
		if(it.own)
		{
			// 依次分配端口
			obj["port"] = it.port = port++;
		}
		chunksJson.push_back(obj);
	}
	info["chunks"] = chunksJson;

	const auto jsonStr = info.dump();
	const auto len = jsonStr.size() + sizeof CommandHeader;
	std::unique_ptr<char[]> buffer(new char[len]);
	auto* header = reinterpret_cast<CommandHeader*>(buffer.get());
	header->ID = -1;
	header->size = jsonStr.size();
	header->code = COMMAND_REG;
	memcpy_s(buffer.get() + sizeof CommandHeader, jsonStr.size(), jsonStr.c_str(), jsonStr.size());
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	for(auto i = 0;i < 5;++i)
	{
		zmq_send(pubSocket_, REPLY_TOPIC, strlen(REPLY_TOPIC), ZMQ_SNDMORE);
		zmq_send(pubSocket_, buffer.get(), len, 0);
		if(zmq_poll(&pollitem,1,100) > 0)
		{
			auto len = zmq_msg_recv(&msg, subSocket_, ZMQ_DONTWAIT);
			if(len < strlen(COMMAND_TOPIC))
				continue;
			if(strcmp(static_cast<char*>(zmq_msg_data(&msg)), COMMAND_TOPIC) != 0)
				continue;
			len = zmq_msg_recv(&msg, subSocket_, ZMQ_DONTWAIT);
			auto* header = static_cast<CommandHeader*>(zmq_msg_data(&msg));
			auto name = std::string(static_cast<char*>(zmq_msg_data(&msg)) + sizeof CommandHeader, header->size);
			if(name == nodeName_)
			{
				zmq_msg_close(&msg);
				return true;
			}
		}
	}
	zmq_msg_close(&msg);
	return false;
}

void dn::SimNode::initSocket()
{
	// 初始化subSocket_
	if (subSocket_)
		zmq_close(subSocket_);
	subSocket_ = zmq_socket(socketContext_, ZMQ_SUB);
	std::stringstream sub;
	sub << "tcp://" << adminIP_ << ":" << adminSendPort_;
	zmq_connect(subSocket_, sub.str().c_str());
	zmq_setsockopt(subSocket_, ZMQ_SUBSCRIBE, COMMAND_TOPIC, strlen(COMMAND_TOPIC));

	// 初始化pub
	if (pubSocket_)
		zmq_close(pubSocket_);
	pubSocket_ = zmq_socket(socketContext_, ZMQ_PUB);
	std::stringstream pub;
	pub << "tcp://" << adminIP_ << ":" << adminRecvPort_;
	zmq_connect(pubSocket_, pub.str().c_str());
}
