#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "rulesdialog.h"
#include "optionsdialog.h"
#include "infodialog.h"
#include "queryipdialog.h"
#include <QDesktopServices>
#include <QPushButton>
#include <QResizeEvent>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_iconVAC.addFile(":/res/icon/secure.png");
    m_iconPassworded.addFile(":/res/icon/passworded.png");

	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	pTable->setColumnHidden(0, true);
    pTable->setColumnWidth(1, 24);
    pTable->setColumnWidth(2, 24);
	pTable->setColumnWidth(3, 230);
	pTable->setColumnWidth(4, 26);
	pTable->setColumnWidth(5, 26);
	pTable->setColumnWidth(6, 26);
	pTable->setColumnWidth(7, 100);
	pTable->setColumnWidth(8, 40);
	pTable->setColumnWidth(9, 250);

	m_QueryMaster = new QueryMaster(this);
	connect(m_QueryMaster, &QueryMaster::ServerIsReady, this, &MainWindow::OnServerReady);
	connect(m_QueryMaster, &QueryMaster::ServerUpdated, this, &MainWindow::OnServerUpdated);
	connect(m_QueryMaster, &QueryMaster::ServerNeedsRemoval, this, &MainWindow::OnServerNeedsRemoval);
	connect(&m_timerAutoRefresh, &QTimer::timeout, this, &MainWindow::OnTimerAutoRefresh);
	connect(&m_timerFullAutoRefresh, &QTimer::timeout, this, &MainWindow::OnTimerFullAutoRefresh);
	connect(&m_timerNotificationsCooldown, &QTimer::timeout, this, &MainWindow::OnTimerNotificationCooldown);

	m_Config.LoadConfig();

	qDebug() << "Cached servers: " << m_Config.m_aCachedServers.size();
	qDebug() << "Favorited servers: " << m_Config.m_aFavoritedServers.size();
	for(auto it = m_Config.m_aCachedServers.begin(); it != m_Config.m_aCachedServers.end(); ++it) {
		m_QueryMaster->LoadCachedServer(*it);
	}
	for(auto it = m_Config.m_aFavoritedServers.begin(); it != m_Config.m_aFavoritedServers.end(); ++it) {
		m_QueryMaster->MakeFavoriteFromAddr(*it);
	}
	if(m_Config.m_aCachedServers.size() == 0)
        m_QueryMaster->QueryMasterServer(m_Config.m_strGameFolder);

	connect(pTable->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(OnServerListSectionResized(int, int, int)));

	m_timerAutoRefresh.setInterval(m_Config.m_flAutoRefreshInterval * 1000.0);
    m_timerFullAutoRefresh.setInterval(m_Config.m_flFullAutoRefreshInterval * 1000.0);
	m_timerNotificationsCooldown.setInterval(m_Config.m_iNotificationCooldownMin * 1000 * 60);

	m_actTrayShow = new QAction;
	m_actTrayConnectBookmarked = new QAction;
	m_actTrayReconnect = new QAction;
	m_actTrayConnectPopulated = new QAction;
	m_actTrayToggleNotifications = new QAction;
	m_actTrayOptions = new QAction;
	m_actTrayInfo = new QAction;
	m_actTrayQuit = new QAction;

	m_actTrayShow->setText("Open");
	m_actTrayReconnect->setText("Reconnect to last");
	if(m_Config.m_bEnableNotifications)
		m_actTrayToggleNotifications->setText("Disable notifications");
	else
		m_actTrayToggleNotifications->setText("Enable notifications");
	m_actTrayOptions->setText("Options");
	m_actTrayInfo->setText("Info");
	m_actTrayQuit->setText("Quit");

	connect(&m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(OnTrayIconActivated(QSystemTrayIcon::ActivationReason)));
	connect(m_actTrayShow, &QAction::triggered, this, &MainWindow::OnActionShowTriggered);
	connect(m_actTrayConnectBookmarked, &QAction::triggered, this, &MainWindow::on_actionConnect_to_bookmarked_None_triggered);
	connect(m_actTrayReconnect, &QAction::triggered, this, &MainWindow::on_actionReconnectLast_triggered);
	connect(m_actTrayConnectPopulated, &QAction::triggered, this, &MainWindow::on_actionConnect_to_populated_triggered);
	connect(m_actTrayToggleNotifications, &QAction::triggered, this, &MainWindow::OnActionToggleNotificationsTriggered);
	connect(m_actTrayOptions, &QAction::triggered, this, &MainWindow::on_actionOptions_triggered);
	connect(m_actTrayInfo, &QAction::triggered, this, &MainWindow::on_actionInfo_triggered);
	connect(m_actTrayQuit, &QAction::triggered, this, &MainWindow::OnActionQuitTriggered);

	m_menuTray = new QMenu;
	m_menuTray->addAction(m_actTrayShow);
	m_menuTray->addAction(m_actTrayConnectBookmarked);
	m_menuTray->addAction(m_actTrayReconnect);
	m_menuTray->addAction(m_actTrayConnectPopulated);
	m_menuTray->addSeparator();
	m_menuTray->addAction(m_actTrayToggleNotifications);
	m_menuTray->addAction(m_actTrayOptions);
	m_menuTray->addAction(m_actTrayInfo);
	m_menuTray->addSeparator();
	m_menuTray->addAction(m_actTrayQuit);

	m_iconTray = new QIcon;
	m_iconTray->addFile(":/res/icon/icon.ico");

	m_tray.setContextMenu(m_menuTray);
	m_tray.setToolTip("of-server-browser");
	m_tray.setObjectName("of-server-browser");
	m_tray.setIcon(*m_iconTray);
	m_tray.setVisible(m_Config.m_bUseTray);

	ApplyConfig();
}

