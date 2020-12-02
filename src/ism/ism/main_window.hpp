#pragma once

#include <ism/backup_manager.hpp>
#include <ism/config.hpp>
#include <QMainWindow>

class MainWidget : public QWidget
{
	Q_OBJECT;

public:
	MainWidget(const Config& cfg);

signals:
	void reconfigure();

private:
	void restore(const QString& saveName);

private:
	BackupManager mBackupMgr;
};

class MainWindow : public QMainWindow
{
	Q_OBJECT;

public:
	static std::optional<Config> getStartupConfig();

	MainWindow();

	void closeEvent(QCloseEvent* event) override;

	void setConfig(const Config& cfg);
};
