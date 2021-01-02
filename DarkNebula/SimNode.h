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
		/**
		 * \brief ������ݿ�
		 * \tparam T ���ݿ�����
		 * \param name ���ݿ����ƣ����ڵ�֮��ͬһ�����ݿ������Ҫ����һ��
		 * \param data ���ݱ�����ÿ������ǰ���ݻ���µ�����
		 * \param write �Ƿ�ӵ�и����ݿ�д��Ȩ��
		 */
		template <typename T>
		void addChunk(const std::string& name,T& data,bool write)
		{
			addChunk(name, &data, sizeof T, write);
		}
		/**
		 * \brief ������ݿ�
		 * \param name ���ݿ����ƣ����ڵ�֮��ͬһ�����ݿ������Ҫ����һ��
		 * \param pData ���ݱ���ָ�룬ÿ������ǰ���ݻ���µ�����
		 * \param size ���ݴ�С
		 * \param write �Ƿ�ӵ�и����ݿ�д��Ȩ��
		 */
		void addChunk(const std::string& name, void* pData, size_t size, bool write);

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
		// �ط�ģʽ����һ���Ļص����������Բ����ã���ʹ���ƽ�����
		void setSimStepBackCallback(SimStepCallback callback);
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
			// ����ָ��
			void* pData = nullptr;
			// ����ָ��
			std::unique_ptr<char[]> buffer = nullptr;
			Chunk(){}
			Chunk(const std::string& n, size_t s, bool write, void* p) :
				name(n), size(s),own(write), init(false),port(0),id(-1),socket(nullptr),pData(p),buffer(new char[size]){}
		};
		
		// �����߳�
		void working() override;
		// ����ע����Ϣ
		bool sendReg();
		// ��ʼ��
		void initSocket();
		// �����ڵ㷢��ָ��
		void send2Admin(int code, const void* data = nullptr, size_t size = 0);
		// �������ڵ�ָ��
		void processAdminCommand();
		// �����ʼ����Ϣ
		void init();
		// ����һ�����ݿ�
		void sendChunk(Chunk& chunk);
		// �����Լ�����Ҫ���������ݿ�
		void sendChunks();
		// ���������ݸ��µ��û�������
		void copyChunks();

	private:
		SimEventCallback initCallback_, startCallback_, pauseCallback_, stopCallback_;
		SimStepCallback simStepCallback_, replayStepCallback_, replayStepBackCallback_;

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
		std::vector<zmq_pollitem_t> pollitems_;
		// ���ݿ��б�
		std::vector<Chunk> chunkList_;
		// ���ݿ鼯��
		std::set<std::string> chunkSet_;
		// ���ڵ�id
		int id_;
		// ���ٽڵ���
		std::mutex slowMutex_;
		// ���ٽڵ��߳�
		std::thread* slowThread_;
		// ����������
		std::atomic_bool slowRunning_;
	};
}

