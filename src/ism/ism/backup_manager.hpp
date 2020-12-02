#pragma once

#include <ism/config.hpp>
#include <QDateTime>
#include <QDir>
#include <QFileSystemWatcher>
#include <QTimer>
#include <deque>

class BackupManager : public QObject
{
	Q_OBJECT;

public:
	BackupManager(const Config& cfg);

	bool restore(const QString& saveName);
	void promoteAutoToManual(const QString& autosave, const QString& manualName);

	const auto& autoSaves() const { return mAutoSaves; }
	const auto& manualSaves() const { return mManualSaves; }

signals:
	void autosaveHappened(const QString& newSave, int numDeleted);
	void manualSaveCreated(const QString& newSave);

private:
	QString tempFileName() const;

	bool autosave();

private:
	Config mCfg;
	std::deque<QString> mAutoSaves;
	std::vector<QString> mManualSaves;
	int mNextManualID = 0;
	bool mPendingAutosave = false; // if false, last autosave corresponds to watched file (both in timestamp and contents); otherwise timer is running
	QFileSystemWatcher mFSWatcher; // watcher for save game modification
	QTimer mReloadRetryTimer; // timer for retrying reloading
};
