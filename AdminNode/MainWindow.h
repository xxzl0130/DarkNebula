#pragma once

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include <QIcon>
#include <queue>
#include <algorithm>
#include "ui_MainWindow.h"
#include "SettingDialog.h"
#include "DarkNebula/AdminNode.h"

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
    void setPort(unsigned admin, unsigned node);

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
	// ����DSM�б�
    QList<QTreeWidgetItem*> createDSMList(UINT nodeId) const;
	// ����DSM�ڵ�
    QTreeWidgetItem* createDSM(const std::pair<UINT, bool> dsm) const;

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
    void nodeRegisterCallback(unsigned node);
	// �ڵ��ʼ����ɻص�����
    void nodeInitOverCallback(unsigned node);
	// �ڵ��ƽ�һ���ص�����
    void nodeAdvanceCallback(unsigned node);
	// �����ƽ�һ���ص�����
    void onAdvanceCallback(unsigned time);

signals:
    void nodeRegister(unsigned node);
    void nodeInitOver(unsigned node);
    void nodeAdvance(unsigned node);
    void onAdvance(unsigned time);

private:
    Ui::MainWindowClass* ui;
    dn::AdminNode* adminNode_;
    QSettings* settings_;
    QIcon nodeIcon_, dataIcon_;
    std::queue<std::pair<unsigned, unsigned>> simTimeAvgQueue_;
    SettingDialog* settingDialog_;
};

