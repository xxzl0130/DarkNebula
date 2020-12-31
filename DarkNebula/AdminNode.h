#pragma once
#include "DarkNebulaGlobal.h"
#include <string>
#include <vector>
#include <cstdint>

namespace std
{
	class thread;
}

namespace dn
{
	// 数据块信息，名称和端口号
	typedef std::pair<std::string, uint16_t> ChunkInfo;

	// 每个节点包含的数据块的信息，编号和所有权
	typedef std::pair<int, bool> NodeChunks;

	struct DN_EXPORT NodeInfo
	{
		// 名称
		std::string name;
		// IP
		std::string ip;
		// 仿真步数
		uint32_t steps;
		// 包含的数据块
		std::vector<NodeChunks> chunks;
	};

	enum SimState
	{
		SimNop = 0,
		SimInit,	// 初始化
		SimRun,		// 运行中
		SimPause,	// 暂停中
		SimStop		// 停止中
	};
	
	class DN_EXPORT AdminNode
	{
	public:
		/**
		 * \brief 构造函数
		 * \param receivePort 接收节点回报的端口
		 * \param sendPort 发布指令的端口
		 */
		AdminNode(uint16_t receivePort = 6666, uint16_t sendPort = 8888);
		~AdminNode();

		void setReceivePort(uint16_t port);
		uint16_t getReceivePort() const { return receivePort_; }

		void setSendPort(uint16_t port);
		uint16_t getSendPort() const { return sendPort_; }

		// 获取仿真状态
		SimState getSimState() const { return simState_; }

		// 获取节点数目
		size_t getNodeCount() const { return nodeList_.size(); }
		// 获取节点信息
		std::vector<NodeInfo> getNodeList() const { return nodeList_; }

		// 获取数据块数目
		size_t getChunkCount() const { return chunkList_.size(); }
		// 获取数据块信息
		std::vector<ChunkInfo> getChunkList() const { return chunkList_; }

		// 设置缓冲区大小，默认1MB
		void setBufferSize(size_t bytes);
		// 获取缓冲区大小
		size_t getBufferSize() const { return bufferSize_; }

	private:
		// 开始监听
		void startListen();
		// 停止监听
		void stopListen();
		// 监听函数
		void listen();
		// 发送消息
		void sendMsg(void* buffer, size_t len);
		// 发送命令，在outBuffer的0处制作
		void sendCommand(int id, int code, size_t size = 0, void const* data = nullptr);
		
	private:
		// 接收节点回报的端口
		uint16_t receivePort_;
		// 发布指令的端口
		uint16_t sendPort_;
		// 仿真状态
		SimState simState_;
		// 节点信息
		std::vector<NodeInfo> nodeList_;
		// 数据块信息
		std::vector<ChunkInfo> chunkList_;
		// 发布指令的socket
		void* pubSocket_;
		// 接收回报的socket
		void* subSocket_;
		// socket环境
		void* socketContext_;
		// 监听线程
		std::thread* listenThread_;
		// 监听线程停止标志
		bool listenStop_,listenStopped_;
		// 缓冲区
		char* inBuffer, *outBuffer;
		// 缓冲区大小
		size_t bufferSize_;
	};
}

