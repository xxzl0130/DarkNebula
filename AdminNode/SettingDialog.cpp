#include "SettingDialog.h"
#include "ui_SettingDialog.h"
#include <QPushButton>
#include <QMessageBox>
#include <QValidator>
#include "CommonHeader.h"

SettingDialog::SettingDialog(QSettings* s,QWidget *parent)
	: QDialog(parent),
	settings_(s)
{
	ui = new Ui::SettingDialog();
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setText(u8"确定");
	ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(u8"取消");
	this->ui->recordNameLineEdit->setValidator(new QRegExpValidator(QRegExp(R"([\w\.:-_+()]+)")));

	//connect
	{
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
		QObject::connect(
			this->ui->recordCheckBox,
			&QCheckBox::stateChanged,
			[&](int state)
			{
				if (state == Qt::Checked)
					this->ui->replayCheckBox->setCheckState(Qt::Unchecked);
			});
		QObject::connect(
			this->ui->replayCheckBox,
			&QCheckBox::stateChanged,
			[&](int state)
			{
				if (state == Qt::Checked)
					this->ui->recordCheckBox->setCheckState(Qt::Unchecked);
			});
		QObject::connect(
			this->ui->delRecordPushButton,
			&QPushButton::clicked,
			this,
			&SettingDialog::deleteRecord
		);
	}
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
		return;
	}
	if(this->ui->recordCheckBox->isChecked() && !this->ui->autoRecordCheckBox->isChecked() && this->ui->recordNameLineEdit->text().isEmpty())
	{
		QMessageBox::warning(this, u8"错误", u8"请输入录制文件名！");
		return;
	}
	if(this->ui->replayCheckBox->isChecked() && this->ui->recordList->currentRow() < 0)
	{
		QMessageBox::warning(this, u8"错误", u8"请选择录制文件！");
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
	emit setRecord(this->ui->recordCheckBox->isChecked(), this->ui->recordNameLineEdit->text());
	emit setReplay(this->ui->replayCheckBox->isChecked(), this->ui->replayCheckBox->isChecked() ? this->ui->recordList->currentItem()->text() : "");
	emit setAutoRecord(this->ui->recordCheckBox->isChecked() && this->ui->autoRecordCheckBox->isChecked());
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
	this->ui->recordList->clear();
	this->ui->recordList->addItems(settings_->value(IniRecordList).toStringList());
	QDialog::open();
}

void SettingDialog::deleteRecord()
{
	if (this->ui->recordList->currentItem() == nullptr)
		return;
	if(QMessageBox::question(this,u8"确认",u8"确认要删除这个录制记录吗？") == QMessageBox::Yes)
	{
		emit removeRecord(this->ui->recordList->currentItem()->text());
		this->ui->recordList->removeItemWidget(this->ui->recordList->currentItem());
		QStringList list;
		for (auto i = 0; i < this->ui->recordList->count(); ++i)
			list.append(this->ui->recordList->item(i)->text());
		settings_->setValue(IniRecordList, list);
	}
}
