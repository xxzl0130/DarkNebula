#pragma once

#include <QDialog>
#include <QSettings>
namespace Ui { class SettingDialog; };

class SettingDialog : public QDialog
{
	Q_OBJECT

public:
	SettingDialog(QWidget *parent = Q_NULLPTR);
	~SettingDialog();

public slots:
	void accept() override;
	void reject() override;
	void open() override;

signals:
	void sendSimStep(unsigned step);
	void sendSimTime(unsigned time);
	void sendSimSpeed(unsigned speed);
	void sendFreeSim(bool free);
	void sendPort(unsigned recv, unsigned send);

private:
	Ui::SettingDialog *ui;
	QSettings* settings_;
};
