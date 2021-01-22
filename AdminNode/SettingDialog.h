#pragma once

#include <QDialog>
#include <QSettings>
namespace Ui { class SettingDialog; };

class SettingDialog : public QDialog
{
	Q_OBJECT

public:
	SettingDialog(QSettings* s, QWidget *parent = Q_NULLPTR);
	~SettingDialog();

public slots:
	void accept() override;
	void reject() override;
	void open() override;

private slots:
	void deleteRecord();

signals:
	void sendSimStep(unsigned step);
	void sendSimTime(unsigned time);
	void sendSimSpeed(unsigned speed);
	void sendFreeSim(bool free);
	void sendPort(unsigned recv, unsigned send);
	void removeRecord(const QString& name);
	void setRecord(bool enable, const QString& name);
	void setReplay(bool enable, const QString name);
	void setAutoRecord(bool enable);

private:
	Ui::SettingDialog *ui;
	QSettings* settings_;
};
