#include "configmaster.h"
#include <QDebug>

ConfigMaster::ConfigMaster() {
	fileCfg.setFileName(m_strConfigFile);
	m_aColumnSizes[0] = 0;
    m_aColumnSizes[1] = 24;
    m_aColumnSizes[2] = 24;
	m_aColumnSizes[3] = 230;
	m_aColumnSizes[4] = 26;
	m_aColumnSizes[5] = 26;
	m_aColumnSizes[6] = 26;
	m_aColumnSizes[7] = 100;
	m_aColumnSizes[8] = 40;
	m_aColumnSizes[9] = 250;
}

void ConfigMaster::LoadConfig() {
	if(!fileCfg.exists())
		SaveConfig();

	fileCfg.open(QIODevice::ReadOnly);

	QByteArray rawData = fileCfg.readAll();
	fileCfg.close();

	quint32 iCursor = 0;
	m_iCfgVersion = ReadInt8(rawData, iCursor);
	m_bHideEmptyPlayers = ReadInt8(rawData, iCursor);
	m_bHideNoVAC = ReadInt8(rawData, iCursor);
	m_bHidePassworded = ReadInt8(rawData, iCursor);
	m_iMaxLatency = ReadUInt32(rawData, iCursor);
	m_iGameVersion = ReadUInt32(rawData, iCursor);
	m_strFilterMap = ReadString(rawData, iCursor);
	m_strFilterTags = ReadString(rawData, iCursor);
	m_bExcludeTags = ReadInt8(rawData, iCursor);
	m_bUseSteamConnectme = ReadInt8(rawData, iCursor);
	m_bAutoRefreshServers = ReadInt8(rawData, iCursor);
	m_flAutoRefreshInterval = ReadFloat(rawData, iCursor);
	m_iStyleIndex = ReadUInt8(rawData, iCursor);
	m_strStyle = ReadString(rawData, iCursor);
	if(m_iCfgVersion >= 2) {
		m_iMainWindowWidth = ReadUInt16(rawData, iCursor);
		m_iMainWindowHeight = ReadUInt16(rawData, iCursor);
		m_iColumns = ReadUInt8(rawData, iCursor);
		for(int i = 0; i < m_iColumns; ++i) {
			m_aColumnSizes[i] = ReadUInt16(rawData, iCursor);
		}
	}
	if(m_iCfgVersion >= 3) {
		m_bCloseAfterConnect = ReadInt8(rawData, iCursor);
	}
	if(m_iCfgVersion >= 4) {
		m_iDoubleClickAction = ReadUInt8(rawData, iCursor);
		m_strStylePath = ReadString(rawData, iCursor);
		m_bUseTray = ReadInt8(rawData, iCursor);
		m_bEnableNotifications = ReadInt8(rawData, iCursor);
		m_iNotificationCooldownMin = ReadUInt32(rawData, iCursor);
		m_iNotificationPlayerThreshold = ReadUInt8(rawData, iCursor);
		m_iLastConnectedAddr = ReadUInt32(rawData, iCursor);
		m_iLastConnectedPort = ReadUInt16(rawData, iCursor);
		m_strLastConnectedName = ReadString(rawData, iCursor);
		m_strFilterHostname = ReadString(rawData, iCursor);
		m_bDisplayOnlyFavorited = ReadInt8(rawData, iCursor);
		int iFavoritedServerCount = ReadUInt16(rawData, iCursor);
		m_aFavoritedServers.clear();
		for(int i = 0; i < iFavoritedServerCount; ++i) {
			quint32 iAddr = ReadUInt32(rawData, iCursor);
			quint16 iPort = ReadUInt16(rawData, iCursor);
			m_aFavoritedServers.push_back(std::make_pair(iAddr, iPort));
		}
	}
	if(m_iCfgVersion >= 5) {
		m_bEnableCaching = ReadInt8(rawData, iCursor);
		m_iCachedServerCount = ReadUInt16(rawData, iCursor);
		for(int i = 0; i < m_iCachedServerCount; ++i) {
			quint32 iAddr = ReadUInt32(rawData, iCursor);
			quint16 iPort = ReadUInt16(rawData, iCursor);
			m_aCachedServers.push_back(std::make_pair(iAddr, iPort));
		}
	}
	if(m_iCfgVersion >= 6) {
		m_iBookmarkedAddr = ReadUInt32(rawData, iCursor);
		m_iBookmarkedPort = ReadUInt16(rawData, iCursor);
		m_strBookmarkedName = ReadString(rawData, iCursor);
	}
	if(m_iCfgVersion >= 7) {
		m_bFullAutoRefreshServers = ReadInt8(rawData, iCursor);
		m_flFullAutoRefreshInterval = ReadFloat(rawData, iCursor);
		m_bNotificationFavoritesOnly = ReadInt8(rawData, iCursor);
	}
    if(m_iCfgVersion >= 8) {
        m_strGameFolder = ReadString(rawData, iCursor);
    }
}

