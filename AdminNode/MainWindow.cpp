#include "MainWindow.h"
#include <QIcon>
#include <QMessageBox>
#include "CommonHeader.h"

MainWindow::MainWindow(QWidget *parent):
	QMainWindow(parent),
    ui(new Ui::MainWindowClass),
	adminNode_(nullptr),
	settings_(new QSettings(IniFilename,QSettings::IniFormat,this)),
	nodeIcon_(":/MainWindow/Resource/node.png"),
	dataIcon_(":/MainWindow/Resource/data.png"),
	settingDialog_(new SettingDialog(this))
{
    ui->setupUi(this);
	connect();
	initNodeTree();
	initAdminNode();
}

MainWindow::~MainWindow()
{
	delete adminNode_;
}

void MainWindow::setSimStep(unsigned step)
{
	this->adminNode_->setStepTime(step);
	this->ui->simStepLineEdit->setText(QString::number(step) + "ms");
	settings_->setValue(IniSimStep, step);
}

void MainWindow::setSimTime(unsigned time)
{
	this->adminNode_->setSimEndTime(time);
	settings_->setValue(IniSimTime, time);
}

void MainWindow::setSimSpeed(unsigned speed)
{
	this->adminNode_->setSimSpeed(speed / 100.0);
	this->ui->simSpeedLineEdit->setText(QString::number(speed) + "%");
	settings_->setValue(IniSimSpeed, speed);
}

void MainWindow::setFreeSim(bool free)
{
	this->adminNode_->setFreeSim(free);
	this->ui->simTypeLineEdit->setText(
		free ? QString(u8"���ɷ���") : QString(u8"����%1��").arg(this->adminNode_->getSimTime()));
	settings_->setValue(IniFreeSim, free);
}

void MainWindow::setPort(unsigned recv, unsigned send)
{
	adminNode_->setReceivePort(recv);
	adminNode_->setSendPort(send);
	settings_->setValue(IniAdminRecvPort, recv);
	settings_->setValue(IniAdminSendPort, send);
}

void MainWindow::connect()
{
	QObject::connect(
		this,
		&MainWindow::nodeRegister,
		this,
		&MainWindow::nodeRegisterCallback
	);
	QObject::connect(
		this,
		&MainWindow::nodeAdvance,
		this,
		&MainWindow::nodeAdvanceCallback
	);
	QObject::connect(
		this,
		&MainWindow::nodeInitOver,
		this,
		&MainWindow::nodeInitOverCallback
	);
	QObject::connect(
		this->ui->initAction,
		&QAction::triggered,
		this,
		&MainWindow::initNodes
	);
	QObject::connect(
		this->ui->pauseAction,
		&QAction::triggered,
		this,
		&MainWindow::pauseSim
	);
	QObject::connect(
		this->ui->searchAction,
		&QAction::triggered,
		this,
		&MainWindow::searchNodes
	);
	QObject::connect(
		this->ui->speedDownAction,
		&QAction::triggered,
		this,
		&MainWindow::speedDownSim
	);
	QObject::connect(
		this->ui->speedUpAction,
		&QAction::triggered,
		this,
		&MainWindow::speedUpSim
	);
	QObject::connect(
		this->ui->startAction,
		&QAction::triggered,
		this,
		&MainWindow::startSim
	);
	QObject::connect(
		this->ui->stopAction,
		&QAction::triggered,
		this,
		&MainWindow::stopSim
	);
	QObject::connect(
		this->ui->pauseAction,
		&QAction::triggered,
		this,
		&MainWindow::pauseSim
	);
	QObject::connect(
		this->ui->setAction,
		&QAction::triggered,
		this->settingDialog_,
		&SettingDialog::open
	);
	QObject::connect(
		this->settingDialog_,
		&SettingDialog::sendSimStep,
		this,
		&MainWindow::setSimStep
	);
	QObject::connect(
		this->settingDialog_,
		&SettingDialog::sendSimTime,
		this,
		&MainWindow::setSimTime
	);
	QObject::connect(
		this->settingDialog_,
		&SettingDialog::sendSimSpeed,
		this,
		&MainWindow::setSimSpeed
	);
	QObject::connect(
		this->settingDialog_,
		&SettingDialog::sendFreeSim,
		this,
		&MainWindow::setFreeSim
	);
	QObject::connect(
		this->settingDialog_,
		&SettingDialog::sendPort,
		this,
		&MainWindow::setPort
	);
}

