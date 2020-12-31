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
	// ����ڵ㷢����ָ��
	enum CommandCode
	{
		COMMAND_NOP = 0,
		COMMAND_INIT,		// ��ʼ��
		COMMAND_STEP,		// �ƽ�һ��
		COMMAND_STOP,		// ����
		COMMAND_RECORD,		// ��¼����
		COMMAND_LOAD,		// ��������
		COMMAND_SIM,		// ����ģʽ
		COMMAND_REPLAY,		// �ط�ģʽ
	};

	// ����ڵ�ر�
	enum ReplyCode
	{
		REPLY_NOP = 0,
		REPLY_INIT,			// ��ʼ�����
		REPLY_STEP,			// �ƽ�һ��
		REPLY_REG,			// ע��
	};

	struct DN_EXPORT CommandHeader
	{
		// �ڵ�ID��-1Ϊȫ��
		int ID;
		int code;
		uint32_t size;
	};

	constexpr auto COMMAND_TOPIC = "command";
	constexpr auto REPLY_TOPIC = "reply";
}
