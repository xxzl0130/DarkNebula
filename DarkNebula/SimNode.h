#pragma once
#include <functional>
#include <thread>
#include <string>
#include <cstdint>
#include <mutex>
#include <vector>



#include "DarkNebulaGlobal.h"

namespace dn
{
	// ����ָ��ص�����
	typedef std::function<void(void)> SimEventCallback;
	// �����ƽ��ص�����
	typedef std::function<void(uint32_t,double)> SimStepCallback;
	
	class DN_EXPORT SimNode
	{
	public:
		/**
		 * \brief ���캯��
		 * \param nodeName ���ڵ����ƣ����ڵ�����ҪΨһ
		 * \param nodeIP ���ڵ�IP��ַ
		 * \param chunkPort ���ݿ���ʼ�˿ڣ�ÿ�����ڵ㷢�������ݿ�Ķ˿��ڴ˻��������μ�һ
		 * \param slowNode ���ٽڵ㣬�����ڵ㲻��֤����ÿһ������
		 * \param adminIP ����ڵ�IP
		 * \param adminRecvPort ����ڵ����ָ��Ķ˿�
		 * \param adminSendPort ����ڵ㷢��ָ��Ķ˿�
		 */
		SimNode(const std::string& nodeName, const std::string& nodeIP = "127.0.0.1", uint16_t chunkPort = 10000, bool slowNode = false,
			const std::string& adminIP = "127.0.0.1", uint16_t adminRecvPort = ADMIN_RECEIVE_PORT, uint16_t adminSendPort = ADMIN_SEND_PORT);
		~SimNode();
		
		/// �ڵ�������ã���Ҫ��ע����ǰ����
		// �ڵ�����
		void setNodeName(const std::string& name);
		// �ڵ�IP
		void setNodeIP(const std::string& IP);
		// ���ݿ���ʼ�˿�
		void setChunkPort(uint16_t port);
		// �����Ƿ�Ϊ���ٽڵ�
		void setSlowNode(bool slow);
		// ����ڵ�IP
		void setAdminIP(const std::string& IP);
		// ����ڵ����ָ��Ķ˿�
		void setAdminRecvPort(uint16_t port);
		// ����ڵ㷢��ָ��Ķ˿�
		void setAdminSendPort(uint16_t port);

		// ע�ᣬ֮���޷��޸Ĳ���
		bool regIn();

		/// �ص��������ã�ע��ص����������������߳�
		// �����ʼ���ص�����
		void setInitCallback(SimEventCallback callback);
		// ���濪ʼ�ص��������ɲ�����
		void setStartCallback(SimEventCallback callback);
		// ������ͣ�ص��������ɲ�����
		void setPauseCallback(SimEventCallback callback);
		// ����ֹͣ�ص��������ɲ�����
		void setStopCallback(SimEventCallback callback);
		// �����ƽ�����
		void setSimStepCallback(SimStepCallback callback);
		// �ط��ƽ�������������ʱ�����طŽ�������ͨ���溯��
		void setReplayStepCallback(SimStepCallback callback);

		// ��ȡ��ǰ���沽��
		uint32_t getSimStep() const;
		// ��ȡ��ǰ����ʱ��
		double getSimTime() const;

	private:
		// �����߳�
		void working();
		// ֹͣ
		void stop();
		// ����ע����Ϣ
		bool sendReg();
		// ��ʼ��
		void initSocket();

		// ���ݿ鶨��
		struct Chunk
		{
			// ����
			std::string name;
			// ����
			size_t size = 0;
			// ����Ȩ��
			bool own = false;
			// �˿�
			uint16_t port = 0;
			// socketָ��
			void* socket;
		};

	private:
		SimEventCallback initCallback_, startCallback_, pauseCallback_, stopCallback_;
		SimStepCallback simStepCallback_, replayStepCallback_;

		/// ������ʱ���ϲ����ͣ�������Ľṹ���ܸ�
		// ���沽��
		uint32_t simSteps_;
		// ��ǰ����ʱ��
		double simTime_;

		// �ڵ�����
		std::string nodeName_;
		// �ڵ�IP
		std::string nodeIP_;
		// ���ݿ�IP
		uint16_t chunkPort_;
		// ����ڵ�IP
		std::string adminIP_;
		// ����ڵ���ն˿�
		uint16_t adminRecvPort_;
		// ����ڵ㷢�Ͷ˿�
		uint16_t adminSendPort_;
		// ���ٽڵ�
		bool slowNode_;
		// ������
		bool running_;
		// ͨ���߳�ֹͣ��־
		bool comStop_;
		// ͨ���߳���
		std::mutex workMutex_;
		// ͨ���߳�
		std::thread* workThread_;

		// socket����
		void* socketContext_;
		// �����ڵ㷢��ָ���socket
		void* pubSocket_;
		// �ӹ���ڵ����ָ���socket
		void* subSocket_;
		// ���ݿ��б�
		std::vector<Chunk> chunks_;
	};
}

