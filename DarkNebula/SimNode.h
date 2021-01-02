#pragma once
#include <functional>
#include <thread>
#include <string>
#include <cstdint>
#include <mutex>
#include <vector>



#include "DarkNebulaGlobal.h"

namespace dn
{
	// 仿真指令回调函数
	typedef std::function<void(void)> SimEventCallback;
	// 仿真推进回调函数
	typedef std::function<void(uint32_t,double)> SimStepCallback;
	
	class DN_EXPORT SimNode
	{
	public:
		/**
		 * \brief 构造函数
		 * \param nodeName 本节点名称，各节点名称要唯一
		 * \param nodeIP 本节点IP地址
		 * \param chunkPort 数据块起始端口，每个本节点发布的数据块的端口在此基础上依次加一
		 * \param slowNode 慢速节点，即本节点不保证跟随每一步仿真
		 * \param adminIP 管理节点IP
		 * \param adminRecvPort 管理节点接收指令的端口
		 * \param adminSendPort 管理节点发送指令的端口
		 */
		SimNode(const std::string& nodeName, const std::string& nodeIP = "127.0.0.1", uint16_t chunkPort = 10000, bool slowNode = false,
			const std::string& adminIP = "127.0.0.1", uint16_t adminRecvPort = ADMIN_RECEIVE_PORT, uint16_t adminSendPort = ADMIN_SEND_PORT);
		~SimNode();
		
		/// 节点参数设置，需要在注册以前设置
		// 节点名称
		void setNodeName(const std::string& name);
		// 节点IP
		void setNodeIP(const std::string& IP);
		// 数据块起始端口
		void setChunkPort(uint16_t port);
		// 设置是否为慢速节点
		void setSlowNode(bool slow);
		// 管理节点IP
		void setAdminIP(const std::string& IP);
		// 管理节点接收指令的端口
		void setAdminRecvPort(uint16_t port);
		// 管理节点发送指令的端口
		void setAdminSendPort(uint16_t port);

		// 注册，之后无法修改参数
		bool regIn();

		/// 回调函数设置，注意回调函数均运行在子线程
		// 仿真初始化回调函数
		void setInitCallback(SimEventCallback callback);
		// 仿真开始回调函数，可不设置
		void setStartCallback(SimEventCallback callback);
		// 仿真暂停回调函数，可不设置
		void setPauseCallback(SimEventCallback callback);
		// 仿真停止回调函数，可不设置
		void setStopCallback(SimEventCallback callback);
		// 仿真推进函数
		void setSimStepCallback(SimStepCallback callback);
		// 回放推进函数，不设置时遇到回放将调用普通仿真函数
		void setReplayStepCallback(SimStepCallback callback);

		// 获取当前仿真步数
		uint32_t getSimStep() const;
		// 获取当前仿真时间
		double getSimTime() const;

	private:
		// 运行线程
		void working();
		// 停止
		void stop();
		// 发送注册消息
		bool sendReg();
		// 初始化
		void initSocket();

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
			// socket指针
			void* socket;
		};

	private:
		SimEventCallback initCallback_, startCallback_, pauseCallback_, stopCallback_;
		SimStepCallback simStepCallback_, replayStepCallback_;

		/// 步数和时间会合并发送，在这里的结构不能改
		// 仿真步数
		uint32_t simSteps_;
		// 当前仿真时间
		double simTime_;

		// 节点名称
		std::string nodeName_;
		// 节点IP
		std::string nodeIP_;
		// 数据块IP
		uint16_t chunkPort_;
		// 管理节点IP
		std::string adminIP_;
		// 管理节点接收端口
		uint16_t adminRecvPort_;
		// 管理节点发送端口
		uint16_t adminSendPort_;
		// 慢速节点
		bool slowNode_;
		// 运行中
		bool running_;
		// 通信线程停止标志
		bool comStop_;
		// 通信线程锁
		std::mutex workMutex_;
		// 通信线程
		std::thread* workThread_;

		// socket环境
		void* socketContext_;
		// 向管理节点发布指令的socket
		void* pubSocket_;
		// 从管理节点接收指令的socket
		void* subSocket_;
		// 数据块列表
		std::vector<Chunk> chunks_;
	};
}

