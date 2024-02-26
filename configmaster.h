#ifndef CONFIGMASTER_H
#define CONFIGMASTER_H

#include <QString>
#include <QStringList>
#include <QFile>
#include "datareadwriter.h"

#define SERVER_LIST_COLUMN_COUNT 10

class ConfigMaster : private DataReadWriter
{
public:
	ConfigMaster();

	const char m_iNewestCfgVersion = 3;
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
	qint8 m_iStyleIndex = 0;
	QString m_strStyle = "Default";
	qint16 m_iMainWindowWidth = 1280;
	qint16 m_iMainWindowHeight = 720;
	qint8 m_iColumns = SERVER_LIST_COLUMN_COUNT;
	qint16 m_aColumnSizes[SERVER_LIST_COLUMN_COUNT];
	bool m_bCloseAfterConnect = false;

	void SaveConfig();
	void LoadConfig();
private:
	QFile fileCfg;
};

#endif // CONFIGMASTER_H
