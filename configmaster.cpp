#include "configmaster.h"
#include <QDebug>

ConfigMaster::ConfigMaster() {
	fileCfg.setFileName(m_strConfigFile);
	m_aColumnSizes[0] = 0;
	m_aColumnSizes[1] = 20;
	m_aColumnSizes[2] = 20;
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

		m_addrLastConnected = ReadUInt32(rawData, iCursor);
		m_strLastConnectedName = ReadString(rawData, iCursor);
		m_strFilterHostname = ReadString(rawData, iCursor);
		m_bDisplayOnlyFavorited = ReadInt8(rawData, iCursor);
		int iFavoritedServerCount = ReadUInt16(rawData, iCursor);
		m_aiFavoritedServers.clear();
		for(int i = 0; i < iFavoritedServerCount; ++i) {
			quint32 iAddr = ReadUInt32(rawData, iCursor);
			quint16 iPort = ReadUInt16(rawData, iCursor);
			m_aiFavoritedServers.push_back(std::make_pair(iAddr, iPort));
		}
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
	WriteUInt32(rawData, m_addrLastConnected);
	WriteString(rawData, m_strLastConnectedName);
	WriteString(rawData, m_strFilterHostname);
	WriteInt8(rawData, m_bDisplayOnlyFavorited);
	WriteUInt16(rawData, m_aiFavoritedServers.size());
	for(int i = 0; i < m_aiFavoritedServers.size(); ++i) {
		WriteUInt32(rawData, m_aiFavoritedServers[i].first);
		WriteUInt16(rawData, m_aiFavoritedServers[i].second);
	}

	fileCfg.open(QIODevice::WriteOnly);
	fileCfg.write(rawData);
	fileCfg.close();
}
