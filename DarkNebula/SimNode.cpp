#include "SimNode.h"
#include <zmq.h>
#include <memory>
#include <sstream>
#include <nlohmann/json.hpp>
using nlohmann::json;

dn::SimNode::SimNode(const std::string& nodeName, const std::string& nodeIP, uint16_t chunkPort, bool slowNode,
	const std::string& adminIP, uint16_t adminRecvPort, uint16_t adminSendPort):
	slowNode_(false),
	running_(false),
	id_(-1),
	slowThread_(nullptr),
	slowRunning_(false)
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
	// Ҫ������ֹͣ��������ȻһЩ�����ᱻ������ɴ���
	stopWorking();
	if (slowNode_ && slowThread_)
	{
		// �ȴ������߳�ִ����
		std::unique_lock<std::mutex> lock(slowMutex_);
		delete slowThread_;
	}
	// �������ݿ��socket
	for (auto& it : chunkList_)
		if (it.socket)
			zmq_close(it.socket);
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
	startWorking();
	running_ = true;
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

void dn::SimNode::setSimStepBackCallback(SimStepCallback callback)
{
	replayStepBackCallback_ = std::move(callback);
}

void dn::SimNode::setReplayStepCallback(SimStepCallback callback)
{
	replayStepCallback_ = std::move(callback);
}

void dn::SimNode::working()
{
	std::unique_lock<std::mutex> lock(workMutex_);
	// ��ʼ�������б�
	pollitems_.clear();
	zmq_pollitem_t item{ subSocket_, 0, ZMQ_POLLIN };
	pollitems_.emplace_back(item);

	while(!workStop_ && !pollitems_.empty())
	{
		if (zmq_poll(pollitems_.data(), pollitems_.size(), 1) > 0)
		{
			for (auto i = 0; i < pollitems_.size(); ++i)
			{
				if (pollitems_[i].revents == ZMQ_POLLIN)
				{
					if (i == 0)
					{
						processAdminCommand();
					}
					else
					{
						// �ȶ���������
						zmq_recv(chunkList_[i - 1].socket, chunkList_[i - 1].buffer.get(), chunkList_[i - 1].size, ZMQ_DONTWAIT);
					}
				}
			}
		}
	}
}

bool dn::SimNode::sendReg()
{
	json info;
	info["name"] = nodeName_;
	info["ip"] = nodeIP_;
	info["slow"] = slowNode_;
	auto chunksJson = json::array();
	auto port = chunkPort_;
	for(auto& it : chunkList_)
	{
		json obj;
		obj["name"] = it.name;
		obj["own"] = it.own;
		if(it.own)
		{
			// ���η���˿�
			obj["port"] = it.port = port++;
		}
		chunksJson.push_back(obj);
	}
	info["chunks"] = chunksJson;

	const auto jsonStr = info.dump();
	send2Admin(COMMAND_REG, jsonStr.data(), jsonStr.size());
	return true;
}

