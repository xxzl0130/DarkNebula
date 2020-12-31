#pragma once
#include "DarkNebulaGlobal.h"
#include <string>
#include <vector>
#include <cstdint>

namespace std
{
	class thread;
}

namespace dn
{
	// ���ݿ���Ϣ�����ƺͶ˿ں�
	typedef std::pair<std::string, uint16_t> ChunkInfo;

	// ÿ���ڵ���������ݿ����Ϣ����ź�����Ȩ
	typedef std::pair<int, bool> NodeChunks;

	struct DN_EXPORT NodeInfo
	{
		// ����
		std::string name;
		// IP
		std::string ip;
		// ���沽��
		uint32_t steps;
		// ���������ݿ�
		std::vector<NodeChunks> chunks;
	};

	enum SimState
	{
		SimNop = 0,
		SimInit,	// ��ʼ��
		SimRun,		// ������
		SimPause,	// ��ͣ��
		SimStop		// ֹͣ��
	};
	
	class DN_EXPORT AdminNode
	{
	public:
		/**
		 * \brief ���캯��
		 * \param receivePort ���սڵ�ر��Ķ˿�
		 * \param sendPort ����ָ��Ķ˿�
		 */
		AdminNode(uint16_t receivePort = 6666, uint16_t sendPort = 8888);
		~AdminNode();

		void setReceivePort(uint16_t port);
		uint16_t getReceivePort() const { return receivePort_; }

		void setSendPort(uint16_t port);
		uint16_t getSendPort() const { return sendPort_; }

		// ��ȡ����״̬
		SimState getSimState() const { return simState_; }

		// ��ȡ�ڵ���Ŀ
		size_t getNodeCount() const { return nodeList_.size(); }
		// ��ȡ�ڵ���Ϣ
		std::vector<NodeInfo> getNodeList() const { return nodeList_; }

		// ��ȡ���ݿ���Ŀ
		size_t getChunkCount() const { return chunkList_.size(); }
		// ��ȡ���ݿ���Ϣ
		std::vector<ChunkInfo> getChunkList() const { return chunkList_; }

		// ���û�������С��Ĭ��1MB
		void setBufferSize(size_t bytes);
		// ��ȡ��������С
		size_t getBufferSize() const { return bufferSize_; }

	private:
		// ��ʼ����
		void startListen();
		// ֹͣ����
		void stopListen();
		// ��������
		void listen();
		// ������Ϣ
		void sendMsg(void* buffer, size_t len);
		// ���������outBuffer��0������
		void sendCommand(int id, int code, size_t size = 0, void const* data = nullptr);
		
	private:
		// ���սڵ�ر��Ķ˿�
		uint16_t receivePort_;
		// ����ָ��Ķ˿�
		uint16_t sendPort_;
		// ����״̬
		SimState simState_;
		// �ڵ���Ϣ
		std::vector<NodeInfo> nodeList_;
		// ���ݿ���Ϣ
		std::vector<ChunkInfo> chunkList_;
		// ����ָ���socket
		void* pubSocket_;
		// ���ջر���socket
		void* subSocket_;
		// socket����
		void* socketContext_;
		// �����߳�
		std::thread* listenThread_;
		// �����߳�ֹͣ��־
		bool listenStop_,listenStopped_;
		// ������
		char* inBuffer, *outBuffer;
		// ��������С
		size_t bufferSize_;
	};
}

