#pragma once

#include <QObject>
#include <QSettings>
#include <string>
#include <QThread>
#include <DarkNebula/SimNode.h>

// ʾ���ķ������ݽṹ
struct SimData
{
	long long data;
};

class SimNode : public QObject, public dn::SimNode
{
	Q_OBJECT

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
	 * \param parent ������
	 */
	SimNode(const std::string& nodeName, const std::string& nodeIP = "127.0.0.1", uint16_t chunkPort = 10000, bool slowNode = false,
		const std::string& adminIP = "127.0.0.1", uint16_t adminRecvPort = dn::ADMIN_RECEIVE_PORT, uint16_t adminSendPort = dn::ADMIN_SEND_PORT,
		QObject* parent = nullptr);
	virtual ~SimNode();

signals:
	// ��ʼ������ź�
	void simInit();

	// ��ʼ�ź�
	void simStart();

	// ��ͣ�ź�
	void simPause();
	
	/**
	 * \brief �����ƽ�һ�����ź�
	 * \param data ����
	 * \param simTime ʱ��
	 * \param simStep ����
	 */
	void simStep(const SimData& data, double simTime, uint32_t simStep);

	// ����������ź�
	void simStop();

protected:
	/// ��������е���غ����������������߳�
	// �����ʼ���ص�����������ʵ��
	void simInitFunc();
	// ���濪ʼ�������ɲ�ʵ��
	void simStartFunc();
	// ������ͣ�������ɲ�ʵ��
	void simPauseFunc();
	// ����ֹͣ�������ɲ�ʵ��
	void simStopFunc();
	// �����ƽ�����������ʵ��
	void simStepFunc(uint32_t steps, double time);
	// �ط�ģʽ����һ���ĺ��������Բ�ʵ�֣���ʹ���ƽ�����
	void replayStepBackFunc(uint32_t steps, double time);
	// �ط��ƽ���������ʵ��ʱ�����طŽ�������ͨ���溯��
	void replayStepFunc(uint32_t steps, double time);

private:
	// ʵ�ʵ����ݶ���
	SimData data_;
};
