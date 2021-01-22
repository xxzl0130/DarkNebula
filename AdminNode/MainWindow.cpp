#include "MainWindow.h"
#include <QIcon>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include "CommonHeader.h"

MainWindow::MainWindow(QWidget *parent):
	QMainWindow(parent),
    ui(new Ui::MainWindowClass),
	adminNode_(nullptr),
	settings_(new QSettings(IniFilename,QSettings::IniFormat,this)),
	nodeIcon_(":/MainWindow/Resource/node.png"),
	dataIcon_(":/MainWindow/Resource/data.png"),
	settingDialog_(new SettingDialog(settings_, this)),
	autoRecord_(false)
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
		free ? QString(u8"自由仿真") : QString(u8"仿真%1秒").arg(this->adminNode_->getSimEndTime()));
	settings_->setValue(IniFreeSim, free);
}

void MainWindow::setPort(unsigned recv, unsigned send)
{
	if (this->adminNode_->getSimState() >= dn::SimRun)
		return;
	adminNode_->setReceivePort(recv);
	adminNode_->setSendPort(send);
	settings_->setValue(IniAdminRecvPort, recv);
	settings_->setValue(IniAdminSendPort, send);
}

void MainWindow::setRecord(bool enable, const QString& name)
{
	if (this->adminNode_->getSimState() >= dn::SimRun)
		return;
	this->adminNode_->setRecord(enable, name.toStdString());
	if (enable)
	{
		this->ui->replayStateLineEdit->setText(u8"录制" + name);
		auto list = this->settings_->value(IniRecordList).toStringList();
		list.append(name);
		this->settings_->setValue(IniRecordList, list);
	}
	else
	{
		this->ui->replayStateLineEdit->setText(u8"无");
	}
}

void MainWindow::setReplay(bool enable, const QString& name)
{
	if (this->adminNode_->getSimState() >= dn::SimRun)
		return;
	this->adminNode_->setReplay(enable, name.toStdString());
	if (enable)
	{
		this->ui->replayStateLineEdit->setText(u8"重播" + name);
	}
	else
	{
		this->ui->replayStateLineEdit->setText(u8"无");
	}
}

void MainWindow::setAutoRecord(bool enable)
{
	if (this->adminNode_->getSimState() >= dn::SimRun)
		return;
	this->autoRecord_ = enable;
}

void MainWindow::removeRecord(const QString& name)
{
	if (this->adminNode_->getSimState() >= dn::SimRun)
		return;
	this->adminNode_->removeRecord(name.toStdString());
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
		this,
		&MainWindow::nodeError,
		this,
		&MainWindow::nodeErrorCallback
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
	/*QObject::connect(
		this->ui->replayAction,
		&QAction::triggered,
		this,
		&MainWindow::setReplay
	);
	QObject::connect(
		this->ui->recordAction,
		&QAction::triggered,
		this,
		&MainWindow::setRecord
	);*/
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
	QObject::connect(
		this->settingDialog_,
		&SettingDialog::setReplay,
		this,
		&MainWindow::setReplay
	);
	QObject::connect(
		this->settingDialog_,
		&SettingDialog::setRecord,
		this,
		&MainWindow::setRecord
	);
	QObject::connect(
		this->settingDialog_,
		&SettingDialog::setAutoRecord,
		this,
		&MainWindow::setAutoRecord
	);
}

void MainWindow::initNodeTree()
{
	QStringList header;
	header << u8"节点" << u8"IP" << u8"初始化" << u8"步数" << u8"读写";
	this->ui->nodeTree->clear();
	this->ui->nodeTree->setHeaderLabels(header);
	this->ui->nodeTree->header()->setSectionResizeMode(0,QHeaderView::Stretch);
	this->ui->nodeTree->header()->setSectionResizeMode(1,QHeaderView::Stretch);
	this->ui->nodeTree->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
	this->ui->nodeTree->header()->setSectionResizeMode(3,QHeaderView::ResizeToContents);
	this->ui->nodeTree->header()->setSectionResizeMode(4,QHeaderView::ResizeToContents);
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
			settings_->value(IniAdminSendPort, IniAdminSendPortDefault).toUInt());
	}
	setSimStep(settings_->value(IniSimStep, IniSimStepDefault).toUInt());
	setSimTime(settings_->value(IniSimTime, IniSimTimeDefault).toUInt());
	setSimSpeed(settings_->value(IniSimSpeed, IniSimSpeedDefault).toUInt());
	setFreeSim(settings_->value(IniFreeSim, IniFreeSimDefault).toBool());
	this->adminNode_->setRegisterCallback([&](unsigned id) {emit nodeRegister(id); });
	this->adminNode_->setAdvanceCallback([&](unsigned id) {emit nodeAdvance(id); });
	this->adminNode_->setInitOverCallback([&](unsigned id) {emit nodeInitOver(id); });
	this->adminNode_->setErrorCallback([&](unsigned id) {emit nodeError(id); });
	updateNodeTree();
}

