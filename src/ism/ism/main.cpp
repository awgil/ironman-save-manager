#include <ism/main_window.hpp>
#include <QApplication>
#include <QSettings>

int main(int argc, char* argv[])
{
	QApplication app{ argc, argv };

	// set some values that are used by QSettings
	QCoreApplication::setOrganizationName("dummy");
	QCoreApplication::setOrganizationDomain("dummy.net");
	QCoreApplication::setApplicationName("Ironman save manager");

	auto startupConfig = MainWindow::getStartupConfig();
	if (!startupConfig)
	{
		printf("User decided not to perform initial configuration...\n");
		return 2;
	}

	MainWindow mw;
	mw.setConfig(*startupConfig);
	mw.show();

	// run application!
	return app.exec();
}
