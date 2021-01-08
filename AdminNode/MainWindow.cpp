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
	this->adminNode_->SetSimStep(step);
	this->ui->simStepLineEdit->setText(QString::number(step) + "ms");
	settings_->setValue(IniSimStep, step);
}

void MainWindow::setSimTime(unsigned time)
{
	this->adminNode_->SetSimEndTime(time);
	settings_->setValue(IniSimTime, time);
}

void MainWindow::setSimSpeed(unsigned speed)
{
	this->adminNode_->SetSimSpeed(speed);
	this->ui->simSpeedLineEdit->setText(QString::number(speed) + "%");
	settings_->setValue(IniSimSpeed, speed);
}

void MainWindow::setFreeSim(bool free)
{
	this->adminNode_->SetFreeSim(free);
	this->ui->simTypeLineEdit->setText(
		free ? QString(u8"自由仿真") : QString(u8"仿真%1秒").arg(this->adminNode_->GetSimEndTime()));
	settings_->setValue(IniFreeSim, free);
}

void MainWindow::setPort(unsigned admin, unsigned node)
{
	adminNode_->SetAdminNodeReceivePort(admin);
	adminNode_->SetSimNodeReceivePort(node);
	settings_->setValue(IniAdminPort, admin);
	settings_->setValue(IniNodePort, node);
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
		&MainWindow::onAdvance,
		this,
		&MainWindow::onAdvanceCallback
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
	header << u8"节点" << u8"IP" << u8"端口" << u8"初始化" << u8"步数" << u8"读写";
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
	auto n = this->adminNode_->GetNodeCount();
	for(auto i = 0;i < n;++i)
	{
		auto* node = createNode(i);
		this->ui->nodeTree->addTopLevelItem(node);
		node->addChildren(createDSMList(i));
	}
	this->ui->nodeTree->expandAll();
}

void MainWindow::initAdminNode()
{
	if(adminNode_ == nullptr)
	{
		adminNode_ = new dn::AdminNode(
			settings_->value(IniAdminPort, IniAdminPortDefault).toUInt(),
			settings_->value(IniNodePort, IniNodePortDefault).toUInt());
	}
	setSimStep(settings_->value(IniSimStep, IniSimStepDefault).toUInt());
	setSimTime(settings_->value(IniSimTime, IniSimTimeDefault).toUInt());
	setSimSpeed(settings_->value(IniSimSpeed, IniSimSpeedDefault).toUInt());
	setFreeSim(settings_->value(IniFreeSim, IniFreeSimDefault).toBool());
	this->adminNode_->SetOnRegister([&](unsigned id) {emit nodeRegister(id); });
	this->adminNode_->SetOnNodeAdvance([&](unsigned id) {emit nodeAdvance(id); });
	this->adminNode_->SetOnInitOver([&](unsigned id) {emit nodeInitOver(id); });
	this->adminNode_->SetOnAdvance([&](unsigned time) {emit onAdvance(time); });
	updateNodeTree();
}

QTreeWidgetItem* MainWindow::createNode(UINT id) const
{
	QStringList data;
	data << QString::fromLocal8Bit(this->adminNode_->GetNodeNameFromID(id).c_str())
		<< QString::fromLocal8Bit(this->adminNode_->GetNodeIPFromID(id).c_str())
		<< QString::number(this->adminNode_->GetNodePortFromID(id))
		<< QString(this->adminNode_->GetNodeInitOver(id) ? u8"√" : "")
		<< QString::number(this->adminNode_->GetSimNumFromID(id))
		<< "";
	auto* item = new QTreeWidgetItem(data);
	item->setIcon(0, nodeIcon_);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	return item;
}

QList<QTreeWidgetItem*> MainWindow::createDSMList(UINT nodeId) const
{
	QList<QTreeWidgetItem*> list;
	const auto& dsmSet = this->adminNode_->GetDsmSetFromID(nodeId);
	for (const auto& dsm : dsmSet)
	{
		list.append(createDSM(dsm));
	}
	return list;
}

QTreeWidgetItem* MainWindow::createDSM(const std::pair<UINT, bool> dsm) const
{
	QStringList dsmData;
	auto id = dsm.first;
	auto own = dsm.second;
	dsmData << QString::fromLocal8Bit(this->adminNode_->GetDsmNameFromID(id).c_str())
		<< "" << "" << "" << "" << QString(own ? u8"写" : u8"读");
	auto* dsmItem = new QTreeWidgetItem(dsmData);
	dsmItem->setIcon(0, dataIcon_);
	dsmItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	return dsmItem;
}