void dn::SimNode::initSocket()
{
	int linger = 0;
	// ��ʼ��subSocket_
	if (subSocket_)
		zmq_close(subSocket_);
	subSocket_ = zmq_socket(socketContext_, ZMQ_SUB);
	std::stringstream sub;
	sub << "tcp://" << adminIP_ << ":" << adminSendPort_;
	zmq_setsockopt(subSocket_, ZMQ_SUBSCRIBE, COMMAND_TOPIC, strlen(COMMAND_TOPIC));
	zmq_setsockopt(subSocket_, ZMQ_LINGER, &linger, sizeof linger);
	zmq_connect(subSocket_, sub.str().c_str());
	
	// ��ʼ��pub
	if (pubSocket_)
		zmq_close(pubSocket_);
	pubSocket_ = zmq_socket(socketContext_, ZMQ_PUB);
	std::stringstream pub;
	pub << "tcp://" << adminIP_ << ":" << adminRecvPort_;
	zmq_setsockopt(pubSocket_, ZMQ_LINGER, &linger, sizeof linger);
	zmq_connect(pubSocket_, pub.str().c_str());
	// �ȴ�����
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void dn::SimNode::send2Admin(int code, const void* data, size_t size)
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


void dn::SimNode::processAdminCommand()
{
	recvMsg(subSocket_);
	if(inString() != COMMAND_TOPIC)
		return;
	recvMsg(subSocket_);
	auto* header = inHeader();
	if(header->ID != ALL_NODE && header->ID != id_)
		return;
	if(header->code == COMMAND_STEP_FORWARD || header->code == COMMAND_STEP_BACKWARD)
	{
		// ��������ʱ����Ϣ
		memcpy_s(&curTime_, sizeof curTime_, inData(), sizeof curTime_);
	}
	switch (header->code)
	{
	case COMMAND_INIT:
		init();
		simState_ = SimInit;
		if (initCallback_)
			initCallback_();
		// �Ȱѳ�ʼ����ֵ���������棬��һ��ǰ���ٿ���������Ȼ�Ḳ������ֵ
		for(auto& it : chunkList_)
		{
			if(!it.own)
			{
				memcpy_s(it.buffer.get(), it.size, it.pData, it.size);
			}
		}
		{
			auto ok = true;
			send2Admin(COMMAND_INIT, &ok, sizeof ok);
		}
		break;
	case COMMAND_START:
		simSteps_ = 0;
		curTime_ = 0;
		simState_ = SimRun;
		if (startCallback_)
			startCallback_();
		break;
	case COMMAND_STEP_FORWARD:
		copyChunks();
		simState_ = SimRun;
		if (!slowNode_)
		{
			if (simReplay_ && replayStepCallback_)
				replayStepCallback_(simSteps_, curTime_);
			else if (simStepCallback_)
				simStepCallback_(simSteps_, curTime_);
			++simSteps_;
		}
		else
		{
			// ���ٽڵ㴦��
			if(!slowRunning_)
			{
				delete slowThread_;
				slowThread_ = new std::thread([this]()
					{
						std::unique_lock<std::mutex> lock(slowMutex_);
						slowRunning_ = true;
						if (simReplay_ && replayStepCallback_)
							replayStepCallback_(simSteps_, curTime_);
						else if (simStepCallback_)
							simStepCallback_(simSteps_, curTime_);
						++simSteps_;
						slowRunning_ = false;
					});
				slowThread_->detach();
			}
		}
		sendChunks();
		send2Admin(COMMAND_STEP_FORWARD, &simSteps_, sizeof simSteps_);
		break;
	case COMMAND_STEP_BACKWARD:
		copyChunks();
		simState_ = SimRun;
		if (!slowNode_)
		{
			if (simReplay_ && replayStepBackCallback_)
				replayStepBackCallback_(simSteps_, curTime_);
			else if (simStepCallback_)
				simStepCallback_(simSteps_, curTime_);
			--simSteps_;
		}
		else
		{
			// ���ٽڵ㴦��
			if (!slowRunning_)
			{
				delete slowThread_;
				slowThread_ = new std::thread([this]()
					{
						std::unique_lock<std::mutex> lock(slowMutex_);
						slowRunning_ = true;
						if (simReplay_ && replayStepBackCallback_)
							replayStepBackCallback_(simSteps_, curTime_);
						else if (simStepCallback_)
							simStepCallback_(simSteps_, curTime_);
						--simSteps_;
						slowRunning_ = false;
					});
				slowThread_->detach();
			}
		}
		sendChunks();
		send2Admin(COMMAND_STEP_BACKWARD, &simSteps_, sizeof simSteps_);
		break;
	case COMMAND_PAUSE:
		simState_ = SimPause;
		if (pauseCallback_)
			pauseCallback_();
		break;
	case COMMAND_STOP:
		simState_ = SimStop;
		if (stopCallback_)
			stopCallback_();
		break;
	case COMMAND_RECORD:
		break;//TODO
	case COMMAND_LOAD:
		break;//TODO
	case COMMAND_SIM:
		break;//TODO
	case COMMAND_REPLAY:
		break;//TODO
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
	pollitems_.resize(1); //����������ָ���socket
	// ��ʼ�����ݿ�
	for (auto& chunk : chunkList_)
	{
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
				ss << "tcp://*:" << chunk.port;
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
			// ȫ����������б�
			zmq_pollitem_t item{ chunk.socket, 0, ZMQ_POLLIN };
			pollitems_.emplace_back(item);
		}
	}
	simTime_ = info["simTime"].get<double>();
	simFree_ = info["free"].get<bool>();
	simReplay_ = info["replay"].get<bool>();
	stepTime_ = info["step"].get<uint32_t>();
	recordName_ = info["record"].get<std::string>();
	// �ȴ���������
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void dn::SimNode::sendChunk(Chunk& chunk)
{
	zmq_send(chunk.socket, chunk.name.c_str(), chunk.name.size(), ZMQ_SNDMORE);
	zmq_send(chunk.socket, chunk.pData, chunk.size, ZMQ_DONTWAIT);
}

void dn::SimNode::sendChunks()
{
	for (auto& it : chunkList_)
		if (it.own)
			sendChunk(it);
}

void dn::SimNode::copyChunks()
{
	for (auto& it : chunkList_)
		if (!it.own)
			memcpy_s(it.pData, it.size, it.buffer.get(), it.size);
}

void dn::SimNode::addChunk(const std::string& name, void* pData, size_t size, bool write)
{
	if(chunkSet_.count(name) > 0)
		return;
	chunkSet_.insert(name);
	chunkList_.emplace_back(name,size,write,pData);
}
