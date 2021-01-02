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
	workStop_(false),
	workThread_(nullptr),
	outBuffer_(nullptr),
	inBuffer_(nullptr),
	socketContext_(zmq_ctx_new()),
	pubSocket_(nullptr),
	subSocket_(nullptr),
	pollitems_(nullptr),
	pollCount_(0),
	id_(-1)
{
	setNodeName(nodeName);
	setNodeIP(nodeIP);
	setChunkPort(chunkPort);
	setAdminIP(adminIP);
	setAdminRecvPort(adminRecvPort);
	setAdminSendPort(adminSendPort);
	setBufferSize(1 * 1024 * 1024);
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

void dn::SimNode::setBufferSize(size_t bytes)
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

size_t dn::SimNode::getBufferSize()
{
	return bufferSize_;
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
	workThread_ = new std::thread([this]() {working(); });
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
	delete[] pollitems_;
	pollCount_ = 1;
	pollitems_ = new zmq_pollitem_t[chunks_.size() + 1];
	pollitems_[0].socket = subSocket_;
	pollitems_[0].fd = 0;
	pollitems_[0].events = ZMQ_POLLIN;

	// 一开始只能监听管理节点
	int itemCount = 1;
	while(!workStop_)
	{
		if(zmq_poll(pollitems_,itemCount,1) > 0)
		{
			// 先处理指令信息
			if(pollitems_[0].revents == ZMQ_POLLIN)
			{
				processAdminCommand();
			}
		}
	}

	delete[] pollitems_;
	pollitems_ = nullptr;
}

void dn::SimNode::stop()
{
	workStop_ = true;
	// 加锁等待
	std::unique_lock<std::mutex> lock(workMutex_);
	delete workThread_;
	workThread_ = nullptr;
	workStop_ = false;
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
	for(auto i = 0;i < 5;++i)
	{
		send2Admin(COMMAND_REG, jsonStr.data(), jsonStr.size());
		if(zmq_poll(&pollitem,1,100) > 0)
		{
			auto len = recvMsg(subSocket_);
			if(len < strlen(COMMAND_TOPIC))
				continue;
			if(inString() != COMMAND_TOPIC)
				continue;
			len = recvMsg(subSocket_);
			auto* header = inHeader();
			if(header->code != COMMAND_REG)
				continue;
			auto name = std::string(inData(), header->size);
			if(name == nodeName_)
			{
				id_ = header->ID;
				return true;
			}
		}
	}
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

void dn::SimNode::send2Admin(int code, const char* data, size_t size)
{
	assert(pubSocket_ != nullptr);
	auto* header = reinterpret_cast<CommandHeader*>(outBuffer_);
	header->ID = id_;
	header->code = code;
	header->size = size;
	if(data && size)
	{
		if(bufferSize_ < size + sizeof CommandHeader)
		{
			setBufferSize(size * 2);
		}
		memcpy_s(outBuffer_ + sizeof CommandHeader, size, data, size);
	}
	zmq_send(pubSocket_, REPLY_TOPIC, REPLY_TOPIC_LEN, ZMQ_SNDMORE);
	zmq_send(pubSocket_, outBuffer_, size + sizeof CommandHeader, 0);
}

char* dn::SimNode::inBufferData() const
{
	assert(inBuffer_ != nullptr);
	return static_cast<char*>(zmq_msg_data(inBuffer_));
}

std::string dn::SimNode::inString() const
{
	assert(inBuffer_ != nullptr);
	return std::string(inBufferData(), zmq_msg_size(inBuffer_));
}

dn::CommandHeader* dn::SimNode::inHeader() const
{
	assert(inBuffer_ != nullptr);
	return static_cast<CommandHeader*>(zmq_msg_data(inBuffer_));
}

char* dn::SimNode::inData() const
{
	return inBufferData() + sizeof CommandHeader;
}

int dn::SimNode::recvMsg(void* socket)
{
	return zmq_msg_recv(inBuffer_, socket, ZMQ_DONTWAIT);
}

void dn::SimNode::processAdminCommand()
{
	recvMsg(subSocket_);
	if(inString() != COMMAND_TOPIC)
		return;
	recvMsg(subSocket_);
	auto* header = inHeader();
	switch (header->code)
	{
	case COMMAND_INIT:
		init();
		if (initCallback_)
			initCallback_();
		break;
	case COMMAND_START:
		if (startCallback_)
			startCallback_();
		break;
	case COMMAND_STEP_FORWARD:
		
	case COMMAND_STEP_BACKWARD:
	case COMMAND_PAUSE:
	case COMMAND_STOP:
	case COMMAND_RECORD:
	case COMMAND_LOAD:
	case COMMAND_SIM:
	case COMMAND_REPLAY:
		break;
	default:
		break;
	}
}

void dn::SimNode::init()
{
	auto* header = inHeader();
	auto info = json::parse(std::string(inData(), header->size));
	const auto& nodes = info["nodes"];
	if(nodes.contains(nodeName_))
	{
		id_ = nodes[nodeName_]["id"].get<int>();
	}
	const auto& chunksInfo = info["chunks"];

	// 初始化数据块
	for(auto i = 0;i < chunks_.size();++i)
	{
		auto& chunk = chunks_[i];
		if(chunksInfo.contains(chunk.name))
		{
			const auto& obj = chunksInfo[chunk.name];
			chunk.id = obj["id"];
			if (chunk.socket)
				zmq_close(chunk.socket);
			if (chunk.own) // pub
			{
				chunk.socket = zmq_socket(socketContext_, ZMQ_PUB);
				std::stringstream ss;
				ss << "tcp://" << nodeIP_ << ":" << chunk.port;
				zmq_bind(chunk.socket, ss.str().c_str());
				chunk.init = true;
			}
			else
			{
				chunk.socket = zmq_socket(socketContext_, ZMQ_SUB);
				zmq_connect(chunk.socket,obj["path"].get<std::string>().c_str());
				zmq_setsockopt(chunk.socket, ZMQ_SUBSCRIBE, chunk.name.c_str(), chunk.name.size());
				chunk.init = false;
			}
			// 全部加入监听列表
			pollitems_[i + 1].socket = chunk.socket;
			pollitems_[i + 1].fd = 0;
			pollitems_[i + 1].events = ZMQ_POLLIN;
		}
	}
	pollCount_ = 1 + chunks_.size();
	while(true)
	{
		// 自己发布的挨个发布一遍 TODO:应该在所有人初始化完成前持续发送
		for(auto& it : chunks_)
		{
			if(it.own)
			{
				sendChunk(it);
			}
		}
		if(zmq_poll(pollitems_ + 1, chunks_.size(),100) > 0)
		{
			for(auto i = 0;i < chunks_.size();++i)
			{
				if(pollitems_[i + 1].revents & ZMQ_POLLIN)
				{
					recvMsg(pollitems_[i + 1].socket);
					if(inString() == chunks_[i].name)
					{
						chunks_[i].init = true;
					}
				}
			}
		}
	}
}

void dn::SimNode::sendChunk(Chunk& chunk)
{
	zmq_send(chunk.socket, chunk.name.c_str(), chunk.name.size(), ZMQ_SNDMORE);
	zmq_send(chunk.socket, chunk.pData, chunk.size, ZMQ_DONTWAIT);
}