void MainWindow::initNodeTree()
{
	QStringList header;
	header << u8"�ڵ�" << u8"IP" << u8"�˿�" << u8"��ʼ��" << u8"����" << u8"��д";
	this->ui->nodeTree->clear();
	this->ui->nodeTree->setHeaderLabels(header);
	this->ui->nodeTree->header()->setSectionResizeMode(0,QHeaderView::Stretch);
	this->ui->nodeTree->header()->setSectionResizeMode(1,QHeaderView::Stretch);
	this->ui->nodeTree->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
	this->ui->nodeTree->header()->setSectionResizeMode(3,QHeaderView::ResizeToContents);
	this->ui->nodeTree->header()->setSectionResizeMode(4,QHeaderView::ResizeToContents);
	this->ui->nodeTree->header()->setSectionResizeMode(5,QHeaderView::ResizeToContents);
}

void MainWindow::updateNodeTree()
{
	this->initNodeTree();
	auto n = this->adminNode_->getNodeCount();
	for(auto i = 0;i < n;++i)
	{
		auto* node = createNode(i);
		this->ui->nodeTree->addTopLevelItem(node);
		node->addChildren(createChunkList(i));
	}
	this->ui->nodeTree->expandAll();
}

void MainWindow::initAdminNode()
{
	if(adminNode_ == nullptr)
	{
		adminNode_ = new dn::AdminNode(
			settings_->value(IniAdminRecvPort, IniAdminRecvPortDefault).toUInt(),
			settings_->value(IniAdminSendPort, IniAdminSendPort).toUInt());
	}
	setSimStep(settings_->value(IniSimStep, IniSimStepDefault).toUInt());
	setSimTime(settings_->value(IniSimTime, IniSimTimeDefault).toUInt());
	setSimSpeed(settings_->value(IniSimSpeed, IniSimSpeedDefault).toUInt());
	setFreeSim(settings_->value(IniFreeSim, IniFreeSimDefault).toBool());
	this->adminNode_->setRegisterCallback([&](unsigned id) {emit nodeRegister(id); });
	this->adminNode_->setAdvanceCallback([&](unsigned id) {emit nodeAdvance(id); });
	this->adminNode_->setInitOverCallback([&](unsigned id) {emit nodeInitOver(id); });
	updateNodeTree();
}

QTreeWidgetItem* MainWindow::createNode(UINT id) const
{
	QStringList data;
	//TODO
	//data << QString::fromLocal8Bit(this->adminNode_->GetNodeNameFromID(id).c_str())
	//	<< QString::fromLocal8Bit(this->adminNode_->GetNodeIPFromID(id).c_str())
	//	<< QString::number(this->adminNode_->GetNodePortFromID(id))
	//	<< QString(this->adminNode_->GetNodeInitOver(id) ? u8"��" : "")
	//	<< QString::number(this->adminNode_->GetSimNumFromID(id))
	//	<< "";
	auto* item = new QTreeWidgetItem(data);
	item->setIcon(0, nodeIcon_);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	return item;
}

QList<QTreeWidgetItem*> MainWindow::createChunkList(UINT nodeId) const
{
	QList<QTreeWidgetItem*> list;
	//TODO
	return list;
}

QTreeWidgetItem* MainWindow::createChunk(const std::pair<UINT, bool> chunk) const
{
	QStringList dsmData;
	auto id = chunk.first;
	auto own = chunk.second;
	dsmData << QString::fromLocal8Bit(this->adminNode_->getChunkList()[id].first.c_str())
		<< "" << "" << "" << "" << QString(own ? u8"д" : u8"��");
	auto* dsmItem = new QTreeWidgetItem(dsmData);
	dsmItem->setIcon(0, dataIcon_);
	dsmItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	return dsmItem;
}

void MainWindow::searchNodes()
{
	if(adminNode_->getSimState() == dn::SimRun || adminNode_->getSimState() == dn::SimPause)
	{
		QMessageBox::warning(this, u8"����", u8"���������в��������ڵ㣡");
		return;
	}
	this->adminNode_->searchNode();
	this->ui->simStateLineEdit->setText(u8"�����ڵ�");
}

void MainWindow::initNodes()
{
	if (adminNode_->getSimState() == dn::SimRun || adminNode_->getSimState() == dn::SimPause)
	{
		QMessageBox::warning(this, u8"����", u8"���������в��ɳ�ʼ���ڵ㣡");
		return;
	}
	this->adminNode_->initSim();
	this->ui->simStateLineEdit->setText(u8"��ʼ����");
}