void MainWindow::searchNodes()
{
	if(adminNode_->GetSimState() == SimRun || adminNode_->GetSimState() == SimPause)
	{
		QMessageBox::warning(this, u8"错误", u8"仿真运行中不可搜索节点！");
		return;
	}
	this->adminNode_->SearchNode();
	this->ui->simStateLineEdit->setText(u8"搜索节点");
}

void MainWindow::initNodes()
{
	if (adminNode_->GetSimState() == SimRun || adminNode_->GetSimState() == SimPause)
	{
		QMessageBox::warning(this, u8"错误", u8"仿真运行中不可初始化节点！");
		return;
	}
	this->adminNode_->SimInit();
	this->ui->simStateLineEdit->setText(u8"初始化中");
}

void MainWindow::startSim()
{
	if (adminNode_->GetSimState() != SimInit && adminNode_->GetSimState() != SimPause)
	{
		QMessageBox::warning(this, u8"错误", u8"请先初始化节点或暂停仿真！");
		return;
	}
	this->adminNode_->SimRun();
	this->ui->simStateLineEdit->setText(u8"运行中");
	if(this->adminNode_->IsFreeSim())
	{
		this->ui->progressBar->setMinimum(0);
		this->ui->progressBar->setMaximum(0);
	}
	else
	{
		this->ui->progressBar->setMaximum(this->adminNode_->GetSimEndTime() * 1000);
		this->ui->progressBar->setMinimum(0);
		this->ui->progressBar->setValue(0);
	}
	this->simTimeAvgQueue_.swap(std::queue<std::pair<unsigned, unsigned>>());
}

void MainWindow::pauseSim()
{
	if (adminNode_->GetSimState() != SimRun && adminNode_->GetSimState() != SimPause)
	{
		QMessageBox::warning(this, u8"错误", u8"请先运行仿真！");
		return;
	}
	this->adminNode_->SimPause();
	this->ui->simStateLineEdit->setText(u8"暂停中");
	if (this->adminNode_->IsFreeSim())
	{
		this->ui->progressBar->setMaximum(0);
		this->ui->progressBar->setMinimum(0);
		this->ui->progressBar->setValue(0);
	}
}

void MainWindow::stopSim()
{
	if (adminNode_->GetSimState() != SimRun && adminNode_->GetSimState() != SimPause && adminNode_->GetSimState() != SimStop)
	{
		QMessageBox::warning(this, u8"错误", u8"请先运行仿真！");
		return;
	}
	this->adminNode_->SimStop();
	this->ui->simStateLineEdit->setText(u8"停止");

	this->ui->progressBar->setMaximum(100);
	this->ui->progressBar->setMinimum(0);
	this->ui->progressBar->setValue(100);

}

void MainWindow::speedUpSim()
{
	this->adminNode_->SimSpeedUp();
	this->ui->simSpeedLineEdit->setText(QString::number(this->adminNode_->GetSimSpeed()) + "%");
}

void MainWindow::speedDownSim()
{
	this->adminNode_->SimSlowDown();
	this->ui->simSpeedLineEdit->setText(QString::number(this->adminNode_->GetSimSpeed()) + "%");
}

void MainWindow::nodeRegisterCallback(unsigned node)
{
	updateNodeTree();
}

void MainWindow::nodeInitOverCallback(unsigned node)
{
	if(node == (UINT)-1)
	{
		for(auto i = 0;i < this->adminNode_->GetNodeCount();++i)
		{
			this->nodeInitOver(i);
		}
	}
	auto* item = this->ui->nodeTree->topLevelItem(node);
	if (item == nullptr)
		return;
	item->setData(3, Qt::DisplayRole, u8"√");
}

void MainWindow::nodeAdvanceCallback(unsigned node)
{
	if (node == (UINT)-1)
	{
		for (auto i = 0; i < this->adminNode_->GetNodeCount(); ++i)
		{
			this->nodeAdvanceCallback(i);
		}
	}
	auto* item = this->ui->nodeTree->topLevelItem(node);
	if (item == nullptr)
		return;
	auto step = this->adminNode_->GetSimNumFromID(node);
	item->setData(4, Qt::DisplayRole, QString::number(step));
}

void MainWindow::onAdvanceCallback(unsigned time)
{
	static size_t cnt = 0;
	this->simTimeAvgQueue_.push(std::make_pair(time, unsigned(double(clock()) / CLOCKS_PER_SEC * 1000)));
	while(simTimeAvgQueue_.size() > 50)
	{
		simTimeAvgQueue_.pop();
	}
	if (!adminNode_->IsFreeSim())
	{
		this->ui->progressBar->setValue(time);
	}
	if(cnt % 10 == 0)
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
	if(adminNode_->GetSimState() == SimStop)
	{
		this->stopSim();
	}
}