void MainWindow::ApplyConfig() {
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	QCheckBox* pCheckAutoRefresh = this->findChild<QCheckBox*>("checkAutoRefresh");
	QCheckBox* pHideInsecure = this->findChild<QCheckBox*>("checkHideInsecure");
	QCheckBox* pHidePassworded = this->findChild<QCheckBox*>("checkHidePassworded");
	QCheckBox* pHideEmpty = this->findChild<QCheckBox*>("checkHideEmpty");
	QCheckBox* pCheckOnlyFavorited = this->findChild<QCheckBox*>("checkFavoritedOnly");
	QComboBox* pComboTags = this->findChild<QComboBox*>("comboTags");
	QSpinBox* pSpinMaxLatency = this->findChild<QSpinBox*>("spinMaxLatency");
	QSpinBox* pSpinGameVersion = this->findChild<QSpinBox*>("spinGameVersion");
	QLineEdit* pInputMap = this->findChild<QLineEdit*>("inputMap");
	QLineEdit* pInputTags = this->findChild<QLineEdit*>("inputTags");
	QLineEdit* pInputName = this->findChild<QLineEdit*>("inputFilterName");
	// won't change anything due to the initial server list being unfiltered
	//pHideEmpty->setCheckState(m_Config.m_bHideEmptyPlayers ? Qt::Checked : Qt::Unchecked);
	//pHideInsecure->setCheckState(m_Config.m_bHideNoVAC ? Qt::Checked : Qt::Unchecked);
	//pHidePassworded->setCheckState(m_Config.m_bHidePassworded ? Qt::Checked : Qt::Unchecked);
	//pCheckOnlyFavorited->setCheckState(m_Config.m_bDisplayOnlyFavorited ? Qt::Checked : Qt::Unchecked);

	pCheckAutoRefresh->setCheckState(Qt::Unchecked);
	if(m_Config.m_bAutoRefreshServers)
		pCheckAutoRefresh->setCheckState(Qt::Checked);

    if(m_Config.m_bFullAutoRefreshServers) {
        m_timerFullAutoRefresh.setInterval(m_Config.m_flFullAutoRefreshInterval*1000);
        m_timerFullAutoRefresh.start();
    }

	pComboTags->setCurrentIndex(m_Config.m_bExcludeTags ? 1 : 0);
	pSpinMaxLatency->setValue(m_Config.m_iMaxLatency);
	pSpinGameVersion->setValue(m_Config.m_iGameVersion);
	pInputMap->setText(m_Config.m_strFilterMap);
	pInputTags->setText(m_Config.m_strFilterTags);
	pInputName->setText(m_Config.m_strFilterHostname);

	if(m_Config.m_iStyleIndex != 0 && !m_Config.m_strStyle.isEmpty() && m_Config.m_strStyle != "Default") {
		OnStyleChanged(m_Config.m_strStyle);
	}

	for(int i = 0; i < m_Config.m_iColumns; ++i) {
		pTable->setColumnWidth(i, m_Config.m_aColumnSizes[i]);
	}

	for(int i = 0; i < pTable->columnCount(); ++i) {
		m_Config.m_aColumnSizes[i] = pTable->columnWidth(i);
	}

	this->resize(m_Config.m_iMainWindowWidth, m_Config.m_iMainWindowHeight);

	UpdateConnectActions();

    m_QueryMaster->m_strGameFolder = m_Config.m_strGameFolder;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnStyleChanged(QString& strStyleName) {
	if(strStyleName.isEmpty() || strStyleName.startsWith("Default")) {
		this->setStyleSheet("");
		return;
	}

	// i'm NOT gonna use winapi magic to set the color of the title bar to black
	// bool bIsDarkTheme = strStyleName == "Darkeum" || strStyleName == "Combinear" || strStyleName == "ManjaroMix"  || strStyleName == "Adaptic";
	if(strStyleName == "Custom") {
		QFile fileStyle(m_Config.m_strStylePath);
		fileStyle.open(QFile::ReadOnly);
		QString strStyleSheet = QLatin1String(fileStyle.readAll());
		fileStyle.close();
		this->setStyleSheet(strStyleSheet);
	} else {
		QString strStylePath = ":/res/styles/";
		strStylePath.append(strStyleName);
		strStylePath.append(".qss");
		QFile fileStyle(strStylePath);
		fileStyle.open(QFile::ReadOnly);
		QString strStyleSheet = QLatin1String(fileStyle.readAll());
		fileStyle.close();
		this->setStyleSheet(strStyleSheet);
	}
}

void MainWindow::on_buttonViewRules_clicked()
{
	RulesDialog* hRulesWnd = new RulesDialog(this);

	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	QTableWidget* pRulesTable = hRulesWnd->findChild<QTableWidget*>("tableRules");

	for(int i = 0; i < pServer->m_aRules.size(); ++i) {
		pRulesTable->setRowCount(pRulesTable->rowCount()+1);

		QTableWidgetItem* pRuleName = new QTableWidgetItem;
		QTableWidgetItem* pRuleValue = new QTableWidgetItem;

		pRuleName->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		pRuleValue->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		pRuleName->setText(pServer->m_aRules[i].first);
		pRuleValue->setText(pServer->m_aRules[i].second);

		pRulesTable->setItem(i, 0, pRuleName);
		pRulesTable->setItem(i, 1, pRuleValue);
	}

	hRulesWnd->exec();
    delete hRulesWnd;
}

void MainWindow::on_actionOptions_triggered()
{
	OptionsDialog* hOptionsWnd = new OptionsDialog(this);
	hOptionsWnd->exec();

	delete hOptionsWnd;
}

void MainWindow::on_actionInfo_triggered()
{
	InfoDialog* hInfoWnd = new InfoDialog(this);
	hInfoWnd->exec();

	delete hInfoWnd;
}

void MainWindow::DisplayFilteredServers() {
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	QTableWidget* pPlayersTable = this->findChild<QTableWidget*>("tablePlayers");
	pTable->clearContents();
	pTable->setRowCount(0);
	pPlayersTable->clearContents();
	pPlayersTable->setRowCount(0);

	QLabel* pServerListLabel = this->findChild<QLabel*>("labelServers");
	QString strServerList = "Server list (";
	strServerList.append(QString::number(pTable->rowCount()));
	strServerList.append("/");
	strServerList.append(QString::number(m_QueryMaster->m_aServers.size()));
	strServerList.append(" total)");
	pServerListLabel->setText(strServerList);


    for(int i = 0; i < m_QueryMaster->m_aServers.size(); ++i) {
		if(!m_QueryMaster->m_aServers[i]->m_bReady)
			continue;

		ServerInfo* pServer = m_QueryMaster->m_aServers[i];
		QStringList astrFilterTags = m_Config.m_strFilterTags.split(",");

        if(!pServer->m_bCached && m_Config.m_bEnableCaching) {
            m_Config.m_aCachedServers.push_back(std::make_pair(pServer->m_hAddress.toIPv4Address(), pServer->m_iQueryPort));
            pServer->m_bCached = true;
            m_Config.SaveConfig();
        }

        if(pServer->m_bBogusServer || pServer->m_strGameFolder != m_Config.m_strGameFolder)
			continue;

		if(m_Config.m_bHideEmptyPlayers && pServer->m_iPlayers == 0)
			continue;

		if(m_Config.m_bHideNoVAC && !pServer->m_bVAC)
			continue;

		if(m_Config.m_bHidePassworded && pServer->m_bPassworded)
			continue;

		if(m_Config.m_iMaxLatency < pServer->m_iLatencyMs && m_Config.m_iMaxLatency != 0)
			continue;

		if(m_Config.m_iGameVersion != pServer->m_iRevision && m_Config.m_iGameVersion != 0)
			continue;

		if(!pServer->m_strMap.toLower().contains(m_Config.m_strFilterMap.toLower()) && !m_Config.m_strFilterMap.isEmpty())
			continue;

		if(!pServer->m_strName.toLower().contains(m_Config.m_strFilterHostname.toLower()) && m_Config.m_strFilterHostname.length() > 0)
			continue;

		if(m_Config.m_bDisplayOnlyFavorited && !pServer->m_bFavorited)
			continue;

		//QStringList astrServerTags = pServer->m_strTags.split(",");
		for(int i = 0; i < astrFilterTags.size(); ++i) {
			if(m_Config.m_bExcludeTags && pServer->m_strTags.contains(astrFilterTags[i]) && !astrFilterTags[i].isEmpty())
				continue;

			if(!m_Config.m_bExcludeTags && !pServer->m_strTags.contains(astrFilterTags[i]))
				continue;
		}

		OnServerReady(m_QueryMaster->m_aServers[i]);
	}
}

void MainWindow::OnServerReady(ServerInfo* pServer) {
	QLabel* pServerListLabel = this->findChild<QLabel*>("labelServers");
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	QString strServerList = "Server list (";
	strServerList.append(QString::number(pTable->rowCount()+1));
	strServerList.append("/");
	strServerList.append(QString::number(m_QueryMaster->m_aServers.size()));
	strServerList.append(" total)");
	pServerListLabel->setText(strServerList);

	QTableWidgetItem* pItemInternalID = new QTableWidgetItem;
	QTableWidgetItem* pItemPassworded = new QTableWidgetItem;
	QTableWidgetItem* pItemVAC = new QTableWidgetItem;
	QTableWidgetItem* pItemHostname = new QTableWidgetItem;
	QTableWidgetItem* pItemBots = new QTableWidgetItem;
	QTableWidgetItem* pItemPlayers = new QTableWidgetItem;
	QTableWidgetItem* pItemMaxPlayers = new QTableWidgetItem;
	QTableWidgetItem* pItemMap = new QTableWidgetItem;
	QTableWidgetItem* pItemLatency = new QTableWidgetItem;
	QTableWidgetItem* pItemTags = new QTableWidgetItem;

	pItemInternalID->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemPassworded->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemVAC->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemHostname->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemBots->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemPlayers->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemMaxPlayers->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemMap->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemLatency->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	pItemTags->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	pItemInternalID->setData(Qt::DisplayRole, pServer->m_iInternalID);
	if(pServer->m_bPassworded)
        pItemPassworded->setIcon(m_iconPassworded);
	else
		pItemPassworded->setText("");

	if(pServer->m_bVAC)
        pItemVAC->setIcon(m_iconVAC);
	else
		pItemVAC->setText("");
	pItemHostname->setText(pServer->m_strName);
	pItemBots->setData(Qt::DisplayRole, pServer->m_iBots);
	pItemPlayers->setData(Qt::DisplayRole, pServer->m_iPlayers);
	pItemMaxPlayers->setData(Qt::DisplayRole, pServer->m_iMaxPlayers);
	pItemMap->setText(pServer->m_strMap);
	pItemLatency->setData(Qt::DisplayRole, pServer->m_iLatencyMs);
	pItemTags->setText(pServer->m_strTags);

	int iRow;
	int iExistingServer = -1;
	for(int i = 0; i < pTable->rowCount(); ++i) {
		if(pTable->item(i, 0)->data(Qt::DisplayRole) == pServer->m_iInternalID) {
			iExistingServer = i;
			break;
		}
	}
	if(iExistingServer != -1)
		iRow = iExistingServer;
	else {
		iRow = pTable->rowCount();
		pTable->setRowCount(pTable->rowCount()+1);
	}

	pTable->setItem(iRow, 0, pItemInternalID);
	pTable->setItem(pItemInternalID->row(), 1, pItemPassworded);
	pTable->setItem(pItemPassworded->row(), 2, pItemVAC);
	pTable->setItem(pItemVAC->row(), 3, pItemHostname);
	pTable->setItem(pItemHostname->row(), 4, pItemBots);
	pTable->setItem(pItemBots->row(), 5, pItemPlayers);
	pTable->setItem(pItemPlayers->row(), 6, pItemMaxPlayers);
	pTable->setItem(pItemMaxPlayers->row(), 7, pItemMap);
	pTable->setItem(pItemMap->row(), 8, pItemLatency);
	pTable->setItem(pItemLatency->row(), 9, pItemTags);

	if(!m_pPopulatedServer) {
		m_pPopulatedServer = pServer;
		UpdateConnectActions();
	}
	else {
		if(pServer->m_iPlayers - pServer->m_iBots > m_pPopulatedServer->m_iPlayers - m_pPopulatedServer->m_iBots) {
			m_pPopulatedServer = pServer;
			UpdateConnectActions();
		}
	}

    if(!pServer->m_bCached && m_Config.m_bEnableCaching) {
        m_Config.m_aCachedServers.push_back(std::make_pair(pServer->m_hAddress.toIPv4Address(), pServer->m_iQueryPort));
        pServer->m_bCached = true;
        m_Config.SaveConfig();
    }
}

void MainWindow::OnServerUpdated(ServerInfo* pServer) {
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");

	for(int i = 0; i < pTable->rowCount(); ++i) {
		QTableWidgetItem* pInternalID = pTable->item(i, 0);
		if(!pInternalID) {
			OnServerReady(pServer);
			return;
		}
		if(pInternalID->data(Qt::DisplayRole) == pServer->m_iInternalID) {
			QTableWidgetItem* pPassworded = pTable->item(i, 1);
			QTableWidgetItem* pVAC = pTable->item(i, 2);
			QTableWidgetItem* pName = pTable->item(i, 3);
			QTableWidgetItem* pBots = pTable->item(i, 4);
			QTableWidgetItem* pPlayers = pTable->item(i, 5);
			QTableWidgetItem* pMaxPlayers = pTable->item(i, 6);
			QTableWidgetItem* pMap = pTable->item(i, 7);
			QTableWidgetItem* pPing = pTable->item(i, 8);
			QTableWidgetItem* pTags = pTable->item(i, 9);

			if(!pPassworded || !pVAC || !pName || !pBots || !pPlayers || !pMaxPlayers || !pMap || !pPing || !pTags) {
				OnServerReady(pServer);
				return;
			}

			if(pServer->m_bPassworded)
                pPassworded->setIcon(m_iconPassworded);
			else
                pPassworded->setText("");
			if(pServer->m_bVAC)
                pVAC->setIcon(m_iconVAC);
			else
				pVAC->setText("");
			pName->setText(pServer->m_strName);
			pBots->setData(Qt::DisplayRole, pServer->m_iBots);
			pPlayers->setData(Qt::DisplayRole, pServer->m_iPlayers);
			pMaxPlayers->setData(Qt::DisplayRole, pServer->m_iMaxPlayers);
			pMap->setText(pServer->m_strMap);
			pPing->setData(Qt::DisplayRole, pServer->m_iLatencyMs);
			pTags->setText(pServer->m_strTags);

			on_tableServers_itemSelectionChanged();

			if(!m_pPopulatedServer) {
				m_pPopulatedServer = pServer;
			} else {
				if(pServer->m_iPlayers - pServer->m_iBots > m_pPopulatedServer->m_iPlayers - m_pPopulatedServer->m_iBots) {
					m_pPopulatedServer = pServer;
				}
			}

			if(pServer == m_pPopulatedServer || !m_pPopulatedServer) {
				UpdateConnectActions();
			}

			if(m_Config.m_bEnableNotifications && !m_bNotificationsOnCooldown) {
				if(pServer->m_iPlayers - pServer->m_iBots >= m_Config.m_iNotificationPlayerThreshold) {
					m_aQueuedNotifServers.push_back(pServer);
					QTimer::singleShot(1000, this, &MainWindow::MakeNotification);
				}
			}
			return;
		}
	}

	on_tableServers_itemSelectionChanged();
}

void MainWindow::on_buttonRefresh_clicked()
{
	this->findChild<QPushButton*>("buttonRefresh")->setEnabled(false);
	m_pPopulatedServer = nullptr;
	m_QueryMaster->ClearServerList();
	for(auto it = m_Config.m_aCachedServers.begin(); it != m_Config.m_aCachedServers.end(); ++it) {
		m_QueryMaster->LoadCachedServer(*it);
	}
	for(auto it = m_Config.m_aFavoritedServers.begin(); it != m_Config.m_aFavoritedServers.end(); ++it) {
		m_QueryMaster->MakeFavoriteFromAddr(*it);
	}
    m_QueryMaster->QueryMasterServer(m_Config.m_strGameFolder);
	DisplayFilteredServers();
	QTimer::singleShot(1000, this, &MainWindow::DelayedRefreshButtonEanble);
}


void MainWindow::on_tableServers_itemSelectionChanged()
{
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	QTableWidget* pPlayersTable = this->findChild<QTableWidget*>("tablePlayers");
	QLineEdit* pOutAddress = this->findChild<QLineEdit*>("outputAddress");
	QLineEdit* pOutHostname = this->findChild<QLineEdit*>("outputName");
	QLineEdit* pOutPlayers = this->findChild<QLineEdit*>("outputPlayers");
	QLineEdit* pOutMap = this->findChild<QLineEdit*>("outputMap");
	QLineEdit* pOutVersion = this->findChild<QLineEdit*>("outputVersion");
	QLineEdit* pOutOS = this->findChild<QLineEdit*>("outputOS");
	QLineEdit* pOutType = this->findChild<QLineEdit*>("outputType");
	QLineEdit* pOutVAC = this->findChild<QLineEdit*>("outputVAC");
	QLineEdit* pOutPassword = this->findChild<QLineEdit*>("outputPassword");
	QLineEdit* pOutTags = this->findChild<QLineEdit*>("outputTags");
	QLineEdit* pOutLatency = this->findChild<QLineEdit*>("outputLatency");
	QLineEdit* pOutSTVName = this->findChild<QLineEdit*>("outputSTVName");
	QLineEdit* pOutSTVPort = this->findChild<QLineEdit*>("outputSTVPort");
	QLineEdit* pOutSteamAppID = this->findChild<QLineEdit*>("outputSteamAppID");
	QLineEdit* pOutSteamID = this->findChild<QLineEdit*>("outputSteamID");
	QLineEdit* pOutConnectURL = this->findChild<QLineEdit*>("outputConnectLink");
	QPushButton* pButtonConnect = this->findChild<QPushButton*>("buttonConnect");
	QPushButton* pButtonViewRules = this->findChild<QPushButton*>("buttonViewRules");

	if(pTable->selectedItems().size() < 1) {
		pTable->clearSelection();
		pOutAddress->setText("");
		pOutHostname->setText("");
		pOutPlayers->setText("");
		pOutMap->setText("");
		pOutVersion->setText("");
		pOutOS->setText("");
		pOutType->setText("");
		pOutVAC->setText("");
		pOutPassword->setText("");
		pOutTags->setText("");
		pOutLatency->setText("");
		pOutSTVName->setText("");
		pOutSTVPort->setText("");
		pOutSteamAppID->setText("");
		pOutSteamID->setText("");
		pOutConnectURL->setText("");
		pOutAddress->setText("");
		pButtonConnect->setEnabled(false);
		pButtonViewRules->setEnabled(false);

		pPlayersTable->clearSelection();
		pPlayersTable->clearContents();
		pPlayersTable->setRowCount(0);
		return;
	}
	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	QString strFullAddr = pServer->m_hAddress.toString();
	strFullAddr.append(":");
	strFullAddr.append(QString::number(pServer->m_iPort));
	pOutAddress->setText(strFullAddr);

	pOutHostname->setText(pServer->m_strName);

	QString strPlayers = QString::number(pServer->m_iBots);
	strPlayers.append("/");
	strPlayers.append(QString::number(pServer->m_iPlayers));
	strPlayers.append("/");
	strPlayers.append(QString::number(pServer->m_iMaxPlayers));
	pOutPlayers->setText(strPlayers);

	pOutMap->setText(pServer->m_strMap);
	pOutVersion->setText(pServer->m_strVersion);
	pOutOS->setText(pServer->m_strOS);
	pOutType->setText(pServer->m_strType);

	if(pServer->m_bVAC)
		pOutVAC->setText("Enabled");
	else
		pOutVAC->setText("Disabled");

	if(pServer->m_bPassworded)
		pOutPassword->setText("Enabled");
	else
		pOutPassword->setText("None");

	pOutTags->setText(pServer->m_strTags);
	pOutLatency->setText(QString::number(pServer->m_iLatencyMs));
	pOutSTVName->setText(pServer->m_strSourceTVName);
	pOutSTVPort->setText(QString::number(pServer->m_iSourceTVPort));
	pOutSteamAppID->setText(QString::number(pServer->m_iGameID));
	pOutSteamID->setText(QString::number(pServer->m_iSteamID));

	pButtonViewRules->setEnabled(pServer->m_bReadyRules);

	QTableWidget* pTablePlayers = this->findChild<QTableWidget*>("tablePlayers");
	pTablePlayers->clearContents();
	pTablePlayers->setRowCount(pServer->m_aPlayers.size());
	for(int i = 0; i < pServer->m_aPlayers.size(); ++i) {
		ServerPlayer& plr = pServer->m_aPlayers[i];

		QTableWidgetItem* pItemPlrName = new QTableWidgetItem;
		QTableWidgetItem* pItemPlrScore = new QTableWidgetItem;
		QTableWidgetItem* pItemPlrDuration = new QTableWidgetItem;
		pItemPlrName->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		pItemPlrScore->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		pItemPlrDuration->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		pItemPlrName->setText(plr.m_strName);
		pItemPlrScore->setData(Qt::DisplayRole, plr.m_iScore);
		pItemPlrDuration->setText(plr.DurationString());

		pTablePlayers->setItem(i, 0, pItemPlrName);
        pTablePlayers->setItem(pItemPlrName->row(), 1, pItemPlrScore);
        pTablePlayers->setItem(pItemPlrScore->row(), 2, pItemPlrDuration);
	}

	pOutConnectURL->setText(MakeConnectURL(pServer));
	pButtonConnect->setEnabled(true);
}


void MainWindow::on_tableServers_itemDoubleClicked(QTableWidgetItem *item)
{
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	if(pTable->selectedItems().size() < 1) {
		pTable->clearSelection();
		return;
	}

	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	/* 0 = Refresh
	 * 1 = Connect
	 * 2 = Favorite
	 * 3 = Copy connect link
	 */
	if(m_Config.m_iDoubleClickAction == 0)
		m_QueryMaster->UpdateServer(iInternalServID);
	if(m_Config.m_iDoubleClickAction == 1) {
		QDesktopServices::openUrl(MakeConnectURL(pServer));
	}
	if(m_Config.m_iDoubleClickAction == 2) {
		on_actionBookmark_selected_server_triggered();
	}
	if(m_Config.m_iDoubleClickAction == 3) {
		QGuiApplication::clipboard()->setText(MakeConnectURL(pServer));
	}
}

void MainWindow::on_buttonQuickRefresh_clicked()
{
	m_QueryMaster->UpdateServers();
}


void MainWindow::on_buttonConnect_clicked()
{
	ConnectToSelectedServer();
	if(m_Config.m_bCloseAfterConnect)
		this->close();
}


void MainWindow::on_checkAutoRefresh_stateChanged(int arg1)
{
	m_timerAutoRefresh.stop();
	m_Config.m_bAutoRefreshServers = arg1;
	m_Config.SaveConfig();
	if(arg1 == 0)
		return;

	m_timerAutoRefresh.setInterval(m_Config.m_flAutoRefreshInterval * 1000);
	m_timerAutoRefresh.setSingleShot(false);
	m_timerAutoRefresh.start();
}

void MainWindow::OnTimerAutoRefresh() {
	m_QueryMaster->UpdateServers();
}

void MainWindow::OnTimerFullAutoRefresh() {
	on_buttonRefresh_clicked();
}

void MainWindow::OnTimerNotificationCooldown() {
	m_bNotificationsOnCooldown = false;
}

void MainWindow::on_spinMaxLatency_textChanged(const QString &arg1)
{
	m_Config.m_iMaxLatency = arg1.toInt();
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_spinGameVersion_textChanged(const QString &arg1)
{
	m_Config.m_iGameVersion = arg1.toInt();
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_inputMap_textChanged(const QString &arg1)
{
	m_Config.m_strFilterMap = arg1;
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_inputTags_textChanged(const QString &arg1)
{
	m_Config.m_strFilterTags = arg1;
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_comboTags_currentTextChanged(const QString &arg1)
{
	m_Config.m_bExcludeTags = arg1 == "Exclude";
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_checkHideEmpty_stateChanged(int arg1)
{
	m_Config.m_bHideEmptyPlayers = arg1;
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_checkHidePassworded_stateChanged(int arg1)
{
	m_Config.m_bHidePassworded = arg1;
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_checkHideInsecure_stateChanged(int arg1)
{
	m_Config.m_bHideNoVAC = arg1;
	DisplayFilteredServers();
	m_Config.SaveConfig();
}

void MainWindow::on_actionRefresh_selected_server_triggered()
{
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	if(pTable->selectedItems().size() < 1) {
		pTable->clearSelection();
		return;
	}
	QTableWidgetItem* pItem = pTable->selectedItems()[0];
	// i was wondering why it wasnt working
	// might've been because of this dumb as shit solution
	// on_tableServers_itemDoubleClicked(pItem);

	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	m_QueryMaster->UpdateServer(iInternalServID);
}


void MainWindow::on_actionRefresh_all_servers_triggered()
{
	on_buttonQuickRefresh_clicked();
}


void MainWindow::on_actionReload_server_list_triggered()
{
	on_buttonRefresh_clicked();
}

void MainWindow::resizeEvent(QResizeEvent* pEvent) {
	m_Config.m_iMainWindowWidth = pEvent->size().width();
	m_Config.m_iMainWindowHeight = pEvent->size().height();
	m_Config.SaveConfig();
}

void MainWindow::OnServerListSectionResized(int iColumn, int iOld, int iNew) {
	m_Config.m_aColumnSizes[iColumn] = iNew;
	m_Config.SaveConfig();
}

void MainWindow::on_actionReset_config_triggered()
{
	QFile fileCfg;
	fileCfg.setFileName(m_Config.m_strConfigFile);
	if(fileCfg.exists()) {
		fileCfg.remove();
		ConfigMaster().SaveConfig();
		m_Config.LoadConfig();
		ApplyConfig();
		m_tray.setVisible(false);
		OnStyleChanged(m_Config.m_strStyle);
	}
}

void MainWindow::on_inputFilterName_textChanged(const QString &arg1)
{
	m_Config.m_strFilterHostname = arg1;
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_checkFavoritedOnly_stateChanged(int arg1)
{
	m_Config.m_bDisplayOnlyFavorited = arg1;
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_buttonQueryIP_clicked()
{
	QueryIPDialog* hQueryWnd = new QueryIPDialog(this);
	hQueryWnd->exec();

	delete hQueryWnd;
}


void MainWindow::on_actionCopy_connect_link_to_clipboard_triggered()
{
	QClipboard* hClipboard = QGuiApplication::clipboard();
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	if(pTable->selectedItems().size() < 1) {
		pTable->clearSelection();
		return;
	}
	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	hClipboard->setText(MakeConnectURL(pServer));
}

QString MainWindow::MakeConnectURL(ServerInfo* pServer, bool bSteamURLOnly) {
	QString strURL;
	if(m_Config.m_bUseSteamConnectme && !bSteamURLOnly)
		strURL = "https://connectsteam.me/?";
	else
		strURL = "steam://connect/";

	strURL.append(pServer->m_hAddress.toString());
	strURL.append(":");
	strURL.append(QString::number(pServer->m_iPort));

	return strURL;
}

QString MainWindow::MakeConnectURL(quint32 iAddr, quint16 iPort, bool bSteamURLOnly) {
	QString strURL;
	if(m_Config.m_bUseSteamConnectme && !bSteamURLOnly)
		strURL = "https://connectsteam.me/?";
	else
		strURL = "steam://connect/";

	strURL.append(QHostAddress(iAddr).toString());
	strURL.append(":");
	strURL.append(QString::number(iPort));

	return strURL;
}

void MainWindow::OnServerNeedsRemoval(ServerInfo* pServer) {
	if(pServer == m_pPopulatedServer)
        m_pPopulatedServer = nullptr;

    if(pServer->m_bBogusServer && pServer->m_bCached) {
		for(auto it = m_Config.m_aCachedServers.begin(); it != m_Config.m_aCachedServers.end(); ++it) {
			if(it->first == pServer->m_hAddress.toIPv4Address() && it->second == pServer->m_iPort) {
				m_Config.m_aCachedServers.erase(it);
				break;
			}
		}
	}

	m_Config.SaveConfig();
	// m_QueryMaster->RemoveServer(pServer->m_iInternalID);
}

void MainWindow::on_actionConnect_triggered()
{
	ConnectToSelectedServer();
}

void MainWindow::ConnectToSelectedServer() {
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	if(pTable->selectedItems().size() < 1) {
		pTable->clearSelection();
		return;
	}

	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	QDesktopServices::openUrl(MakeConnectURL(pServer, true));

	m_Config.m_strLastConnectedName = pServer->m_strName;
	m_Config.m_iLastConnectedAddr = pServer->m_hAddress.toIPv4Address();
	m_Config.m_iLastConnectedPort = pServer->m_iPort;

	UpdateConnectActions();

	m_Config.SaveConfig();
}


void MainWindow::on_actionReconnectLast_triggered()
{
	if(m_Config.m_iLastConnectedAddr == 0 && m_Config.m_iLastConnectedPort == 0)
		return;

	ServerInfo* pServer = m_QueryMaster->FindServerFromAddress(QHostAddress(m_Config.m_iLastConnectedAddr), m_Config.m_iLastConnectedPort);
	if(!pServer) {
		m_Config.m_strLastConnectedName = "None";
		m_Config.m_iLastConnectedAddr = 0;
		m_Config.m_iLastConnectedPort = 0;
		UpdateConnectActions();
		return;
	}

	QDesktopServices::openUrl(MakeConnectURL(pServer, true));
}

void MainWindow::UpdateConnectActions() {
	//ServerInfo* pServer = m_QueryMaster->FindServerFromAddress(QHostAddress(m_Config.m_iLastConnectedAddr), m_Config.m_iLastConnectedPort);
	QAction* pMenuConnectBookmarked = this->findChild<QAction*>("actionConnect_to_bookmarked_None");
	QAction* pMenuReconnect = this->findChild<QAction*>("actionReconnectLast");
	QAction* pMenuConnectPopulated = this->findChild<QAction*>("actionConnect_to_populated");

	ServerInfo* pBookmarkedServer = m_QueryMaster->FindServerFromAddress(QHostAddress(m_Config.m_iBookmarkedAddr), m_Config.m_iBookmarkedPort);
	ServerInfo* pLastServer = m_QueryMaster->FindServerFromAddress(QHostAddress(m_Config.m_iLastConnectedAddr), m_Config.m_iLastConnectedPort);

	QString strConnectBookmarked;
	if(pBookmarkedServer)
		strConnectBookmarked = QString("Connect to %1").arg(pBookmarkedServer->m_strName);
	else
		strConnectBookmarked = QString("Connect to %1").arg(m_Config.m_strBookmarkedName);

	QString strReconnect;
	if(pLastServer)
		strReconnect = QString("Reconnect to %1").arg(pLastServer->m_strName);
	else
		strReconnect = QString("Reconnect to %1").arg(m_Config.m_strLastConnectedName);

	QString strConnectPopulated;
	if(m_pPopulatedServer)
		strConnectPopulated = QString("Connect to %1").arg(m_pPopulatedServer->m_strName);
	else
		strConnectPopulated = QString("Connect to %1").arg("None");

	if(strConnectBookmarked.length() > 29) {
		strConnectBookmarked.truncate(29);
		strConnectBookmarked.append("..");
	}
	if(strReconnect.length() > 29) {
		strReconnect.truncate(29);
		strReconnect.append("..");
	}
	if(strConnectPopulated.length() > 29) {
		strConnectPopulated.truncate(29);
		strConnectPopulated.append("..");
	}

	if(pBookmarkedServer)
		strConnectBookmarked.append(QString(" (%1/%2)").arg(QString::number(pBookmarkedServer->m_iPlayers - pBookmarkedServer->m_iBots), QString::number(pBookmarkedServer->m_iMaxPlayers)));
	else
		strConnectBookmarked.append(" (N/A)");

	if(pLastServer)
		strReconnect.append(QString(" (%1/%2)").arg(QString::number(pLastServer->m_iPlayers - pLastServer->m_iBots), QString::number(pLastServer->m_iMaxPlayers)));
	else
		strReconnect.append(" (N/A)");

	if(m_pPopulatedServer)
		strConnectPopulated.append(QString(" (%1/%2)").arg(QString::number(m_pPopulatedServer->m_iPlayers - m_pPopulatedServer->m_iBots), QString::number(m_pPopulatedServer->m_iMaxPlayers)));
	else
		strConnectPopulated.append(" (N/A)");

	pMenuConnectBookmarked->setText(strConnectBookmarked);
	pMenuReconnect->setText(strReconnect);
	pMenuConnectPopulated->setText(strConnectPopulated);

	if(m_actTrayConnectBookmarked)
		m_actTrayConnectBookmarked->setText(strConnectBookmarked);

	if(m_actTrayReconnect)
		m_actTrayReconnect->setText(strReconnect);

	if(m_actTrayConnectPopulated)
		m_actTrayConnectPopulated->setText(strConnectPopulated);
}

void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason hReason) {
	if(hReason == QSystemTrayIcon::ActivationReason::Trigger) {
		this->show();
	}
}

void MainWindow::OnActionShowTriggered() {
	this->show();
}

void MainWindow::OnActionToggleNotificationsTriggered() {
	m_Config.m_bEnableNotifications = !m_Config.m_bEnableNotifications;
	if(m_Config.m_bEnableNotifications)
		m_actTrayToggleNotifications->setText("Disable notifications");
	else
		m_actTrayToggleNotifications->setText("Enable notifications");
	m_Config.SaveConfig();
}

void MainWindow::OnActionQuitTriggered() {
	m_Config.m_bUseTray = false;
	this->close();
}

void MainWindow::closeEvent(QCloseEvent * pEvent) {
	if(m_Config.m_bUseTray) {
		this->hide();
		pEvent->ignore();
	} else {
		pEvent->accept();
		QApplication::instance()->exit(0);
	}
}

void MainWindow::on_actionClear_cache_triggered()
{
	m_Config.m_aCachedServers.clear();
	m_Config.SaveConfig();
}

void MainWindow::DelayedRefreshButtonEanble() {
	this->findChild<QPushButton*>("buttonRefresh")->setEnabled(true);
}

void MainWindow::on_actionRequest_server_rules_triggered()
{
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	if(pTable->selectedItems().size() < 1) {
		pTable->clearSelection();
		return;
	}

	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	pServer->m_bQueryingRules = true;
	m_QueryMaster->UpdateServer(pServer->m_iInternalID);
}

void MainWindow::OnLocalServerNewConnection() {
	this->show();
}

void MainWindow::on_actionConnect_to_bookmarked_None_triggered()
{
	if(m_Config.m_iBookmarkedAddr == 0 && m_Config.m_iBookmarkedPort == 0)
		return;

	QDesktopServices::openUrl(MakeConnectURL(m_Config.m_iBookmarkedAddr, m_Config.m_iBookmarkedPort, true));
}


void MainWindow::on_actionConnect_to_populated_triggered()
{
	if(!m_pPopulatedServer)
		return;

	m_Config.m_strLastConnectedName = m_pPopulatedServer->m_strName;
	m_Config.m_iLastConnectedAddr = m_pPopulatedServer->m_hAddress.toIPv4Address();
	m_Config.m_iLastConnectedPort = m_pPopulatedServer->m_iPort;

	UpdateConnectActions();

	m_Config.SaveConfig();

	QDesktopServices::openUrl(MakeConnectURL(m_pPopulatedServer, true));
}


void MainWindow::on_actionFavorite_selected_server_triggered()
{
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	if(pTable->selectedItems().size() < 1) {
		pTable->clearSelection();
		return;
	}
	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	for(auto it = m_Config.m_aFavoritedServers.begin(); it != m_Config.m_aFavoritedServers.end(); ++it) {
		if(pServer->m_hAddress.toIPv4Address() == it->first && pServer->m_iPort == it->second) {
			m_Config.m_aFavoritedServers.erase(it);
			pServer->m_bFavorited = false;
			DisplayFilteredServers();
			m_Config.SaveConfig();
			return;
		}
	}

	m_Config.m_aFavoritedServers.push_back(std::make_pair(pServer->m_hAddress.toIPv4Address(), pServer->m_iPort));
	pServer->m_bFavorited = true;
	DisplayFilteredServers();
	m_Config.SaveConfig();
}


void MainWindow::on_actionBookmark_selected_server_triggered()
{
	QTableWidget* pTable = this->findChild<QTableWidget*>("tableServers");
	if(pTable->selectedItems().size() < 1) {
		pTable->clearSelection();
		return;
	}
	int iInternalServID = pTable->item(pTable->selectedItems()[0]->row(), 0)->data(Qt::DisplayRole).toInt();
	ServerInfo* pServer = m_QueryMaster->m_aServers[iInternalServID];

	m_Config.m_iBookmarkedAddr = pServer->m_hAddress.toIPv4Address();
	m_Config.m_iBookmarkedPort = pServer->m_iPort;
	m_Config.m_strBookmarkedName = pServer->m_strName;

	UpdateConnectActions();

	m_Config.SaveConfig();
}

void MainWindow::MakeNotification() {
	if(m_bNotificationsOnCooldown)
		return;

	while(!m_aQueuedNotifServers.empty()) {
		ServerInfo* pServer = m_aQueuedNotifServers.head();
		if(m_Config.m_bNotificationFavoritesOnly && !pServer->m_bFavorited) {
			m_aQueuedNotifServers.pop_front();
			continue;
		}
		if(pServer->m_iPlayers - pServer->m_iBots < m_Config.m_iNotificationPlayerThreshold)
			return;

		QString strTitle = "A game is going!";
		QString strDescription = QString("%1 has reached ").arg(pServer->m_strName);
		strDescription.append(QString::number(pServer->m_iPlayers - pServer->m_iBots));
		if(pServer->m_iPlayers - pServer->m_iBots > 1)
			strDescription.append(" players!");
		else
			strDescription.append(" player!");

		m_tray.showMessage(strTitle, strDescription, QSystemTrayIcon::MessageIcon::NoIcon);

		m_timerNotificationsCooldown.setInterval(m_Config.m_iNotificationCooldownMin * 1000 * 60);
		m_timerNotificationsCooldown.start();
		m_bNotificationsOnCooldown = true;
		m_aQueuedNotifServers.clear();
		break;
	}
}
