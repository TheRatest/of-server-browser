#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include "querymaster.h"
#include "configmaster.h"
#include <QTimer>
#include <QSystemTrayIcon>

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

	void resizeEvent(QResizeEvent* pEvent);

	void ApplyConfig();
public slots:
	void OnServerReady(ServerInfo*);
	void OnServerUpdated(ServerInfo*);
	void OnStyleChanged(QString&);
	void OnServerListSectionResized(int, int, int);
private slots:

    void on_buttonViewRules_clicked();

    void on_actionOptions_triggered();

	void on_buttonRefresh_clicked();

	void on_tableServers_itemSelectionChanged();

	void on_tableServers_itemDoubleClicked(QTableWidgetItem *item);

	void on_buttonQuickRefresh_clicked();

	void on_buttonConnect_clicked();

	void on_checkAutoRefresh_stateChanged(int arg1);

	void OnTimerAutoRefresh();
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

	void on_actionBookmark_selected_server_triggered();

	void on_inputFilterName_textChanged(const QString &arg1);
	void on_checkFavoritedOnly_stateChanged(int arg1);

	void on_buttonQueryIP_clicked();

	void on_actionCopy_connect_link_to_clipboard_triggered();

private:
    Ui::MainWindow *ui;

	QTimer timerAutoRefresh;
	void DisplayFilteredServers();
	QString MakeConnectURL(ServerInfo* pServer);
};
#endif // MAINWINDOW_H
