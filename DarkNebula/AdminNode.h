#pragma once
#include "DarkNebulaGlobal.h"

#include "Node.h"
#include "Timer.h"

namespace dn
{
	// ���ݿ���Ϣ�����ƺ͵�ַ
	typedef std::pair<std::string, std::string> ChunkInfo;

	// ÿ���ڵ���������ݿ����Ϣ����ź�����Ȩ
	typedef std::pair<int, bool> NodeChunks;

	// �ص�����������Ϊ�ڵ�/Chunk���
	typedef std::function<void(int)> AdminCallback;

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
		// ���ٽڵ㣬������֤����ÿһ������
		bool slow = false;
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
	
	class DN_EXPORT AdminNode : public Node
	{
	public:
		/**
		 * \brief ���캯��
		 * \param receivePort ���սڵ�ر��Ķ˿�
		 * \param sendPort ����ָ��Ķ˿�
		 */
		AdminNode(uint16_t receivePort = ADMIN_RECEIVE_PORT, uint16_t sendPort = ADMIN_SEND_PORT);
		virtual ~AdminNode();

		/// ��������

		// ���ý��ն˿�
		void setReceivePort(uint16_t port);
		uint16_t getReceivePort() const;

		// ���÷��Ͷ˿�
		void setSendPort(uint16_t port);
		uint16_t getSendPort() const;

		// ��ȡ����״̬
		SimState getSimState() const;
		bool isFreeSim() const;
		bool isReplaySim() const;

		// ���û�������С
		virtual void setBufferSize(size_t bytes) override;
		
		// ��ȡ�ڵ���Ŀ
		size_t getNodeCount() const;
		// ��ȡ�ڵ���Ϣ
		std::vector<NodeInfo> getNodeList() const;

		// ��ȡ���ݿ���Ŀ
		size_t getChunkCount() const;
		// ��ȡ���ݿ���Ϣ
		std::vector<ChunkInfo> getChunkList() const;

		// ��ȡ��ǰ����ʱ��
		double getCurTime() const;
		// ��ȡ��ǰ���沽��
		unsigned getCurSteps() const;
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
		// ���÷����ٶȣ�ʵʱΪ1
		void setSimSpeed(double speed = 1);

		/// ��������
		// ��ʼ������
		void initSim();
		// ��ʼ����
		void startSim();
		// ��ͣ����
		void pauseSim();
		// ֹͣ����
		void stopSim();
		// �ֶ������ƽ�
		void stepForward();
		// �ֶ���������
		void stepBackward();

		/// �ص�����
		// ��ʼ�����
		void onInitOver(AdminCallback callback);
		// ע��
		void onRegister(AdminCallback callback);
		// �ƽ�
		void onAdvance(AdminCallback callback);

	private:
		// ��������
		void working() override;
		// ������Ϣ
		void sendMsg(void* buffer, size_t len);
		// ���������outBuffer��0������
		void sendCommand(int id, int code, size_t size = 0, void const* data = nullptr);

		// ǰ��һ�����ֶ������Զ���
		void stepAdvance();

		// ������нڵ��ʼ���Ƿ����
		bool checkInit();
		// ������нڵ��ƽ��Ƿ����
		bool checkStep();
		// ����ڵ�ע��
		void nodeReg(char* buffer, int len);
		// ����ڵ��ʼ��
		void nodeInit(char* buffer, int len);
		// ����ڵ��ƽ�
		void nodeStep(char* buffer, int len);
		// ��ʱ�¼�
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

		// �����ʱ��
		double simTime_;
		// ���ɷ�������
		bool simFree_;
		// �طŷ�������
		bool simReplay_;
		// ���沽��,ms
		uint32_t stepTime_;
		// ��ǰһ�����е�ʱ�䣬�ﵽ�������ƽ���һ��,ms
		double curStepTime_;
		// �������ʣ�ʵʱΪ1
		double simSpeed_;
		// ��¼����
		std::string recordName_;
		// ��ʱ��
		Timer timer_;

		// �ص�����
		AdminCallback initCallback_, registerCallback_, advanceCallback_;
	};
}

