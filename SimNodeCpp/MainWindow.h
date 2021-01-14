#pragma once

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include "ui_MainWindow.h"
#include "SimNode.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);

private slots:
	// 初始化完成信号
	void simInit();

	// 开始信号
	void simStart();

	// 暂停信号
	void simPause();

	/**
	 * \brief 仿真推进一步的信号
	 * \param data 数据
	 * \param simTime 时间
	 * \param simStep 步数
	 */
	void simStep(const SimData& data, double simTime, uint32_t simStep);

	// 仿真结束的信号
	void simStop();
	
private:
    Ui::MainWindowClass* ui;
    QSettings* settings_;
    SimNode* simNode_;
};
