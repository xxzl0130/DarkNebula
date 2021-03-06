#include "SimNode.h"
#include <zmq.h>
#include <memory>
#include <sstream>
#include <nlohmann/json.hpp>
using nlohmann::json;

namespace dn
{
	// 数据块定义
	struct Chunk
	{
		// 名称
		std::string name;
		// 容量
		size_t size = 0;
		// 所有权限
		bool own = false;
		// 端口
		uint16_t port = 0;
		// 编号
		int id = 0;
		// socket指针
		void* socket = nullptr;
		// 数据指针
		void* pData = nullptr;
		// 缓存指针
		std::unique_ptr<char[]> buffer = nullptr;
		// 路径
		std::string path;
		Chunk() {}
		Chunk(const std::string& n, size_t s, bool write, void* p, const std::string& path_) :
			name(n), size(s), own(write), id(-1), pData(p), buffer(new char[size]),path(path_) {}
	};
}

dn::SimNode::SimNode(const std::string& nodeName, const std::string& nodeIP, uint16_t chunkPort, bool slowNode,
	const std::string& adminIP, uint16_t adminRecvPort, uint16_t adminSendPort) :
	slowNode_(false),
	running_(false),
	id_(-1),
	slowThread_(nullptr),
	slowRunning_(false),
	recordFile_(nullptr),
	recordSize_(0),
	recordBuffer_(nullptr),
	recordFolder_(".")
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
	// 要在这先停止工作，不然一些变量会被析构造成错误
	stopWorking();
	if (slowNode_ && slowThread_)
	{
		// 等待慢速线程执行完
		std::unique_lock<std::mutex> lock(slowMutex_);
		delete slowThread_;
	}
	// 清理数据块的socket
	for (auto& it : chunkList_)
		if (it.socket)
			zmq_close(it.socket);
}