void ConfigMaster::SaveConfig() {
	QByteArray rawData;
	quint32 iCursor = 0;
	WriteInt8(rawData, m_iNewestCfgVersion);
	WriteInt8(rawData, m_bHideEmptyPlayers);
	WriteInt8(rawData, m_bHideNoVAC);
	WriteInt8(rawData, m_bHidePassworded);
	WriteUInt32(rawData, m_iMaxLatency);
	WriteUInt32(rawData, m_iGameVersion);
	WriteString(rawData, m_strFilterMap);
	WriteString(rawData, m_strFilterTags);
	WriteInt8(rawData, m_bExcludeTags);
	WriteInt8(rawData, m_bUseSteamConnectme);
	WriteInt8(rawData, m_bAutoRefreshServers);
	WriteFloat(rawData, m_flAutoRefreshInterval);
	WriteUInt8(rawData, m_iStyleIndex);
	WriteString(rawData, m_strStyle);
	WriteUInt16(rawData, m_iMainWindowWidth);
	WriteUInt16(rawData, m_iMainWindowHeight);
	WriteUInt8(rawData, m_iColumns);
	for(int i = 0; i < m_iColumns; ++i) {
		WriteUInt16(rawData, m_aColumnSizes[i]);
	}
	WriteInt8(rawData, m_bCloseAfterConnect);

	WriteUInt8(rawData, m_iDoubleClickAction);
	WriteString(rawData, m_strStylePath);
	WriteInt8(rawData, m_bUseTray);
	WriteInt8(rawData, m_bEnableNotifications);
	WriteUInt32(rawData, m_iNotificationCooldownMin);
	WriteUInt8(rawData, m_iNotificationPlayerThreshold);
	WriteUInt32(rawData, m_iLastConnectedAddr);
	WriteUInt16(rawData, m_iLastConnectedPort);
	WriteString(rawData, m_strLastConnectedName);
	WriteString(rawData, m_strFilterHostname);
	WriteInt8(rawData, m_bDisplayOnlyFavorited);
	WriteUInt16(rawData, m_aFavoritedServers.size());
	for(int i = 0; i < m_aFavoritedServers.size(); ++i) {
		WriteUInt32(rawData, m_aFavoritedServers[i].first);
		WriteUInt16(rawData, m_aFavoritedServers[i].second);
	}
	WriteInt8(rawData, m_bEnableCaching);
	WriteUInt16(rawData, m_aCachedServers.size());
	for(int i = 0; i < m_aCachedServers.size(); ++i) {
		WriteUInt32(rawData, m_aCachedServers[i].first);
		WriteUInt16(rawData, m_aCachedServers[i].second);
	}
	WriteUInt32(rawData, m_iBookmarkedAddr);
	WriteUInt16(rawData, m_iBookmarkedPort);
	WriteString(rawData, m_strBookmarkedName);
	WriteUInt8(rawData, m_bFullAutoRefreshServers);
	WriteFloat(rawData, m_flFullAutoRefreshInterval);
	WriteUInt8(rawData, m_bNotificationFavoritesOnly);
    WriteString(rawData, m_strGameFolder);

	fileCfg.open(QIODevice::WriteOnly);
	fileCfg.write(rawData);
	fileCfg.close();
}
