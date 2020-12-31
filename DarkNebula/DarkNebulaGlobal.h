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
	// ָ��
	enum CommandCode
	{
		COMMAND_NOP = 0,
		COMMAND_INIT,				// ��ʼ��
		COMMAND_REG,				// ע��
		COMMAND_START,				// ��ʼ
		COMMAND_STEP_FORWARD,		// ��ǰ�ƽ�һ��
		COMMAND_STEP_BACKWARD,		// ����ƽ�һ��
		COMMAND_STEP_IN,			// ����ģʽ
		COMMAND_PAUSE,				// ��ͣ
		COMMAND_STOP,				// ����
		COMMAND_RECORD,				// ��¼����
		COMMAND_LOAD,				// ��������
		COMMAND_SIM,				// ����ģʽ
		COMMAND_REPLAY,				// �ط�ģʽ
	};

	struct DN_EXPORT CommandHeader
	{
		// �ڵ�ID��-1Ϊȫ��
		int ID;
		int code;
		uint32_t size;
	};

	constexpr auto ALL_NODE = -1;

	constexpr auto COMMAND_TOPIC = "command";
	constexpr auto REPLY_TOPIC = "reply";
}
