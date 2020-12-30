#pragma once
#include "DarkNebulaGlobal.h"
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

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
		// ���沽��
		uint32_t steps;
		// ���������ݿ�
		std::vector<NodeChunks> chunks;
	};
	template class DN_EXPORT std::vector<NodeInfo>;

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
	};
}

