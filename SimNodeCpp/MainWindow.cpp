#include "MainWindow.h"

// 配置字段
constexpr auto INI_NODE_IP = "/node/ip";
constexpr auto INI_NODE_CHUNK = "/node/chunk";
constexpr auto INI_NODE_SLOW = "/node/slow";
constexpr auto INI_ADMIN_IP = "/admin/ip";
constexpr auto INI_ADMIN_RECV = "/admin/recv";
constexpr auto INI_ADMIN_SEND = "/admin/send";

// 以下根据项目不同进行修改
constexpr auto NODE_NAME = "SimNode";
constexpr auto INI_FILENAME = "SimNode.ini";
// 配置项默认值，对应修改
constexpr auto NODE_IP_VALUE = "127.0.0.1";
constexpr auto NODE_CHUNK_VALUE = 33333; // 如果节点运行在同一台电脑上，chunk port要间隔足够的距离
constexpr auto NODE_SLOW_VALUE = false;
constexpr auto ADMIN_IP_VALUE = "127.0.0.1";
constexpr auto ADMIN_RECV_VALUE = dn::ADMIN_RECEIVE_PORT;
constexpr auto ADMIN_SEND_VALUE = dn::ADMIN_SEND_PORT;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent),
	ui(new Ui::MainWindowClass),
	settings_(new QSettings(INI_FILENAME, QSettings::IniFormat))
{
    ui->setupUi(this);
	simNode_ = new SimNode(NODE_NAME,
		settings_->value(INI_NODE_IP, NODE_IP_VALUE).toString().toStdString(),
		settings_->value(INI_NODE_CHUNK, NODE_CHUNK_VALUE).toUInt(),
		settings_->value(INI_NODE_SLOW, NODE_SLOW_VALUE).toBool(),
		settings_->value(INI_ADMIN_IP, ADMIN_IP_VALUE).toString().toStdString(),
		settings_->value(INI_ADMIN_RECV, ADMIN_RECV_VALUE).toUInt(),
		settings_->value(INI_ADMIN_SEND, ADMIN_SEND_VALUE).toUInt()
		);

	QObject::connect(this->simNode_, &SimNode::simInit, this, &MainWindow::simInit, Qt::QueuedConnection);
	QObject::connect(this->simNode_, &SimNode::simStart, this, &MainWindow::simStart, Qt::QueuedConnection);
	QObject::connect(this->simNode_, &SimNode::simPause, this, &MainWindow::simPause, Qt::QueuedConnection);
	QObject::connect(this->simNode_, &SimNode::simStop, this, &MainWindow::simStop, Qt::QueuedConnection);
	QObject::connect(this->simNode_, &SimNode::simStep, this, &MainWindow::simStep, Qt::QueuedConnection);
	QObject::connect(this->ui->regPushButton, &QPushButton::clicked, [this]() {simNode_->regIn(); });
}

void MainWindow::simInit()
{
	this->ui->stateLabel->setText(u8"初始化");
}

void MainWindow::simStart()
{
	this->ui->stateLabel->setText(u8"运行");
	this->ui->regPushButton->setEnabled(false);
}

void MainWindow::simPause()
{
	this->ui->stateLabel->setText(u8"暂停");
}

void MainWindow::simStep(const SimData& data, double simTime, uint32_t simStep)
{
	this->ui->stateLabel->setText(u8"运行");
	this->ui->timeLabel->setText(QString::number(simTime));
	this->ui->stepsLabel->setText(QString::number(simStep));
	this->ui->valueLabel->setText(QString::number(data.data));
}

void MainWindow::simStop()
{
	this->ui->stateLabel->setText(u8"停止");
	this->ui->regPushButton->setEnabled(true);
}
