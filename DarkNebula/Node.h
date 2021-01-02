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

		// ���û�������С��Ĭ��1MB
		virtual void setBufferSize(size_t bytes);
		size_t getBufferSize();

		// ��ȡ��ǰ���沽��
		uint32_t getSimStep() const;
		// ��ȡ��ǰ����ʱ��
		double getSimTime() const;

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
	};
}

