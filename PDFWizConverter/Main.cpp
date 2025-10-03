#include <QtWidgets/QApplication>
#include <QThreadPool>
#include <NXTheme.h>
#include <NXApplication.h>
#include "UI/PWConverterWidget.h"

int main(int argc, char* argv[])
{
	QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount() >> 1);
	QApplication app(argc, argv);
	nxApp->init();
	nxTheme->setThemeMode(NXThemeType::ThemeMode::Light);
	PWConverterWidget widget;
	widget.show();
	return app.exec();
}

