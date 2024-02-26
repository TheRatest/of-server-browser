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
	m_iMaxLatency = ReadInt32(rawData, iCursor);
	m_iGameVersion = ReadInt32(rawData, iCursor);
	m_strFilterMap = ReadString(rawData, iCursor);
	m_strFilterTags = ReadString(rawData, iCursor);
	m_bExcludeTags = ReadInt8(rawData, iCursor);
	m_bUseSteamConnectme = ReadInt8(rawData, iCursor);
	m_bAutoRefreshServers = ReadInt8(rawData, iCursor);
	m_flAutoRefreshInterval = ReadFloat(rawData, iCursor);
	m_iStyleIndex = ReadInt8(rawData, iCursor);
	m_strStyle = ReadString(rawData, iCursor);
	if(m_iCfgVersion >= 2) {
		m_iMainWindowWidth = ReadInt16(rawData, iCursor);
		m_iMainWindowHeight = ReadInt16(rawData, iCursor);
		m_iColumns = ReadInt8(rawData, iCursor);
		for(int i = 0; i < m_iColumns; ++i) {
			m_aColumnSizes[i] = ReadInt16(rawData, iCursor);
		}
	}
	if(m_iCfgVersion >= 3) {
		m_bCloseAfterConnect = ReadInt8(rawData, iCursor);
	}
}

void ConfigMaster::SaveConfig() {
	QByteArray rawData;
	quint32 iCursor = 0;
	WriteInt8(rawData, m_iNewestCfgVersion);
	WriteInt8(rawData, m_bHideEmptyPlayers);
	WriteInt8(rawData, m_bHideNoVAC);
	WriteInt8(rawData, m_bHidePassworded);
	WriteInt32(rawData, m_iMaxLatency);
	WriteInt32(rawData, m_iGameVersion);
	WriteString(rawData, m_strFilterMap);
	WriteString(rawData, m_strFilterTags);
	WriteInt8(rawData, m_bExcludeTags);
	WriteInt8(rawData, m_bUseSteamConnectme);
	WriteInt8(rawData, m_bAutoRefreshServers);
	WriteFloat(rawData, m_flAutoRefreshInterval);
	WriteInt8(rawData, m_iStyleIndex);
	WriteString(rawData, m_strStyle);
	WriteInt16(rawData, m_iMainWindowWidth);
	WriteInt16(rawData, m_iMainWindowHeight);
	WriteInt8(rawData, m_iColumns);
	for(int i = 0; i < m_iColumns; ++i) {
		WriteInt16(rawData, m_aColumnSizes[i]);
	}
	WriteInt8(rawData, m_bCloseAfterConnect);

	fileCfg.open(QIODevice::WriteOnly);
	fileCfg.write(rawData);
	fileCfg.close();
}