void MainWindow::startSim()
{
	if (adminNode_->getSimState() != dn::SimInit && adminNode_->getSimState() != dn::SimPause)
	{
		QMessageBox::warning(this, u8"����", u8"���ȳ�ʼ���ڵ����ͣ���棡");
		return;
	}
	this->adminNode_->startSim();
	this->ui->simStateLineEdit->setText(u8"������");
	if(this->adminNode_->isFreeSim())
	{
		this->ui->progressBar->setMinimum(0);
		this->ui->progressBar->setMaximum(0);
	}
	else
	{
		this->ui->progressBar->setMaximum(this->adminNode_->getSimEndTime() * 1000);
		this->ui->progressBar->setMinimum(0);
		this->ui->progressBar->setValue(0);
	}
	this->simTimeAvgQueue_.swap(std::queue<std::pair<unsigned, unsigned>>());
}

void MainWindow::pauseSim()
{
	if (adminNode_->getSimState() != dn::SimRun && adminNode_->getSimState() != dn::SimPause)
	{
		QMessageBox::warning(this, u8"����", u8"�������з��棡");
		return;
	}
	this->adminNode_->pauseSim();
	this->ui->simStateLineEdit->setText(u8"��ͣ��");
	if (this->adminNode_->isFreeSim())
	{
		this->ui->progressBar->setMaximum(0);
		this->ui->progressBar->setMinimum(0);
		this->ui->progressBar->setValue(0);
	}
}

void MainWindow::stopSim()
{
	if (adminNode_->getSimState() != dn::SimRun && adminNode_->getSimState() != dn::SimPause && adminNode_->getSimState() != dn::SimStop)
	{
		QMessageBox::warning(this, u8"����", u8"�������з��棡");
		return;
	}
	this->adminNode_->stopSim();
	this->ui->simStateLineEdit->setText(u8"ֹͣ");

	this->ui->progressBar->setMaximum(100);
	this->ui->progressBar->setMinimum(0);
	this->ui->progressBar->setValue(100);

}

void MainWindow::speedUpSim()
{
	this->adminNode_->speedUp();
	this->ui->simSpeedLineEdit->setText(QString::number(this->adminNode_->getSimSpeed() * 100.0) + "%");
}

void MainWindow::speedDownSim()
{
	this->adminNode_->speedDown();
	this->ui->simSpeedLineEdit->setText(QString::number(this->adminNode_->getSimSpeed() * 100.0) + "%");
}

void MainWindow::nodeRegisterCallback(int node)
{
	updateNodeTree();
}

void MainWindow::nodeInitOverCallback(int node)
{
	if(node == dn::ALL_NODE)
	{
		for(auto i = 0;i < this->adminNode_->getNodeCount();++i)
		{
			this->nodeInitOver(i);
		}
	}
	auto* item = this->ui->nodeTree->topLevelItem(node);
	if (item == nullptr)
		return;
	item->setData(3, Qt::DisplayRole, u8"��");
}

void MainWindow::nodeAdvanceCallback(int node)
{
	static size_t cnt = 0;
	if (node == dn::ALL_NODE)
	{
		for (auto i = 0; i < this->adminNode_->getNodeCount(); ++i)
		{
			this->nodeAdvanceCallback(i);
		}
		unsigned time = adminNode_->getSimTime() * 1000.0;
		this->simTimeAvgQueue_.push(std::make_pair(time, unsigned(double(clock()) / CLOCKS_PER_SEC * 1000)));
		while (simTimeAvgQueue_.size() > 50)
		{
			simTimeAvgQueue_.pop();
		}
		if (!adminNode_->isFreeSim())
		{
			this->ui->progressBar->setValue(time);
		}
		if (cnt % 10 == 0)
		{
			auto t0 = this->simTimeAvgQueue_.front();
			auto t1 = this->simTimeAvgQueue_.back();
			this->ui->simTrueSpeedLineEdit->setText(QString::number((t1.first - t0.first) / double(t1.second - t0.second) * 100) + "%");
			this->ui->simStateLineEdit->setText(u8"������");
		}
		auto fff = time % 1000;
		auto ss = (time / 1000) % 60;
		auto mm = (time / 1000 / 60) % 60;
		auto hh = time / 1000 / 60 / 60;
		this->ui->simTimeLineEdit->setText(QString("%1:%2:%3.%4").arg(hh, 2, 10, QChar::fromLatin1('0'))
			.arg(mm, 2, 10, QChar::fromLatin1('0'))
			.arg(ss, 2, 10, QChar::fromLatin1('0'))
			.arg(fff, 3, 10, QChar::fromLatin1('0')));
		++cnt;
		if (adminNode_->getSimState() == dn::SimStop)
		{
			this->stopSim();
		}
		return;
	}
	auto* item = this->ui->nodeTree->topLevelItem(node);
	if (item == nullptr)
		return;
	auto step = this->adminNode_->getNodeList()[node].steps;
	item->setData(4, Qt::DisplayRole, QString::number(step));
}
