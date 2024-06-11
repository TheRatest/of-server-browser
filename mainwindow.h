#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include "querymaster.h"
#include "configmaster.h"
#include <QTimer>
#include <QSystemTrayIcon>
#include <QLocalServer>
#include <QLocalSocket>
#include <QQueue>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

	// public bc this needs to be accessed by the options dialog
	QueryMaster* m_QueryMaster;
	ConfigMaster m_Config;

	QSystemTrayIcon m_tray;
	QMenu* m_menuTray;
	QIcon* m_iconTray;
	QAction* m_actTrayShow = nullptr;
	QAction* m_actTrayConnectBookmarked = nullptr;
	QAction* m_actTrayReconnect = nullptr;
	QAction* m_actTrayConnectPopulated = nullptr;
	QAction* m_actTrayToggleNotifications = nullptr;
	QAction* m_actTrayOptions = nullptr;
	QAction* m_actTrayInfo = nullptr;
	QAction* m_actTrayQuit = nullptr;

	// to not open the app multiple times
	QLocalServer* m_serverLocal;

	// need to be accessed from the options dialog
	QTimer m_timerAutoRefresh;
	QTimer m_timerFullAutoRefresh;
	QTimer m_timerNotificationsCooldown;

	void resizeEvent(QResizeEvent* pEvent);
	void closeEvent(QCloseEvent * pEvent);

	void ApplyConfig();
public slots:
	void OnServerReady(ServerInfo*);
	void OnServerUpdated(ServerInfo*);
	void OnServerNeedsRemoval(ServerInfo*);
	void OnStyleChanged(QString&);
	void OnServerListSectionResized(int, int, int);

	void OnLocalServerNewConnection();
private slots:
	void OnTrayIconActivated(QSystemTrayIcon::ActivationReason);
	void OnActionShowTriggered();
	void OnActionToggleNotificationsTriggered();
	void OnActionQuitTriggered();
	// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	void DelayedRefreshButtonEanble();

    void on_buttonViewRules_clicked();

    void on_actionOptions_triggered();

	void on_buttonRefresh_clicked();

	void on_tableServers_itemSelectionChanged();

	void on_tableServers_itemDoubleClicked(QTableWidgetItem *item);

	void on_buttonQuickRefresh_clicked();

	void on_buttonConnect_clicked();

	void on_checkAutoRefresh_stateChanged(int arg1);

	void OnTimerAutoRefresh();
	void OnTimerFullAutoRefresh();
	void OnTimerNotificationCooldown();
	void on_spinMaxLatency_textChanged(const QString &arg1);

	void on_spinGameVersion_textChanged(const QString &arg1);

	void on_inputMap_textChanged(const QString &arg1);

	void on_inputTags_textChanged(const QString &arg1);

	void on_comboTags_currentTextChanged(const QString &arg1);

	void on_checkHideEmpty_stateChanged(int arg1);

	void on_checkHidePassworded_stateChanged(int arg1);

	void on_checkHideInsecure_stateChanged(int arg1);

	void on_actionInfo_triggered();

	void on_actionRefresh_selected_server_triggered();

	void on_actionRefresh_all_servers_triggered();

	void on_actionReload_server_list_triggered();

	void on_actionReset_config_triggered();

	void on_inputFilterName_textChanged(const QString &arg1);
	void on_checkFavoritedOnly_stateChanged(int arg1);

	void on_buttonQueryIP_clicked();

	void on_actionCopy_connect_link_to_clipboard_triggered();

	void on_actionConnect_triggered();

	void on_actionReconnectLast_triggered();
	void on_actionClear_cache_triggered();

	void on_actionRequest_server_rules_triggered();

	void on_actionConnect_to_bookmarked_None_triggered();

	void on_actionConnect_to_populated_triggered();

	void on_actionFavorite_selected_server_triggered();

	void on_actionBookmark_selected_server_triggered();

private:
    Ui::MainWindow *ui;

	bool m_bNotificationsOnCooldown = false;

    QIcon m_iconVAC;
    QIcon m_iconPassworded;

	ServerInfo* m_pPopulatedServer = nullptr;
	void DisplayFilteredServers();
	void ConnectToSelectedServer();
	void UpdateConnectActions();
	QQueue<ServerInfo*> m_aQueuedNotifServers;
	void MakeNotification();
	QString MakeConnectURL(ServerInfo* pServer, bool bSteamURLOnly = false);
	QString MakeConnectURL(quint32 iAddr, quint16 iPort, bool bSteamURLOnly = false);

    bool CheckInternetConnection();
};
#endif // MAINWINDOW_H
