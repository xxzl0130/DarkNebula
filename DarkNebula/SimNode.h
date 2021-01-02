#pragma once
#include "DarkNebulaGlobal.h"

#include "Node.h"

namespace dn
{
	// ����ָ��ص�����
	typedef std::function<void(void)> SimEventCallback;
	// �����ƽ��ص�����
	typedef std::function<void(uint32_t,double)> SimStepCallback;
	
	class DN_EXPORT SimNode : public Node
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
		virtual ~SimNode();
		
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

	private:
		// ���ݿ鶨��
		struct Chunk
		{
			// ����
			std::string name;
			// ����
			size_t size = 0;
			// ����ָ��
			void* pData = nullptr;
			// ����Ȩ��
			bool own = false;
			// ��ʼ��
			bool init = false;
			// �˿�
			uint16_t port = 0;
			// ���
			int id = 0;
			// socketָ��
			void* socket = nullptr;
		};
		
		// �����߳�
		void working() override;
		// ����ע����Ϣ
		bool sendReg();
		// ��ʼ��
		void initSocket();
		// �����ڵ㷢��ָ��
		void send2Admin(int code, const char* data = nullptr, size_t size = 0);
		// �������ڵ�ָ��
		void processAdminCommand();
		// �����ʼ����Ϣ
		void init();
		// ����һ�����ݿ�
		void sendChunk(Chunk& chunk);

	private:
		SimEventCallback initCallback_, startCallback_, pauseCallback_, stopCallback_;
		SimStepCallback simStepCallback_, replayStepCallback_;

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
		
		// �����б�
		zmq_pollitem_t* pollitems_;
		// ��������
		int pollCount_;
		// ���ݿ��б�
		std::vector<Chunk> chunks_;
		// ���ڵ�id
		int id_;
	};
}

