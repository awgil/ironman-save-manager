#pragma once

#include <QDialog>
#include <QLineEdit>
#include <optional>

struct Config
{
	QString mSavePath;
	QString mBackupDir;
	int mAutosaveCountLimit = 0; // if num autosaves is bigger than this amount, consider deleting oldest
	int mAutosaveAgeLimit = 0; // (milliseconds) if oldest autosave is older than this amount, consider deleting it
};

class FileSelector : public QWidget
{
	Q_OBJECT;

public:
	FileSelector(const QString& initialValue, bool selectFile);

	QString text() const { return mEditor->text(); }

	bool valid() const;

signals:
	void modified();

private:
	QLineEdit* mEditor = nullptr;
	bool mSelectFile = false;
};

class ConfigDialog : public QDialog
{
	Q_OBJECT;

public:
	ConfigDialog(const Config& cfg);

	Config getConfig() const;

	static std::optional<Config> run(const Config& initial);

private:
	FileSelector* mSavePath = nullptr;
	FileSelector* mBackupDir = nullptr;
	QLineEdit* mCountLimit = nullptr;
	QLineEdit* mAgeLimit = nullptr;
};
