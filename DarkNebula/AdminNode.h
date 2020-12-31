#pragma once
#include "DarkNebulaGlobal.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "Timer.h"

namespace std
{
	class thread;
}

namespace dn
{
	// 数据块信息，名称和地址
	typedef std::pair<std::string, std::string> ChunkInfo;

	// 每个节点包含的数据块的信息，编号和所有权
	typedef std::pair<int, bool> NodeChunks;

	struct DN_EXPORT NodeInfo
	{
		// 名称
		std::string name;
		// IP
		std::string ip;
		// 仿真步数
		uint32_t steps = 0;
		// 仿真类型
		bool replay = false;
		// 初始化
		bool init = false;
		// 包含的数据块
		std::vector<NodeChunks> chunks;
	};

	enum SimState
	{
		SimNop = 0,
		SimInit,		// 初始化
		SimStop,		// 停止中
		SimRun = 0x10,	// 运行中
		SimPause,		// 暂停中
		SimStep,		// 单步模式
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

		/// 环境设置

		// 设置接收端口
		void setReceivePort(uint16_t port);
		uint16_t getReceivePort() const { return receivePort_; }

		// 设置发送端口
		void setSendPort(uint16_t port);
		uint16_t getSendPort() const { return sendPort_; }

		// 获取仿真状态
		SimState getSimState() const { return simState_; }
		bool isFreeSim() const { return simFree_; }
		bool isReplaySim() const { return simReplay_; }

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

		// 获取当前仿真时间
		double getCurTime() const { return curTime_; }
		// 获取当前仿真步数
		int getCurSteps() const { return simSteps_; }
		// 清除所有已经注册的节点信息
		void clear();

		/// 仿真设置
		
		// 设置是否为自由仿真
		void setFreeSim(bool en);
		// 设置仿真时长，小于0为自由仿真，回放模式时长不能大于录制文件
		void setSimTime(double time);
		// 设置仿真步长
		void setStepTime(unsigned ms);
		// 设置启用录制，启用时需要提供保存的文件名
		void setRecord(bool enable, char const* name = nullptr);
		/**
		 * \brief 设置启用回放
		 * \param node 节点编号，-1为所有节点
		 * \param enable 是否启用
		 * \param name 启用时需要提供保存的文件名
		 */
		void setReply(int node, bool enable, char const* name = nullptr);

		/// 仿真命令
		// 初始化仿真
		void initSim();
		// 开始仿真
		void startSim();
		// 暂停仿真
		void pauseSim();
		// 停止仿真
		void stopSim();
		// 单步推进
		void stepForward();
		// 单步后退
		void stepBackward();

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

		// 检查所有节点初始化是否完成
		bool checkInit();
		// 处理节点注册
		void nodeReg(char* buffer, int len);
		// 处理节点初始化
		void nodeInit(char* buffer, int len);
		// 处理节点推进
		void nodeStep(char* buffer, int len);

		void timerEvent();
		
	private:
		// 接收节点回报的端口
		uint16_t receivePort_;
		// 发布指令的端口
		uint16_t sendPort_;
		// 仿真状态
		SimState simState_;
		// 节点信息
		std::vector<NodeInfo> nodeList_;
		std::map<std::string, int> nodeMap_;
		// 数据块信息
		std::vector<ChunkInfo> chunkList_;
		std::map<std::string, int> chunkMap_;
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

		// 当前仿真时间
		double curTime_;
		// 最长仿真时间
		double simTime_;
		// 仿真步数
		int simSteps_;
		// 自由仿真类型
		bool simFree_;
		// 回放仿真类型
		bool simReplay_;
		// 仿真步长
		unsigned stepTime_;
		// 记录名称
		std::string recordName_;
		// 定时器
		Timer timer_;
	};
}

