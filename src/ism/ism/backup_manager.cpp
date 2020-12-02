#include <ism/backup_manager.hpp>

namespace {
	QString generateAutosaveName(const QDateTime& timestamp)
	{
		return "auto_" + timestamp.toString("yyyy-MM-dd_HH-mm-ss_zzz");
	}

	QString generateManualName(int id, const QString& name)
	{
		return QString("m%1_%2").arg(id, 4, 10, QChar('0')).arg(name);
	}
}

BackupManager::BackupManager(const Config& cfg)
	: mCfg(cfg)
{
	if (QFile::exists(tempFileName()))
		QFile::remove(tempFileName());

	QDir backupDir(cfg.mBackupDir);
	auto autosaves = backupDir.entryList({ "auto_*" }, QDir::Files, QDir::Name);
	mAutoSaves.assign(autosaves.begin(), autosaves.end());

	auto manualSaves = backupDir.entryList({ "m*" }, QDir::Files, QDir::Name);
	mManualSaves.assign(manualSaves.begin(), manualSaves.end());

	if (!mManualSaves.empty())
	{
		mNextManualID = mManualSaves.back().midRef(1, 4).toInt() + 1;
	}

	// when watched file changes, start backup timer (give some time for game to write save)
	connect(&mFSWatcher, &QFileSystemWatcher::fileChanged, [this]() {
		mPendingAutosave = true;
		mReloadRetryTimer.start(100);
	});
	connect(&mReloadRetryTimer, &QTimer::timeout, [this]() {
		if (!autosave())
			return; // try again on next timeout
		mPendingAutosave = false;
		mReloadRetryTimer.stop();
		mFSWatcher.addPath(mCfg.mSavePath); // this is needed, if game deletes file before overwriting
	});
	mFSWatcher.addPath(mCfg.mSavePath);

	// initial autosave
	auto initialAutosaveSuccess = autosave();
	assert(initialAutosaveSuccess);
}

bool BackupManager::restore(const QString& saveName)
{
	if (!QFile::remove(mCfg.mSavePath)) // note: thiw would trigger FS watcher, which would trigger autosave - this is expected (for now, at least)
		return false;
	if (!QFile::copy(QDir(mCfg.mBackupDir).filePath(saveName), mCfg.mSavePath))
		return false;

	// touch file
	{
		QFile save(mCfg.mSavePath);
		save.open(QIODevice::ReadWrite);
		save.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileModificationTime);
	}

	return true;
}

void BackupManager::promoteAutoToManual(const QString& autosave, const QString& manualName)
{
	QDir backupDir(mCfg.mBackupDir);
	auto newSave = generateManualName(mNextManualID++, manualName);
	if (QFile::copy(backupDir.filePath(autosave), backupDir.filePath(newSave)))
	{
		mManualSaves.push_back(newSave);
		emit manualSaveCreated(newSave);
	}
}

QString BackupManager::tempFileName() const
{
	return QDir(mCfg.mBackupDir).filePath("_temp");
}

bool BackupManager::autosave()
{
	auto tempFn = tempFileName();
	assert(!QFile::exists(tempFn));

	auto preModTime = QFileInfo(mCfg.mSavePath).lastModified();
	if (!preModTime.isValid())
		return false;

	if (!QFile::copy(mCfg.mSavePath, tempFn))
	{
		QFile::remove(tempFn); // TODO: not sure whether this is needed...
		return false;
	}

	auto postModTime = QFileInfo(mCfg.mSavePath).lastModified();
	if (postModTime != preModTime)
	{
		QFile::remove(tempFn);
		return false;
	}

	auto autosaveName = generateAutosaveName(postModTime);
	assert(mAutoSaves.empty() || mAutoSaves.back() <= autosaveName);
	if (mAutoSaves.empty() || mAutoSaves.back() < autosaveName)
	{
		auto res = QFile::rename(tempFn, QDir(mCfg.mBackupDir).filePath(autosaveName));
		assert(res);
		mAutoSaves.push_back(autosaveName);

		// delete old autosaves, if needed
		QDir backupDir(mCfg.mBackupDir);
		auto limitName = generateAutosaveName(postModTime.addMSecs(-mCfg.mAutosaveAgeLimit));
		int numDeleted = 0;
		while (mAutoSaves.size() > mCfg.mAutosaveCountLimit + 1 && mAutoSaves.front() < limitName)
		{
			QFile::remove(backupDir.filePath(mAutoSaves.front()));
			mAutoSaves.pop_front();
			++numDeleted;
		}

		emit autosaveHappened(autosaveName, numDeleted);
	}
	else
	{
		QFile::remove(tempFn); // this save already exists
	}
	return true;
}
