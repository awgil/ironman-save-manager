#include <ism/main_window.hpp>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

MainWidget::MainWidget(const Config& cfg)
	: mBackupMgr(cfg)
{
	// autosave order is reversed (new on top)
	auto* autoSaves = new QListWidget;
	for (auto i = mBackupMgr.autoSaves().rbegin(); i != mBackupMgr.autoSaves().rend(); ++i)
		autoSaves->addItem(*i);
	connect(&mBackupMgr, &BackupManager::autosaveHappened, [autoSaves](const QString& newSave, int numDeleted) {
		autoSaves->insertItem(0, newSave);
		while (numDeleted--)
		{
			delete autoSaves->takeItem(autoSaves->count() - 1);
		}
	});
	connect(autoSaves, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item) {
		auto manualName = QInputDialog::getText(this, "Define save name", "Name:");
		if (!manualName.isEmpty())
			mBackupMgr.promoteAutoToManual(item->text(), manualName);
	});

	// manual save order is reversed (new on top)
	auto* manualSaves = new QListWidget;
	for (auto i = mBackupMgr.manualSaves().rbegin(); i != mBackupMgr.manualSaves().rend(); ++i)
		manualSaves->addItem(*i);
	connect(&mBackupMgr, &BackupManager::manualSaveCreated, [manualSaves](const QString& newSave) {
		manualSaves->insertItem(0, newSave);
	});
	connect(manualSaves, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item) {
		restore(item->text());
	});
	// TODO: edit (rename), delete

	// quicksave: promote last autosave to manual save (TODO: consider what should happen if new autosave is now pending...)
	auto* quicksave = new QPushButton("Quick Save");
	connect(quicksave, &QPushButton::clicked, [this]() {
		mBackupMgr.promoteAutoToManual(mBackupMgr.autoSaves().back(), "quicksave");
	});

	// quickload: restore last manual save
	auto* quickload = new QPushButton("Quick Load");
	connect(quickload, &QPushButton::clicked, [this]() {
		if (!mBackupMgr.manualSaves().empty())
			mBackupMgr.restore(mBackupMgr.manualSaves().back());
	});

	auto* reconfig = new QPushButton("Configure...");
	connect(reconfig, &QPushButton::clicked, this, &MainWidget::reconfigure);

	auto* layout = new QVBoxLayout;
	layout->addWidget(new QLabel("Autosaves: (double-click item to promote to manual save)"));
	layout->addWidget(autoSaves);
	layout->addWidget(new QLabel("Manual saves: (double-click item to load)"));
	layout->addWidget(manualSaves);
	layout->addWidget(quicksave);
	layout->addWidget(quickload);
	layout->addWidget(reconfig);
	setLayout(layout);
	// TODO: global shortcuts for quicksave / quickload...
}

void MainWidget::restore(const QString& saveName)
{
	if (!mBackupMgr.restore(saveName))
	{
		QMessageBox::critical(this, "Error", "Failed to restore requested save");
	}
}

std::optional<Config> MainWindow::getStartupConfig()
{
	Config cfg;
	QSettings settings;
	cfg.mSavePath = settings.value("cfg/savePath").toString();
	cfg.mBackupDir = settings.value("cfg/backupDir").toString();
	cfg.mAutosaveCountLimit = settings.value("cfg/autosaveCountLimit").toInt();
	cfg.mAutosaveAgeLimit = settings.value("cfg/autosaveAgeLimit").toInt();
	if (QFileInfo(cfg.mSavePath).isFile() && QFileInfo(cfg.mBackupDir).isDir())
		return cfg;

	return ConfigDialog::run(cfg);
}

MainWindow::MainWindow()
{
	QSettings settings;
	restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
	restoreState(settings.value("mainWindow/windowState").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	QSettings settings;
	settings.setValue("mainWindow/geometry", saveGeometry());
	settings.setValue("mainWindow/windowState", saveState());
}

void MainWindow::setConfig(const Config& cfg)
{
	auto* cw = new MainWidget(cfg);
	setCentralWidget(cw);
	connect(cw, &MainWidget::reconfigure, [this, cfg]() {
		if (auto newCfg = ConfigDialog::run(cfg))
			setConfig(*newCfg);
	});

	setWindowTitle("Ironman save manager: " + cfg.mSavePath);

	QSettings settings;
	settings.setValue("cfg/savePath", cfg.mSavePath);
	settings.setValue("cfg/backupDir", cfg.mBackupDir);
	settings.setValue("cfg/autosaveCountLimit", cfg.mAutosaveCountLimit);
	settings.setValue("cfg/autosaveAgeLimit", cfg.mAutosaveAgeLimit);
}