void dn::SimNode::setNodeName(const std::string& name)
{
	if (running_)
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
	if (running_)
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
	// 初始化监听列表
	pollitems_.clear();
	zmq_pollitem_t item{ subSocket_, 0, ZMQ_POLLIN };
	pollitems_.emplace_back(item);

	while (!workStop_ && !pollitems_.empty())
	{
		if (zmq_poll(pollitems_.data(), pollitems_.size(), 1) <= 0)
			continue;
		for (auto i = 0; i < pollitems_.size(); ++i)
		{
			if (pollitems_[i].revents & ZMQ_POLLIN)
			{
				if (i == 0)
				{
					processAdminCommand();
				}
				else
				{
					recvMsg(pollitems_[i].socket);
					if(inString() != chunkList_[i - 1].name)
						continue;
					// 先读到缓存里
					zmq_recv(chunkList_[i - 1].socket, chunkList_[i - 1].buffer.get(), chunkList_[i - 1].size, ZMQ_DONTWAIT);
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
	for (auto& it : chunkList_)
	{
		json obj;
		obj["name"] = it.name;
		obj["own"] = it.own;
		if (it.own)
		{
			// 依次分配端口
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
	// 初始化subSocket_
	if (subSocket_)
		zmq_close(subSocket_);
	subSocket_ = zmq_socket(socketContext_, ZMQ_SUB);
	std::stringstream sub;
	sub << "tcp://" << adminIP_ << ":" << adminSendPort_;
	zmq_setsockopt(subSocket_, ZMQ_SUBSCRIBE, COMMAND_TOPIC, strlen(COMMAND_TOPIC));
	zmq_setsockopt(subSocket_, ZMQ_LINGER, &linger, sizeof linger);
	zmq_connect(subSocket_, sub.str().c_str());

	// 初始化pub
	if (pubSocket_)
		zmq_close(pubSocket_);
	pubSocket_ = zmq_socket(socketContext_, ZMQ_PUB);
	std::stringstream pub;
	pub << "tcp://" << adminIP_ << ":" << adminRecvPort_;
	zmq_setsockopt(pubSocket_, ZMQ_LINGER, &linger, sizeof linger);
	zmq_connect(pubSocket_, pub.str().c_str());
	// 等待连接
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void dn::SimNode::send2Admin(int code, const void* data, size_t size)
{
	assert(pubSocket_ != nullptr);
	auto* header = reinterpret_cast<CommandHeader*>(outBuffer_);
	header->ID = id_;
	header->code = code;
	header->size = size;
	if (data && size)
	{
		if (bufferSize_ < size + sizeof CommandHeader)
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
	if (inString() != COMMAND_TOPIC)
		return;
	recvMsg(subSocket_);
	auto* header = inHeader();
	if (header->ID != ALL_NODE && header->ID != id_)
		return;
	if (header->code == COMMAND_STEP_FORWARD || header->code == COMMAND_STEP_BACKWARD)
	{
		// 拷贝仿真时间信息
		memcpy_s(&curTime_, sizeof curTime_, inData(), sizeof curTime_);
	}
	switch (header->code)
	{
	case COMMAND_INIT:
	{
		auto ok = init();
		if (ok != ERR_NOP)
		{
			send2Admin(COMMAND_INIT, &ok, sizeof ok);
			break;
		}
		simState_ = SimInit;
		if (initCallback_)
			initCallback_();
		// 先把初始化的值拷贝到缓存，下一步前会再拷回来，不然会覆盖垃圾值
		for (auto& it : chunkList_)
		{
			if (!it.own)
			{
				memcpy_s(it.buffer.get(), it.size, it.pData, it.size);
			}
		}
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
		copyNOwnChunks();
		if (replayState_ == Replaying)
		{
			loadNext();
			copyOwnChunks();
		}
		simState_ = SimRun;
		if (!slowNode_)
		{
			if (replayState_ == Replaying && replayStepCallback_)
				replayStepCallback_(simSteps_, curTime_);
			else if (simStepCallback_)
				simStepCallback_(simSteps_, curTime_);
			// 覆盖数据
			if (replayState_ == Replaying)
			{
				copyOwnChunks();
			}
			sendChunks();
			++simSteps_;
		}
		else
		{
			// 慢速节点处理
			if (!slowRunning_)
			{
				delete slowThread_;
				slowThread_ = new std::thread([this]()
					{
						std::unique_lock<std::mutex> lock(slowMutex_);
						slowRunning_ = true;
						if (replayState_ == Replaying && replayStepCallback_)
							replayStepCallback_(simSteps_, curTime_);
						else if (simStepCallback_)
							simStepCallback_(simSteps_, curTime_);
						++simSteps_;
						slowRunning_ = false;
						// 覆盖数据
						if (replayState_ == Replaying)
						{
							copyOwnChunks();
						}
						sendChunks();
					});
				slowThread_->detach();
			}
		}
		send2Admin(COMMAND_STEP_FORWARD, &simSteps_, sizeof simSteps_);
		break;
	case COMMAND_STEP_BACKWARD:
		copyNOwnChunks();
		if (replayState_ == Replaying)
		{
			loadBack();
			copyOwnChunks();
		}
		simState_ = SimRun;
		if (!slowNode_)
		{
			if (replayState_ == Replaying && replayStepBackCallback_)
				replayStepBackCallback_(simSteps_, curTime_);
			else if (simStepCallback_)
				simStepCallback_(simSteps_, curTime_);
			--simSteps_;
			// 覆盖数据
			if (replayState_ == Replaying)
			{
				copyOwnChunks();
			}
			sendChunks();
		}
		else
		{
			// 慢速节点处理
			if (!slowRunning_)
			{
				delete slowThread_;
				slowThread_ = new std::thread([this]()
					{
						std::unique_lock<std::mutex> lock(slowMutex_);
						slowRunning_ = true;
						if (replayState_ == Replaying && replayStepBackCallback_)
							replayStepBackCallback_(simSteps_, curTime_);
						else if (simStepCallback_)
							simStepCallback_(simSteps_, curTime_);
						--simSteps_;

						// 覆盖数据
						if (replayState_ == Replaying)
						{
							copyOwnChunks();
						}
						sendChunks();
						slowRunning_ = false;
					});
				slowThread_->detach();
			}
		}
		send2Admin(COMMAND_STEP_BACKWARD, &simSteps_, sizeof simSteps_);
		break;
	case COMMAND_PAUSE:
		simState_ = SimPause;
		if (pauseCallback_)
			pauseCallback_();
		break;
	case COMMAND_STOP:
		simState_ = SimStop;
		if (recordFile_)
			fclose(recordFile_);
		if (stopCallback_)
			stopCallback_();
		break;
	case COMMAND_REG:
		regIn();
		break;
	case COMMAND_DEL_REC:
		{
		auto name = std::string(inData(), header->size);
		name = recordFolder_ + "/" + name + RECORD_FILE_SUFFIX;
		remove(name.c_str()); // 删除文件
		}
	default:
		break;
	}
}

uint16_t dn::SimNode::init()
{
	auto* header = inHeader();
	auto info = json::parse(std::string(inData(), header->size));
	simTime_ = info["simTime"].get<double>();
	simFree_ = info["free"].get<bool>();
	replayState_ = info["replay"].get<int>();
	stepTime_ = info["step"].get<uint32_t>();
	recordName_ = info["record"].get<std::string>();

	const auto& nodes = info["nodes"];
	if (nodes.contains(nodeName_))
	{
		id_ = nodes[nodeName_]["id"].get<int>();
	}
	else
	{
		return ERR_INFO;
	}
	const auto& chunksInfo = info["chunks"];
	pollitems_.clear();
	zmq_pollitem_t subadmin{ subSocket_, 0, ZMQ_POLLIN };
	pollitems_.emplace_back(subadmin);
	recordSize_ = 0;

	std::vector<zmq_pollitem_t> monitors;
	// 初始化数据块
	for (auto& chunk : chunkList_)
	{
		if (!chunksInfo.contains(chunk.name))
			return ERR_INFO;
		const auto& obj = chunksInfo[chunk.name];
		chunk.id = obj["id"];
		if (chunk.own) // pub
		{
			std::stringstream ss;
			ss << "tcp://*:" << chunk.port;
			if (chunk.path != ss.str())
			{
				if (chunk.socket)
				{
					zmq_close(chunk.socket);
					chunk.socket = nullptr;
				}
				chunk.socket = zmq_socket(socketContext_, ZMQ_PUB);
				chunk.path = ss.str();
				zmq_bind(chunk.socket, chunk.path.c_str());
			}
			recordSize_ += chunk.size;
		}
		else
		{
			if (!obj.contains("path") || obj["path"].get<std::string>().length() <= 10)
			{
				return ERR_INFO;
			}
			if (chunk.socket)
			{
				// 路径没变就保持连接
				if (chunk.path == obj["path"].get<std::string>())
					continue;
				zmq_close(chunk.socket);
				chunk.socket = nullptr;
			}
			chunk.socket = zmq_socket(socketContext_, ZMQ_SUB);
			chunk.path = obj["path"].get<std::string>();
			zmq_connect(chunk.socket, chunk.path.c_str());
			zmq_setsockopt(chunk.socket, ZMQ_SUBSCRIBE, chunk.name.c_str(), chunk.name.size());

			// 连接成功的监控
			auto monitorAddr = "inproc://monitor-" + chunk.name;
			zmq_socket_monitor(chunk.socket, monitorAddr.c_str(), ZMQ_EVENT_CONNECTED);
			zmq_pollitem_t monitor;
			monitor.socket = zmq_socket(socketContext_, ZMQ_PAIR);
			zmq_connect(monitor.socket, monitorAddr.c_str());
			monitor.fd = 0;
			monitor.events = ZMQ_POLLIN;
			monitors.emplace_back(monitor);
		}
		if (chunk.socket)
		{
			// 全部加入监听列表
			zmq_pollitem_t item{ chunk.socket, 0, ZMQ_POLLIN };
			pollitems_.emplace_back(item);
		}
	}

	// 记录文件操作
	if (recordSize_)
		recordBuffer_.reset(new char[recordSize_]);
	auto filename = recordFolder_ + "/" + recordName_ + "_" + nodeName_ + RECORD_FILE_SUFFIX;
	if (recordFile_)
		fclose(recordFile_);
	if (replayState_ == Replaying && recordSize_)
	{
		if (fopen_s(&recordFile_, filename.c_str(), "rb") == 0)
		{
			uint32_t magic;
			fread_s(&magic, sizeof magic, 1, sizeof magic, recordFile_);
			if (magic != RECORD_FILE_MAGIC)
			{
				fclose(recordFile_);
				recordFile_ = nullptr;
				return ERR_FILE_READ;
			}
		}
		else
		{
			recordFile_ = nullptr;
			return ERR_FILE_READ;
		}
	}
	else if (replayState_ == Recording && recordSize_)
	{
		if (fopen_s(&recordFile_, filename.c_str(), "wb") == 0) // 写入magic
		{
			auto magic = RECORD_FILE_MAGIC;
			fwrite(&magic, 1, sizeof magic, recordFile_);
			fflush(recordFile_);
		}
		else
		{
			recordFile_ = nullptr;
			return ERR_FILE_WRITE;
		}
	}

	if (monitors.empty())
		return ERR_NOP;
	auto t0 = std::chrono::steady_clock::now();
	auto connected = 0;
	zmq_msg_t msg;
	bool ok = false;
	// 等待建立连接，3s
	while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - t0).count() < 3.0 && !ok)
	{
		if (zmq_poll(monitors.data(), monitors.size(), 1) <= 0)
			continue;
		for (auto& it : monitors)
		{
			if (!(it.revents & ZMQ_POLLIN))
				continue;
			zmq_msg_init(&msg);
			zmq_msg_recv(&msg, it.socket, 0);
			auto event = *static_cast<uint16_t*>(zmq_msg_data(&msg));
			zmq_msg_init(&msg);
			zmq_msg_recv(&msg, it.socket, 0);
			zmq_msg_close(&msg);

			if (event == ZMQ_EVENT_CONNECTED)
			{
				++connected;
				if (connected >= monitors.size())
				{
					ok = true;
					break;
				}
			}
		}
	}
	for (auto& it : monitors)
		zmq_close(it.socket);

	return ok ? ERR_NOP : ERR_SOCKET;
}

void dn::SimNode::sendChunk(Chunk& chunk)
{
	zmq_send(chunk.socket, chunk.name.c_str(), chunk.name.size(), ZMQ_SNDMORE);
	zmq_send(chunk.socket, chunk.pData, chunk.size, 0);
	if (replayState_ == Recording && recordFile_) // 写入数据
	{
		fwrite(chunk.pData, 1, chunk.size, recordFile_);
		fflush(recordFile_);
	}
}

void dn::SimNode::sendChunks()
{
	for (auto& it : chunkList_)
		if (it.own)
			sendChunk(it);
}

void dn::SimNode::copyNOwnChunks()
{
	for (auto& it : chunkList_)
		if (!it.own)
			memcpy_s(it.pData, it.size, it.buffer.get(), it.size);
}

void dn::SimNode::loadNext()
{
	if (replayState_ == Replaying && recordFile_)
	{
		fread_s(recordBuffer_.get(), recordSize_, 1, recordSize_, recordFile_);
		size_t offset = 0;
		for (auto& it : chunkList_)
		{
			if (it.own)
			{
				memcpy_s(it.buffer.get(), it.size, recordBuffer_.get() + offset, it.size);
				offset += it.size;
			}
		}
		if (feof(recordFile_))
		{
			fclose(recordFile_);
			recordFile_ = nullptr;
			replayState_ = ReplayNop;
		}
	}
}

void dn::SimNode::loadBack()
{

	if (replayState_ == Replaying && recordFile_)
	{
		// 向前偏移两倍
		fseek(recordFile_, -static_cast<long>(recordSize_ * 2), SEEK_CUR);
		loadNext();
	}
}

void dn::SimNode::copyOwnChunks()
{
	for (auto& it : chunkList_)
		if (it.own)
			memcpy_s(it.pData, it.size, it.buffer.get(), it.size);
}

void dn::SimNode::addChunk(const std::string& name, void* pData, size_t size, bool write)
{
	if (chunkSet_.count(name) > 0)
		return;
	chunkSet_.insert(name);
	chunkList_.emplace_back(name, size, write, pData, "");
}

void dn::SimNode::setRecordDataFolder(const std::string& folder)
{
	if (running_)
		return;
	recordFolder_ = folder;
	system(("mkdir " + recordFolder_).c_str());
}
