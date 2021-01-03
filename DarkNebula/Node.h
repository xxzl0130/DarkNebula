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

		// ���û�������С��Ĭ��1MB
		virtual void setBufferSize(size_t bytes);
		size_t getBufferSize();

		// ��ȡ��ǰ���沽��
		uint32_t getSimStep() const;
		// ��ȡ��ǰ����ʱ��
		double getSimTime() const;

		// ��ȡ����״̬
		SimState getSimState() const;
		// �Ƿ�Ϊ���ɷ���
		bool isFreeSim() const;
		// �Ƿ�Ϊ�طŷ���
		bool isReplaySim() const;
		// ��ȡ��ǰ����ʱ��
		double getCurTime() const;
		// ��ȡ��ǰ���沽��
		unsigned getCurSteps() const;
		// ��ȡ��¼����
		std::string getRecordName() const;

	protected:
		
		// ��ʼ�����߳�
		void startWorking();
		// ֹͣ�����߳�
		void stopWorking();
		// �����߳�
		virtual void working() = 0;
		// ��ȡָ��
		char* inBufferData() const;
		// ��ȡ�����ַ���
		std::string inString() const;
		// ��ȡͷ
		CommandHeader* inHeader() const;
		// ��ȡͷ������������ָ��
		char* inData() const;
		// ��socket�������ݶ���inBuffer
		int recvMsg(void* socket);
		
		// ����ָ���socket
		void* pubSocket_;
		// ���ջر���socket
		void* subSocket_;
		// socket����
		void* socketContext_;
		// �����߳�
		std::thread* workThread_;
		// �����߳�ֹͣ��־
		bool workStop_;
		// ���������
		char* outBuffer_;
		// ���뻺����
		zmq_msg_t* inBuffer_;
		// ��������С
		size_t bufferSize_;
		// ������
		std::mutex workMutex_;

		/// ������ʱ���ϲ����ͣ�������Ľṹ���ܸ�
		// ���沽��
		uint32_t simSteps_;
		// ��ǰ����ʱ��
		double curTime_;
		// �����ʱ��
		double simTime_;
		// ���ɷ�������
		bool simFree_;
		// ���沽��,ms
		uint32_t stepTime_;
		// ����״̬
		SimState simState_;
		// �ط�״̬
		int replayState_;
		// ��¼����
		std::string recordName_;
	};
}

