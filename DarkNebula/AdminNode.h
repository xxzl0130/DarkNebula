#pragma once
#include "DarkNebulaGlobal.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "Timer.h"

namespace std
{
	class thread;
}

namespace dn
{
	// ���ݿ���Ϣ�����ƺ͵�ַ
	typedef std::pair<std::string, std::string> ChunkInfo;

	// ÿ���ڵ���������ݿ����Ϣ����ź�����Ȩ
	typedef std::pair<int, bool> NodeChunks;

	struct DN_EXPORT NodeInfo
	{
		// ����
		std::string name;
		// IP
		std::string ip;
		// ���沽��
		uint32_t steps = 0;
		// ��������
		bool replay = false;
		// ��ʼ��
		bool init = false;
		// ���������ݿ�
		std::vector<NodeChunks> chunks;
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

		/// ��������

		// ���ý��ն˿�
		void setReceivePort(uint16_t port);
		uint16_t getReceivePort() const { return receivePort_; }

		// ���÷��Ͷ˿�
		void setSendPort(uint16_t port);
		uint16_t getSendPort() const { return sendPort_; }

		// ��ȡ����״̬
		SimState getSimState() const { return simState_; }
		bool isFreeSim() const { return simFree_; }
		bool isReplaySim() const { return simReplay_; }

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

		// ��ȡ��ǰ����ʱ��
		double getCurTime() const { return curTime_; }
		// ��ȡ��ǰ���沽��
		int getCurSteps() const { return simSteps_; }
		// ��������Ѿ�ע��Ľڵ���Ϣ
		void clear();

		/// ��������
		
		// �����Ƿ�Ϊ���ɷ���
		void setFreeSim(bool en);
		// ���÷���ʱ����С��0Ϊ���ɷ��棬�ط�ģʽʱ�����ܴ���¼���ļ�
		void setSimTime(double time);
		// ���÷��沽��
		void setStepTime(unsigned ms);
		// ��������¼�ƣ�����ʱ��Ҫ�ṩ������ļ���
		void setRecord(bool enable, char const* name = nullptr);
		/**
		 * \brief �������ûط�
		 * \param node �ڵ��ţ�-1Ϊ���нڵ�
		 * \param enable �Ƿ�����
		 * \param name ����ʱ��Ҫ�ṩ������ļ���
		 */
		void setReply(int node, bool enable, char const* name = nullptr);

		/// ��������
		// ��ʼ������
		void initSim();
		// ��ʼ����
		void startSim();
		// ��ͣ����
		void pauseSim();
		// ֹͣ����
		void stopSim();
		// �����ƽ�
		void stepForward();
		// ��������
		void stepBackward();

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

		// ������нڵ��ʼ���Ƿ����
		bool checkInit();
		// ����ڵ�ע��
		void nodeReg(char* buffer, int len);
		// ����ڵ��ʼ��
		void nodeInit(char* buffer, int len);
		// ����ڵ��ƽ�
		void nodeStep(char* buffer, int len);

		void timerEvent();
		
	private:
		// ���սڵ�ر��Ķ˿�
		uint16_t receivePort_;
		// ����ָ��Ķ˿�
		uint16_t sendPort_;
		// ����״̬
		SimState simState_;
		// �ڵ���Ϣ
		std::vector<NodeInfo> nodeList_;
		std::map<std::string, int> nodeMap_;
		// ���ݿ���Ϣ
		std::vector<ChunkInfo> chunkList_;
		std::map<std::string, int> chunkMap_;
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

		// ��ǰ����ʱ��
		double curTime_;
		// �����ʱ��
		double simTime_;
		// ���沽��
		int simSteps_;
		// ���ɷ�������
		bool simFree_;
		// �طŷ�������
		bool simReplay_;
		// ���沽��
		unsigned stepTime_;
		// ��¼����
		std::string recordName_;
		// ��ʱ��
		Timer timer_;
	};
}

