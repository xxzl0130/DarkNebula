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
#include <cstdint>

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
		COMMAND_RECORD,				// 记录数据
		COMMAND_LOAD,				// 加载数据
		COMMAND_SIM,				// 仿真模式
		COMMAND_REPLAY,				// 重放模式
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
}
