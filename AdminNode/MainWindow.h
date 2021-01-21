#pragma once

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include <QIcon>
#include <queue>
#include <algorithm>
#include "ui_MainWindow.h"
#include "SettingDialog.h"
#include "DarkNebula/AdminNode.h"

namespace dn {
	struct Chunk;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

public slots:
	// ���÷��沽������λms
    void setSimStep(unsigned step);
	// ���÷���ʱ�䣬��λs
    void setSimTime(unsigned time);
	// ���÷����ٶȣ���λ%
    void setSimSpeed(unsigned speed);
	// �������ɷ���
    void setFreeSim(bool free = true);
	// ���ö˿�
    void setPort(unsigned recv, unsigned send);

private:
	// �����źŲ�
    void connect();
    // ��ʼ���ڵ���
    void initNodeTree();
    // ���½ڵ���
    void updateNodeTree();
	// ��ʼ������ڵ�
    void initAdminNode();
	// ����Node�ڵ�
    QTreeWidgetItem* createNode(UINT id) const;
	// ����Chunk�б�
    QList<QTreeWidgetItem*> createChunkList(UINT nodeId) const;
	// ����Chunk�ڵ�
    QTreeWidgetItem* createChunk(const dn::NodeChunks& chunk) const;

private slots:
	// �����ڵ�
    void searchNodes();
	// ��ʼ���ڵ�
    void initNodes();
	// ��ʼ����
    void startSim();
	// ��ͣ����
    void pauseSim();
	// ֹͣ����
    void stopSim();
	// �������
    void speedUpSim();
	// �������
    void speedDownSim();

	// �ڵ�ע��ص�����
    void nodeRegisterCallback(int node);
	// �ڵ��ʼ����ɻص�����
    void nodeInitOverCallback(int node);
	// �ڵ��ƽ�һ���ص�����
    void nodeAdvanceCallback(int node);
    // �ڵ����ص�����
    void nodeErrorCallback(int node);

signals:
    void nodeRegister(unsigned node);
    void nodeInitOver(unsigned node);
    void nodeAdvance(unsigned node);

private:
    Ui::MainWindowClass* ui;
    dn::AdminNode* adminNode_;
    QSettings* settings_;
    QIcon nodeIcon_, dataIcon_;
    std::queue<std::pair<unsigned, unsigned>> simTimeAvgQueue_;
    SettingDialog* settingDialog_;
};