QTreeWidgetItem* MainWindow::createNode(UINT id) const
{
	QStringList data;
	const auto& node = this->adminNode_->getNodeList()[id];
	data << QString::fromUtf8(node.name.c_str())
		<< QString::fromUtf8(node.ip.c_str())
		<< QString(node.init ? u8"√" : "")
		<< QString::number(node.steps)
		<< "";
	auto* item = new QTreeWidgetItem(data);
	item->setIcon(0, nodeIcon_);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	return item;
}

QList<QTreeWidgetItem*> MainWindow::createChunkList(UINT nodeId) const
{
	QList<QTreeWidgetItem*> list;
	for(const auto& it : this->adminNode_->getNodeList()[nodeId].chunks)
	{
		list.append(createChunk(it));
	}
	return list;
}

QTreeWidgetItem* MainWindow::createChunk(const dn::NodeChunks& chunk) const
{
	QStringList data;
	const auto& chunkInfo = this->adminNode_->getChunkList()[chunk.first];
	data << QString::fromUtf8(chunkInfo.first.c_str()) << QString::fromUtf8(chunkInfo.second.c_str())
		<< "" << "" << QString(chunk.second ? u8"写" : u8"读");
	auto* item = new QTreeWidgetItem(data);
	item->setIcon(0, dataIcon_);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	return item;
}

void MainWindow::searchNodes()
{
	if(adminNode_->getSimState() == dn::SimRun || adminNode_->getSimState() == dn::SimPause)
	{
		QMessageBox::warning(this, u8"错误", u8"仿真运行中不可搜索节点！");
		return;
	}
	this->adminNode_->searchNode();
	this->ui->simStateLineEdit->setText(u8"搜索节点");
}

void MainWindow::initNodes()
{
	if (adminNode_->getSimState() == dn::SimRun || adminNode_->getSimState() == dn::SimPause)
	{
		QMessageBox::warning(this, u8"错误", u8"仿真运行中不可初始化节点！");
		return;
	}
	if(autoRecord_)
	{
		this->setRecord(true, QDateTime::currentDateTime().toString("yyyy.MM.dd.hh.mm.ss"));
	}
	this->adminNode_->initSim();
	this->ui->simStateLineEdit->setText(u8"初始化中");
}

void MainWindow::startSim()
{
	if (adminNode_->getSimState() != dn::SimInit && adminNode_->getSimState() != dn::SimPause)
	{
		QMessageBox::warning(this, u8"错误", u8"请先初始化节点或暂停仿真！");
		return;
	}
	this->adminNode_->startSim();
	this->ui->simStateLineEdit->setText(u8"运行中");
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
		QMessageBox::warning(this, u8"错误", u8"请先运行仿真！");
		return;
	}
	this->adminNode_->pauseSim();
	this->ui->simStateLineEdit->setText(u8"暂停中");
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
		QMessageBox::warning(this, u8"错误", u8"请先运行仿真！");
		return;
	}
	this->adminNode_->stopSim();
	this->ui->simStateLineEdit->setText(u8"停止");

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
	if(this->adminNode_->getNodeList()[node].init)
		item->setData(2, Qt::DisplayRole, u8"√");
	else
		item->setData(2, Qt::DisplayRole, u8" ");
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
			this->ui->simStateLineEdit->setText(u8"运行中");
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
	item->setData(3, Qt::DisplayRole, QString::number(step));
}

void MainWindow::nodeErrorCallback(int node)
{
	auto* item = this->ui->nodeTree->topLevelItem(node);
	if (item == nullptr)
		return;
	auto code = this->adminNode_->getNodeList()[node].errorCode;
	QString errStr = "";
	switch (code)
	{
	case dn::ERR_NOP:
		break;
	case dn::ERR_SOCKET:
		errStr = u8"ERR_SOCKET";
		break;
	case dn::ERR_INFO:
		errStr = u8"ERR_INFO";
		break;
	case dn::ERR_FILE_READ:
	case dn::ERR_FILE_WRITE:
		errStr = u8"ERR_FILE";
		break;
	}
	item->setData(2, Qt::DisplayRole, errStr);
}
