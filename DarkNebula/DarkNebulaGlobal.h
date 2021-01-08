#pragma once

#ifndef BUILD_STATIC
#ifdef DARKNEBULA_EXPORTS
#define DN_EXPORT __declspec(dllexport)
#else
#define DN_EXPORT __declspec(dllimport)
#endif
#else
#define DN_EXPORT
#endif

#pragma warning(disable:4267)
#pragma warning(disable:4251)

#include <cstdint>
#include <functional>
#include <thread>
#include <string>
#include <cstdint>
#include <mutex>
#include <vector>
#include <map>
#include <set>
#include <algorithm>


struct zmq_msg_t;
struct zmq_pollitem_t;

namespace dn
{
	// 指令
	enum CommandCode
	{
		COMMAND_NOP = 0,
		COMMAND_INIT,				// 初始化
		COMMAND_REG,				// 注册
		COMMAND_START,				// 开始
		COMMAND_STEP_FORWARD,		// 向前推进一步
		COMMAND_STEP_BACKWARD,		// 向后推进一步
		COMMAND_STEP_IN,			// 单步模式
		COMMAND_PAUSE,				// 暂停
		COMMAND_STOP,				// 结束
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

	enum ReplayState
	{
		ReplayNop = 0,
		Recording = 0x10,		// 记录数据
		Replaying = 0x20,		// 重放数据
	};

	enum ErrorCode : uint16_t
	{
		ERR_NOP		= 0,		// 无错误
		ERR_SOCKET,				// socket错误
		ERR_FILE_READ,			// 文件读取失败
		ERR_FILE_WRITE,			// 文件写入失败
		ERR_INFO,				// 信息不足
	};

	struct DN_EXPORT CommandHeader
	{
		// 节点ID，-1为全部
		int ID;
		int code;
		uint32_t size;
	};

	constexpr auto ALL_NODE = -1;

	constexpr auto COMMAND_TOPIC = "command";
	constexpr auto COMMAND_TOPIC_LEN = 7;
	constexpr auto REPLY_TOPIC = "reply";
	constexpr auto REPLY_TOPIC_LEN = 5;
	constexpr uint16_t ADMIN_RECEIVE_PORT = 6666;
	constexpr uint16_t ADMIN_SEND_PORT = 8888;
	constexpr uint16_t CHUNK_PORT = 10000;
	constexpr auto RECORD_FILE_SUFFIX = ".dnr";
	constexpr uint32_t RECORD_FILE_MAGIC = 0x44417A9F;
}
