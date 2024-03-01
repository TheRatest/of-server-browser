#ifndef CONFIGMASTER_H
#define CONFIGMASTER_H

#include <QString>
#include <QStringList>
#include <QFile>
#include "datareadwriter.h"
#include <vector>

#define SERVER_LIST_COLUMN_COUNT 10

class ConfigMaster : private DataReadWriter
{
public:
	ConfigMaster();

	const char m_iNewestCfgVersion = 4;
	const QString m_strConfigFile = "config.cfg";

	qint8 m_iCfgVersion = 0;
	bool m_bHideEmptyPlayers = false;
	bool m_bHideNoVAC = false;
	bool m_bHidePassworded = false;
	int m_iMaxLatency = 0;
	int m_iGameVersion = 0;
	QString m_strFilterMap = "";
	QString m_strFilterTags = {};
	bool m_bExcludeTags = false;
	bool m_bUseSteamConnectme = false;
	bool m_bAutoRefreshServers = true;
	float m_flAutoRefreshInterval = 5.0;
	quint8 m_iStyleIndex = 0;
	QString m_strStyle = "Default";
	quint16 m_iMainWindowWidth = 1280;
	quint16 m_iMainWindowHeight = 720;
	quint8 m_iColumns = SERVER_LIST_COLUMN_COUNT;
	quint16 m_aColumnSizes[SERVER_LIST_COLUMN_COUNT];
	bool m_bCloseAfterConnect = false;

	quint8 m_iDoubleClickAction = 0;
	QString m_strStylePath = "customstyle.qss";
	bool m_bUseTray = false;
	bool m_bEnableNotifications = false;
	quint32 m_iNotificationCooldownMin = 30;
	quint8 m_iNotificationPlayerThreshold = 3;
	QString m_strFilterHostname = "";
	bool m_bDisplayOnlyFavorited = false;
	quint16 m_iFavoritedServerCount = 0;
	std::vector<std::pair<quint32, quint16>> m_aiFavoritedServers = {};

	quint32 m_addrLastConnected = 0;
	QString m_strLastConnectedName = "None";

	void SaveConfig();
	void LoadConfig();
private:
	QFile fileCfg;
};

#endif // CONFIGMASTER_H
