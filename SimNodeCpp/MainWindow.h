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
	
private:
    Ui::MainWindowClass* ui;
    QSettings* settings_;
    SimNode* simNode_;
};
