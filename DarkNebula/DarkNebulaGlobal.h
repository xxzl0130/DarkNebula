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

namespace dn
{
	// 管理节点发布的指令
	enum CommandCode
	{
		COMMAND_NOP = 0,
		COMMAND_INIT,		// 初始化
		COMMAND_STEP,		// 推进一步
		COMMAND_STOP,		// 结束
		COMMAND_RECORD,		// 记录数据
		COMMAND_LOAD,		// 加载数据
		COMMAND_SIM,		// 仿真模式
		COMMAND_REPLAY,		// 重放模式
	};

	// 仿真节点回报
	enum ReplyCode
	{
		REPLY_NOP = 0,
		REPLY_INIT,			// 初始化完成
		REPLY_STEP,			// 推进一步
		REPLY_REG,			// 注册
	};

	struct DN_EXPORT CommandHeader
	{
		// 节点ID，-1为全部
		int ID;
		int code;
		uint32_t size;
	};

	constexpr auto COMMAND_TOPIC = "command";
	constexpr auto REPLY_TOPIC = "reply";
}
