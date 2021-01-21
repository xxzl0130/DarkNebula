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
		// ��ʼ��
		bool init = false;
		// ���ٽڵ㣬������֤����ÿһ������
		bool slow = false;
		// ������
		uint16_t errorCode = ERR_NOP;
		// ���������ݿ�
		std::vector<NodeChunks> chunks;
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

		// ���û�������С
		virtual void setBufferSize(size_t bytes) override;
		
		// ��ȡ�ڵ���Ŀ
		size_t getNodeCount() const;
		// ��ȡ�ڵ���Ϣ
		const std::vector<NodeInfo>& getNodeList() const;

		// ��ȡ���ݿ���Ŀ
		size_t getChunkCount() const;
		// ��ȡ���ݿ���Ϣ
		const std::vector<ChunkInfo>& getChunkList() const;

		// ��������Ѿ�ע��Ľڵ���Ϣ
		void clear();

		/// ��������
		
		// �����Ƿ�Ϊ���ɷ���
		void setFreeSim(bool en);
		// ���÷���ʱ����С��0Ϊ���ɷ��棬�ط�ģʽʱ�����ܴ���¼���ļ�
		void setSimEndTime(double time);
		// ��ȡ����ʱ��
		double getSimEndTime() const;
		// ���÷��沽��
		void setStepTime(unsigned ms);
		// ��������¼�ƣ�����ʱ��Ҫ�ṩ����ļ�¼������طŻ��⣬ȡ��ʱ�����Զ����ûط�
		void setRecord(bool enable, const std::string& name = "");
		/**
		 * \brief �������ûطţ���¼�ƻ��⣬ȡ��ʱ�����Զ�����¼��
		 * \param enable �Ƿ�����
		 * \param name ����ʱ��Ҫ�ṩ����ļ�¼��
		 */
		void setReplay(bool enable, const std::string& name = "");
		// ���÷����ٶȣ�ʵʱΪ1
		void setSimSpeed(double speed = 1);
		// ��ȡ�����ٶ�
		double getSimSpeed() const;
		// ����ٶ�
		void speedUp();
		// �����ٶ�
		void speedDown();

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
		// �����ڵ�
		void searchNode();

		/// �ص�����
		// ��ʼ�����
		void setInitOverCallback(AdminCallback callback);
		// ע��
		void setRegisterCallback(AdminCallback callback);
		// �ƽ�
		void setAdvanceCallback(AdminCallback callback);

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
		// �ڵ���Ϣ
		std::vector<NodeInfo> nodeList_;
		std::map<std::string, int> nodeMap_;
		// �ڵ�ͳ��
		int slowNodeCount_, stepNodeCount_;
		// ���ݿ���Ϣ
		std::vector<ChunkInfo> chunkList_;
		std::map<std::string, int> chunkMap_;
	
		// ��ǰһ�����е�ʱ�䣬�ﵽ�������ƽ���һ��,ms
		double curStepTime_;
		// �������ʣ�ʵʱΪ1
		double simSpeed_;
		// ��ʱ��
		Timer timer_;
		// ������Ҫ���͵�ָ���ͣ��ָֹͣ��Ҫ�ڼ����̷߳��ͣ����������п��ܻ��ͻ
		SimState cmd2send_;

		// �ص�����
		AdminCallback initCallback_, registerCallback_, advanceCallback_;
	};
}

