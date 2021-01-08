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
	};

	enum SimState
	{
		SimNop = 0,
		SimInit,		// ��ʼ��
		SimStop,		// ֹͣ��
		SimRun = 0x10,	// ������
		SimPause,		// ��ͣ��
		SimStep,		// ����ģʽ
	};

	enum ReplayState
	{
		ReplayNop = 0,
		Recording = 0x10,		// ��¼����
		Replaying = 0x20,		// �ط�����
	};

	enum ErrorCode : uint16_t
	{
		ERR_NOP		= 0,		// �޴���
		ERR_SOCKET,				// socket����
		ERR_FILE_READ,			// �ļ���ȡʧ��
		ERR_FILE_WRITE,			// �ļ�д��ʧ��
		ERR_INFO,				// ��Ϣ����
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
	constexpr auto COMMAND_TOPIC_LEN = 7;
	constexpr auto REPLY_TOPIC = "reply";
	constexpr auto REPLY_TOPIC_LEN = 5;
	constexpr uint16_t ADMIN_RECEIVE_PORT = 6666;
	constexpr uint16_t ADMIN_SEND_PORT = 8888;
	constexpr uint16_t CHUNK_PORT = 10000;
	constexpr auto RECORD_FILE_SUFFIX = ".dnr";
	constexpr uint32_t RECORD_FILE_MAGIC = 0x44417A9F;
}
