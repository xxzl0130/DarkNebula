#pragma once
#include "DarkNebulaGlobal.h"
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

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
		// 仿真步数
		uint32_t steps;
		// 包含的数据块
		std::vector<NodeChunks> chunks;
	};
	template class DN_EXPORT std::vector<NodeInfo>;

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
	};
}

