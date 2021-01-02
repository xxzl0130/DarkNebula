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

		// 设置缓冲区大小，默认1MB
		virtual void setBufferSize(size_t bytes);
		size_t getBufferSize();

		// 获取当前仿真步数
		uint32_t getSimStep() const;
		// 获取当前仿真时间
		double getSimTime() const;

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
	};
}

