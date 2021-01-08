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
	// 设置仿真步长，单位ms
    void setSimStep(unsigned step);
	// 设置仿真时间，单位s
    void setSimTime(unsigned time);
	// 设置仿真速度，单位%
    void setSimSpeed(unsigned speed);
	// 设置自由仿真
    void setFreeSim(bool free = true);
	// 设置端口
    void setPort(unsigned admin, unsigned node);

private:
	// 链接信号槽
    void connect();
    // 初始化节点表格
    void initNodeTree();
    // 更新节点表格
    void updateNodeTree();
	// 初始化管理节点
    void initAdminNode();
	// 创建Node节点
    QTreeWidgetItem* createNode(UINT id) const;
	// 创建DSM列表
    QList<QTreeWidgetItem*> createDSMList(UINT nodeId) const;
	// 创建DSM节点
    QTreeWidgetItem* createDSM(const std::pair<UINT, bool> dsm) const;

private slots:
	// 搜索节点
    void searchNodes();
	// 初始化节点
    void initNodes();
	// 开始仿真
    void startSim();
	// 暂停仿真
    void pauseSim();
	// 停止仿真
    void stopSim();
	// 仿真加速
    void speedUpSim();
	// 仿真减速
    void speedDownSim();

	// 节点注册回调函数
    void nodeRegisterCallback(unsigned node);
	// 节点初始化完成回调函数
    void nodeInitOverCallback(unsigned node);
	// 节点推进一步回调函数
    void nodeAdvanceCallback(unsigned node);
	// 整体推进一步回调函数
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

