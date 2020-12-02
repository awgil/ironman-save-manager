#include <ism/config.hpp>
#include <QIntValidator>
#include <QFileDialog>
#include <QFormLayout>
#include <QPushButton>

namespace {
	QLineEdit* makeIntEditor(int initialValue)
	{
		auto* editor = new QLineEdit(QString::number(initialValue));
		editor->setValidator(new QIntValidator(0, INT_MAX, editor));
		return editor;
	}
}

FileSelector::FileSelector(const QString& initialValue, bool selectFile)
	: mSelectFile(selectFile)
{
	mEditor = new QLineEdit(initialValue);
	connect(mEditor, &QLineEdit::textChanged, this, &FileSelector::modified);

	auto* browseButton = new QPushButton("...");
	connect(browseButton, &QPushButton::clicked, [this]() {
		auto fn = mSelectFile
			? QFileDialog::getOpenFileName(this, "Select file...", mEditor->text())
			: QFileDialog::getExistingDirectory(this, "Select directory...", mEditor->text());
		if (!fn.isEmpty())
		{
			mEditor->setText(fn);
		}
	});

	auto* layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(mEditor);
	layout->addWidget(browseButton);
	setLayout(layout);
}

bool FileSelector::valid() const
{
	QFileInfo fi(mEditor->text());
	return mSelectFile ? fi.isFile() : fi.isDir();
}

ConfigDialog::ConfigDialog(const Config& cfg)
{
	setWindowTitle("Configure");

	mSavePath = new FileSelector(cfg.mSavePath, true);
	mBackupDir = new FileSelector(cfg.mBackupDir, false);
	mCountLimit = makeIntEditor(cfg.mAutosaveCountLimit);
	mAgeLimit = makeIntEditor(cfg.mAutosaveAgeLimit);

	auto* okButton = new QPushButton("OK");
	okButton->setDefault(true);
	connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

	auto* cancelButton = new QPushButton("Cancel");
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	// ok button should be enabled only if both paths are valid
	auto updateOkState = [this, okButton]() {
		okButton->setEnabled(mSavePath->valid() && mBackupDir->valid());
	};
	updateOkState();
	connect(mSavePath, &FileSelector::modified, updateOkState);
	connect(mBackupDir, &FileSelector::modified, updateOkState);

	auto* buttons = new QHBoxLayout;
	buttons->addWidget(okButton);
	buttons->addWidget(cancelButton);

	auto* layout = new QFormLayout;
	layout->addRow("Save path:", mSavePath);
	layout->addRow("Backup directory:", mBackupDir);
	layout->addRow("Autosave count limit:", mCountLimit);
	layout->addRow("Autosave age limit (milliseconds):", mAgeLimit);
	layout->addRow(buttons);
	setLayout(layout);
}

Config ConfigDialog::getConfig() const
{
	return { mSavePath->text(), mBackupDir->text(), mCountLimit->text().toInt(), mAgeLimit->text().toInt() };
}

std::optional<Config> ConfigDialog::run(const Config& initial)
{
	ConfigDialog dlg(initial);
	if (dlg.exec() == Accepted)
		return dlg.getConfig();
	else
		return {};
}
