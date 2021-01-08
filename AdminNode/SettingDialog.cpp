#include "SettingDialog.h"
#include "ui_SettingDialog.h"
#include <QPushButton>
#include <QMessageBox>
#include "CommonHeader.h"

SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent),
	settings_(new QSettings(IniFilename, QSettings::IniFormat, this))
{
	ui = new Ui::SettingDialog();
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setText(u8"确定");
	ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(u8"取消");
	QObject::connect(
		this->ui->buttonBox,
		&QDialogButtonBox::accepted,
		this,
		&SettingDialog::accept
	);
	QObject::connect(
		this->ui->buttonBox,
		&QDialogButtonBox::rejected,
		this,
		&SettingDialog::reject
	);
	QObject::connect(
		this->ui->freeCheckBox,
		&QCheckBox::toggled,
		[&](bool checked) {this->ui->simTimeSpinBox->setEnabled(!checked); }
	);
}

SettingDialog::~SettingDialog()
{
	delete ui;
}

void SettingDialog::accept()
{
	if(this->ui->recvPortSpinBox->value() == this->ui->sendPortSpinBox->value())
	{
		QMessageBox::warning(this, u8"错误端口", u8"端口号不能相同！");
		reject();
		return;
	}
	emit sendSimSpeed(this->ui->simSpeedSpinBox->value());
	if(!this->ui->freeCheckBox->isChecked())
	{
		emit sendSimTime(this->ui->simTimeSpinBox->value());
	}
	emit sendFreeSim(this->ui->freeCheckBox->isChecked());
	emit sendSimStep(this->ui->simStepSpinBox->value());
	emit sendPort(this->ui->recvPortSpinBox->value(), this->ui->sendPortSpinBox->value());
	QDialog::accept();
}

void SettingDialog::reject()
{
	QDialog::reject();
}

void SettingDialog::open()
{
	this->ui->simStepSpinBox->setValue(settings_->value(IniSimStep, IniSimStepDefault).toUInt());
	this->ui->simSpeedSpinBox->setValue(settings_->value(IniSimSpeed, IniSimSpeedDefault).toUInt());
	this->ui->simTimeSpinBox->setValue(settings_->value(IniSimTime, IniSimTimeDefault).toUInt());
	this->ui->freeCheckBox->setChecked(settings_->value(IniFreeSim, IniFreeSimDefault).toBool());
	this->ui->recvPortSpinBox->setValue(settings_->value(IniAdminRecvPort, IniAdminRecvPortDefault).toUInt());
	this->ui->sendPortSpinBox->setValue(settings_->value(IniAdminSendPort, IniAdminSendPortDefault).toUInt());
	QDialog::open();
}
