#include "mainwindow.h"
#include <QApplication>
#include <QLocalSocket>
#include <QLocalServer>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    MainWindow hWnd;

	QLocalSocket hLocalSocket;
	hLocalSocket.connectToServer(a.applicationName());
	bool bAnotherAppRunning = hLocalSocket.isOpen();
	if(bAnotherAppRunning) {
		hLocalSocket.write("show of-server-browser");
		hLocalSocket.close();
		return 0;
	}
	hLocalSocket.close();

	hWnd.m_serverLocal = new QLocalServer;
	hWnd.m_serverLocal->setSocketOptions(QLocalServer::WorldAccessOption);
	hWnd.m_serverLocal->listen(a.applicationName());

	hWnd.connect(hWnd.m_serverLocal, SIGNAL(newConnection()), &hWnd, SLOT(OnLocalServerNewConnection()));

	hWnd.show();
    return a.exec();
}

