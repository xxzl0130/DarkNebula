#pragma once
#include <thread>
#include <mutex>
#include "DarkNebulaGlobal.h"

namespace dn
{
	class DN_EXPORT Node
	{
	public:
		Node();
		virtual ~Node();
		
		enum SimState
		{
			SimNop = 0,
			SimInit,		// 初始化
			SimStop,		// 停止中
			SimRun = 0x10,	// 运行中
			SimPause,		// 暂停中
			SimStep,		// 单步模式
		};

		enum ReplayState
		{
			ReplayNop = 0,
			Recording = 0x10,		// 记录数据
			Replaying = 0x20,		// 重放数据
		};

		// 设置缓冲区大小，默认1MB
		virtual void setBufferSize(size_t bytes);
		size_t getBufferSize();

		// 获取当前仿真步数
		uint32_t getSimStep() const;
		// 获取当前仿真时间
		double getSimTime() const;

		// 获取仿真状态
		SimState getSimState() const;
		// 是否为自由仿真
		bool isFreeSim() const;
		// 是否为重放仿真
		bool isReplaySim() const;
		// 获取当前仿真时间
		double getCurTime() const;
		// 获取当前仿真步数
		unsigned getCurSteps() const;
		// 获取记录名称
		std::string getRecordName() const;

	protected:
		
		// 开始工作线程
		void startWorking();
		// 停止工作线程
		void stopWorking();
		// 工作线程
		virtual void working() = 0;
		// 获取指针
		char* inBufferData() const;
		// 获取接收字符串
		std::string inString() const;
		// 获取头
		CommandHeader* inHeader() const;
		// 获取头部后续的数据指针
		char* inData() const;
		// 从socket接收数据读到inBuffer
		int recvMsg(void* socket);
		
		// 发布指令的socket
		void* pubSocket_;
		// 接收回报的socket
		void* subSocket_;
		// socket环境
		void* socketContext_;
		// 工作线程
		std::thread* workThread_;
		// 监听线程停止标志
		bool workStop_;
		// 输出缓冲区
		char* outBuffer_;
		// 输入缓冲区
		zmq_msg_t* inBuffer_;
		// 缓冲区大小
		size_t bufferSize_;
		// 监听锁
		std::mutex workMutex_;

		/// 步数和时间会合并发送，在这里的结构不能改
		// 仿真步数
		uint32_t simSteps_;
		// 当前仿真时间
		double curTime_;
		// 最长仿真时间
		double simTime_;
		// 自由仿真类型
		bool simFree_;
		// 仿真步长,ms
		uint32_t stepTime_;
		// 仿真状态
		SimState simState_;
		// 回放状态
		int replayState_;
		// 记录名称
		std::string recordName_;
	};
}

