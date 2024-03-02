#include "querymaster.h"
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QHostInfo>
#include <QRandomGenerator>
//#include "mainwindow.h"

QString ServerPlayer::DurationString() {
	int iHours = floor(m_flDuration / 3600);
	int iMinutes = (int)floor(m_flDuration / 60) % 60;
	int iSeconds = floor((int)m_flDuration % 60);
	QString strOut = "%1:%2:%3";
	strOut = strOut.arg(iHours).arg(iMinutes).arg(iSeconds);
	return strOut;
}

ServerInfo::ServerInfo() {

}

void ServerInfo::ParseInfoPacket(QByteArray rawData) {
	// here i go..
	// read cursor starts at 5 bc the first 5 bytes are the packet header
	quint32 iCursor = 5;
	m_iProtocol = ReadInt8(rawData, iCursor);
	m_strName = ReadString(rawData, iCursor);
	m_strMap = ReadString(rawData, iCursor);
	m_strGameFolder = ReadString(rawData, iCursor);
	m_strGame = ReadString(rawData, iCursor);
	m_iSteamAppID = ReadInt16(rawData, iCursor);
	m_iPlayers = ReadInt8(rawData, iCursor);
	m_iMaxPlayers = ReadInt8(rawData, iCursor);
	m_iBots = ReadInt8(rawData, iCursor);
	// ba bah
	m_iPlayers -= m_iBots;
	qint8 iServType = ReadInt8(rawData, iCursor);
	if(iServType == 'd')
		m_strType = "Dedicated";
	else if(iServType == 'l')
		m_strType = "Listen";
	else if(iServType == 'p')
		m_strType = "SourceTV Relay";
	qint8 iServOS = ReadInt8(rawData, iCursor);
	if(iServOS == 'w')
		m_strOS = "Windows";
	else if(iServOS == 'l')
		m_strOS = "Linux";
	else if(iServOS == 'm' || iServOS == 'o')
		m_strOS = "Mac";

	m_bPassworded = ReadInt8(rawData, iCursor);
	m_bVAC = ReadInt8(rawData, iCursor);
	m_strVersion = ReadString(rawData, iCursor);
	m_iRevision = m_strVersion.toInt() - 10000000;
	m_nExtraDataFlags = ReadInt8(rawData, iCursor);
	if(m_nExtraDataFlags & 0x80)
		m_iPort = ReadInt16(rawData, iCursor);
	if(m_nExtraDataFlags & 0x10)
		m_iSteamID = ReadInt64(rawData, iCursor);
	if(m_nExtraDataFlags & 0x40) {
		m_iSourceTVPort = ReadInt16(rawData, iCursor);
		m_strSourceTVName = ReadString(rawData, iCursor);
	}
	if(m_nExtraDataFlags & 0x20)
		m_strTags = ReadString(rawData, iCursor);
	if(m_nExtraDataFlags & 0x1)
		m_iGameID = ReadInt64(rawData, iCursor);

	m_bReadyInfo = true;
	// if you decide to wait for the players packet as well it takes way too long for a server to be ready
	//if(m_bReadyPlayers)
		m_bReady = true;
}

void ServerInfo::ParsePlayersPacket(QByteArray rawData) {
	m_aPlayers.clear();
	quint32 iCursor = 5;
	short iPlayers = ReadInt8(rawData, iCursor);
	for(short i = 0; i < iPlayers; ++i) {
		ServerPlayer plr;
		int iPlrIndex = ReadInt8(rawData, iCursor);
		plr.m_strName = ReadString(rawData, iCursor);
		plr.m_iScore = ReadInt32(rawData, iCursor);
		plr.m_flDuration = ReadFloat(rawData, iCursor);
		m_aPlayers.push_back(plr);
	}

	m_bReadyPlayers = true;
	if(m_bReadyInfo)
		m_bReady = true;
}

void ServerInfo::ParseRulesPacket(QByteArray rawData) {
	m_aRules.clear();

	quint32 iCursor = 5;
	int iRulesCount = ReadInt16(rawData, iCursor);
	for(int i = 0; i < iRulesCount; ++i) {
		QString strName = ReadString(rawData, iCursor);
		QString strVal = ReadString(rawData, iCursor);
		m_aRules.push_back(std::make_pair(strName, strVal));
	}

	m_bQueryingRules = false;
	m_bReadyRules = true;
}

void ServerInfo::ParseStitchedRules() {
	m_aRules.clear();
	m_rulesRaw.clear();
	for(int i = 0; i < m_aRulesPackets.size(); ++i) {
		for(int j = 0; i < m_aRulesPackets.size(); ++j) {
			if(m_aRulesPackets[j].first == i) {
				m_rulesRaw.append(m_aRulesPackets[j].second);
				break;
			}
		}
	}

	quint32 iCursor = 5;
	// why does the server say that theres 2 more rules???
	// theyre both literally just empty
	qint16 iRulesCount = ReadInt16(m_rulesRaw, iCursor) - 2;

	for(int i = 0; i < iRulesCount; ++i) {
		QString strName = ReadString(m_rulesRaw, iCursor);
		QString strVal = ReadString(m_rulesRaw, iCursor);
		m_aRules.push_back(std::make_pair(strName, strVal));
	}

	m_bQueryingRules = false;
	m_bReadyRules = true;
}

bool ServerInfo::QueueRulesPacket(QByteArray rawData) {
	quint32 iCursor = 4;

	qint32 iPacketID = ReadInt32(rawData, iCursor);
	qint8 iPacketTotal = ReadInt8(rawData, iCursor);
	qint8 iPacketNum = ReadInt8(rawData, iCursor);
	qint16 iPacketSize = ReadInt16(rawData, iCursor);
	if(m_iRulesPacketsID == 0)
		m_iRulesPacketsID = iPacketID;

	if(m_iRulesPacketsCount == 0)
		m_iRulesPacketsCount = iPacketTotal;

	if(iPacketID != m_iRulesPacketsID || iPacketTotal != m_iRulesPacketsCount)
		return false;

	++m_iRulesPacketsReceived;

	QByteArray pushPacket;
	pushPacket.resize(iPacketSize);
	// sigsegv my beloved
	memcpy(pushPacket.begin(), rawData.begin()+iCursor, rawData.size()-iCursor);
	m_aRulesPackets.push_back(std::make_pair(iPacketNum, pushPacket));

	if(m_iRulesPacketsReceived == m_iRulesPacketsCount) {
		m_iRulesPacketsID = 0;
		m_iRulesPacketsCount = 0;
		ParseStitchedRules();
		return true;
	}

	return false;
}

bool ServerInfo::IsOpenFortressServer() {
	return (m_strGameFolder == "open_fortress");
}

QueryMaster::QueryMaster(QObject* pParent) {
	this->setParent(pParent);

	m_hSocket = new QUdpSocket(this);
	connect(m_hSocket, &QUdpSocket::readyRead, this, &QueryMaster::ReadPendingPackets);
	m_hSocket->bind(m_hMasterAddress, HL2MASTER_PORT);
	qDebug() << "Total master servers: " << m_aStoredMasterAddresses.size();
}

void QueryMaster::QueryMasterServer() {
	if(m_bLocalOnly) {
		QueryLocalServer();
		return;
	}

	m_hMasterAddress = m_aStoredMasterAddresses[m_iStoredAddressesIndex];

	m_bReceivedMasterPacket = false;
	m_bFinishedQueryingMaster = false;
	QByteArray sendPacket;
	sendPacket.append("\x31\xFF"); //memcpy(sendPacket.begin(), "\x31\xFF", 2);
	sendPacket.append(m_strSeed.toStdString().c_str()); //memcpy(sendPacket.begin() + 2, m_strSeed.toStdString().c_str(), m_strSeed.toStdString().length()+1);
	sendPacket.append('\0');
	sendPacket.append( "\\gamedir\\open_fortress"); //memcpy(sendPacket.begin() + m_strSeed.toStdString().length() + 2 + 1 + 1, "\\gamedir\\open_fortress", 23);
	sendPacket.append('\0');

	int iBytesSent = m_hSocket->writeDatagram(sendPacket, m_hMasterAddress, HL2MASTER_PORT);
	if(iBytesSent < 0) {
		qDebug() << m_hSocket->error();
		qDebug() << m_hSocket->errorString();
	} else {
		qDebug() << "Querying master server";
	}
	QTimer::singleShot(m_iTimeoutThresholdMs, this, &QueryMaster::CheckForTimeouts);
}

void QueryMaster::QueryLocalServer() {
	qDebug() << "Querying local server";
	QHostAddress hAddrServer = QHostAddress::LocalHost;
	quint16 iPort = 27015;
	ServerInfo* pServer = new ServerInfo;
	pServer->m_hAddress = hAddrServer;
	pServer->m_iPort = iPort;
	pServer->m_iInternalID = m_aServers.size();
	pServer->m_timeQueryStart = std::chrono::system_clock::now();

	m_aServers.push_back(pServer);

	SendInfoQuery(pServer);
	//SendPlayersQuery(pServer);
	//SendRulesQuery(pServer);
}

void QueryMaster::ReadPendingPackets() {
	while(m_hSocket->hasPendingDatagrams()) {
		QHostAddress hAddrSender;
		quint16 iPort;
		QByteArray recvPacket;
		recvPacket.resize(m_hSocket->pendingDatagramSize());
		quint64 iBytesRead = m_hSocket->readDatagram(recvPacket.begin(), recvPacket.size(), &hAddrSender, &iPort);

		if(iBytesRead < 5 || recvPacket.size() < 5)
			continue;

		if(*reinterpret_cast<int*>(&recvPacket[0]) != -1 && *reinterpret_cast<int*>(&recvPacket[0]) != -2)
			continue;

		ServerInfo* pServer = FindServerFromAddress(hAddrSender, iPort);
		if(pServer == nullptr && recvPacket[4] != RESPONSE_SERVERS)
			continue;

		if(*reinterpret_cast<int*>(&recvPacket[0]) == -2) {
			if(recvPacket.size() < 15)
				continue;

			//qDebug() << ": " << recvPacket.toHex(' ');
			if(recvPacket[16] == RESPONSE_RULES && !pServer->m_bQueryingRules)
				pServer->m_bQueryingRules = true;

			if(!pServer->m_bQueryingRules)
				continue;

			// the packet gets checked for being the right one inside that method, not here
			if(pServer->QueueRulesPacket(recvPacket))
				emit ServerUpdated(pServer);

			continue;
		}

		switch(recvPacket[4]) {
		case RESPONSE_CHALLENGE: {
			if(m_aServers.size() == 0)
				return;

			quint32* iChallenge = reinterpret_cast<quint32*>(&recvPacket[5]);
			if(!pServer->m_bReadyInfo)
				SendInfoQuery(pServer, *iChallenge);

			if(!pServer->m_bReadyPlayers)
				SendPlayersQuery(pServer, *iChallenge);

			if(!pServer->m_bReadyRules) // && pServer->m_bQueryingRules)
				SendRulesQuery(pServer, *iChallenge);

			break;
		}
		case RESPONSE_INFO: {
			if(m_aServers.size() == 0)
				return;

			if(pServer->m_bReadyInfo)
				continue;
			// updating latency!!
			std::chrono::time_point<std::chrono::system_clock> timeNow = std::chrono::system_clock::now();
			// fuck this type bs, im just gonna use auto :c
			std::chrono::milliseconds timePing = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - pServer->m_timeQueryStart);
			// love the readyRead signal delay !!
			// setting the latency to the mean of the last value and the new one for a more stable value
			pServer->m_iLatencyMs = (pServer->m_iLatencyMs + std::max((qint64)1, (qint64)(timePing.count() - 15 - (timePing.count()/20)))) / 2;

			bool bReadyBefore = pServer->m_bReady;
			pServer->ParseInfoPacket(recvPacket);
			pServer->m_bBogusServer = !pServer->IsOpenFortressServer();

			if(pServer->m_bBogusServer) {
				pServer->m_bBogusServer = true;
				if(pServer->m_bCached) {
					emit ServerNeedsRemoval(pServer);
				} else {
					ClearServerList();
					const int iRequeryDelayMs = 2000;
					QTimer::singleShot(iRequeryDelayMs, this, &QueryMaster::QueryMasterServer);
				}
				return;
			}

			if(!bReadyBefore && pServer->m_bReady) {
				//m_aReadyServers.push_back(pServer);
				emit ServerIsReady(pServer);
			}
			else if(bReadyBefore)
				emit ServerUpdated(pServer);

			break;
		}
		case RESPONSE_PLAYERS: {
			if(pServer->m_bReadyPlayers)
				continue;

			bool bReadyBefore = pServer->m_bReady;
			pServer->ParsePlayersPacket(recvPacket);
			if(!bReadyBefore && pServer->m_bReady) {
				//m_aReadyServers.push_back(pServer);
				emit ServerIsReady(pServer);
			}
			else if(bReadyBefore)
				emit ServerUpdated(pServer);

			break;
		}
		case RESPONSE_RULES: {
			if(pServer->m_bReadyRules)
				continue;

			bool bReadyBefore = pServer->m_bReady;
			pServer->ParseRulesPacket(recvPacket);
			if(!bReadyBefore && pServer->m_bReady) {
				//m_aReadyServers.push_back(pServer);
				emit ServerIsReady(pServer);
			}
			else if(bReadyBefore)
				emit ServerUpdated(pServer);

			break;
		}
		case RESPONSE_SERVERS: {
			qDebug() << "Received server list";

			if(!m_bReceivedMasterPacket)
				m_bReceivedMasterPacket = true;

			if(m_bFinishedQueryingMaster)
				continue;

			for(int iCursor = 6; iCursor < recvPacket.size(); iCursor += sizeof(char) * 4 + sizeof(short)) {
				quint32 iAddress = *(quint32*)(recvPacket.begin()+iCursor);
				char* iAddressBytes = reinterpret_cast<char*>(&iAddress);
				// yippiee !!
				std::swap(iAddressBytes[0], iAddressBytes[3]);
				std::swap(iAddressBytes[1], iAddressBytes[2]);
				QHostAddress hAddrServer = QHostAddress(iAddress);
				quint16 iPort = (*(quint16*)(recvPacket.begin()+iCursor + sizeof(char) * 4));
				// fuckign network order..
				char* iPortBytes = reinterpret_cast<char*>(&iPort);
				std::swap(iPortBytes[0], iPortBytes[1]);
				if(hAddrServer.toIPv4Address() == 0 && iPort == 0) {
					m_bFinishedQueryingMaster = true;
					break;
				}

				ServerInfo* pExistingServer = FindServerFromAddress(hAddrServer, iPort);
				if(pExistingServer) {
					SendInfoQuery(pExistingServer);
					continue;
				}

				ServerInfo* pServer = new ServerInfo;
				pServer->m_hAddress = hAddrServer;
				pServer->m_iPort = iPort;
				pServer->m_iInternalID = m_aServers.size();
				pServer->m_timeQueryStart = std::chrono::system_clock::now();

				m_aServers.push_back(pServer);

				SendInfoQuery(pServer);
				//SendPlayersQuery(pServer);
				//SendRulesQuery(pServer);

				// another list of servers coming up ahead!
				if(iCursor + sizeof(char) * 4 + sizeof(short) > recvPacket.size()) {
					m_strSeed = hAddrServer.toString();
					m_strSeed.append(":");
					m_strSeed.append(QString::number(iPort));
					QueryMasterServer();
					break;
				}
			}
			qDebug() << "Received servers from master: " << m_aServers.size();
			break;
		}
		}
	}
}

void QueryMaster::SendInfoQuery(ServerInfo* pServer) {
	if(pServer == nullptr)
		return;

	QByteArray sendPacket;
	sendPacket.resize(25);
	memcpy(sendPacket.begin(), "\xFF\xFF\xFF\xFF" "TSource Engine Query", 25);
	quint64 iBytesSent = m_hSocket->writeDatagram(sendPacket, pServer->m_hAddress, pServer->m_iPort);
	if(iBytesSent < 0) {
		qDebug() << m_hSocket->error();
		qDebug() << m_hSocket->errorString();
	}
}

void QueryMaster::SendInfoQuery(ServerInfo* pServer, int iChallenge) {
	if(pServer == nullptr)
		return;

	QByteArray sendPacket;
	sendPacket.resize(25+4);
	memcpy(sendPacket.begin(), "\xFF\xFF\xFF\xFF" "TSource Engine Query", 25);
	memcpy(sendPacket.begin() + 25, &iChallenge, 4);
	quint64 iBytesSent = m_hSocket->writeDatagram(sendPacket, pServer->m_hAddress, pServer->m_iPort);
	if(iBytesSent < 0) {
		qDebug() << m_hSocket->error();
		qDebug() << m_hSocket->errorString();
	}
}

void QueryMaster::SendPlayersQuery(ServerInfo* pServer) {
	QueryMaster::SendPlayersQuery(pServer, -1);
}

void QueryMaster::SendPlayersQuery(ServerInfo* pServer, int iChallenge) {
	if(pServer == nullptr)
		return;

	QByteArray sendPacket;
	sendPacket.resize(5+4);
	memcpy(sendPacket.begin(), "\xFF\xFF\xFF\xFF" "U", 5);
	memcpy(sendPacket.begin() + 5, &iChallenge, 4);
	quint64 iBytesSent = m_hSocket->writeDatagram(sendPacket, pServer->m_hAddress, pServer->m_iPort);
	if(iBytesSent < 0) {
		qDebug() << m_hSocket->error();
		qDebug() << m_hSocket->errorString();
	}
}

void QueryMaster::SendRulesQuery(ServerInfo* pServer) {
	QueryMaster::SendRulesQuery(pServer, -1);
}

void QueryMaster::SendRulesQuery(ServerInfo* pServer, int iChallenge) {
	if(pServer == nullptr)
		return;

	QByteArray sendPacket;
	sendPacket.resize(5+4);
	memcpy(sendPacket.begin(), "\xFF\xFF\xFF\xFF" "V", 5);
	memcpy(sendPacket.begin() + 5, &iChallenge, 4);
	quint64 iBytesSent = m_hSocket->writeDatagram(sendPacket, pServer->m_hAddress, pServer->m_iPort);
	if(iBytesSent < 0) {
		qDebug() << m_hSocket->error();
		qDebug() << m_hSocket->errorString();
	}
}

ServerInfo* QueryMaster::FindServerFromAddress(QHostAddress hAddr, quint16 iPort) {
	for(auto it = m_aServers.begin(); it != m_aServers.end(); ++it) {
		if((*it)->m_hAddress.toIPv4Address() == hAddr.toIPv4Address() && (*it)->m_iPort == iPort)
			return (*it);
	}
	return nullptr;
}

void QueryMaster::ClearServerList() {
	for(auto it = m_aServers.begin(); it != m_aServers.end(); ++it) {
		delete *it;
	}

	m_aServers.clear();
	//m_aReadyServers.clear();
	m_strSeed = "0.0.0.0:0";
}

void QueryMaster::UpdateServer(int iServer) {
	ServerInfo* pServer = m_aServers[iServer];
	pServer->m_timeQueryStart = std::chrono::system_clock::now();
	pServer->m_bReadyInfo = false;
	pServer->m_bReadyPlayers = false;

	SendInfoQuery(pServer);
	SendPlayersQuery(pServer);
}

void QueryMaster::UpdateServers() {
	for(auto it = m_aServers.begin(); it != m_aServers.end(); ++it) {
		UpdateServer((*it)->m_iInternalID);
	}
}

void QueryMaster::MakeFavoriteFromAddr(std::pair<quint32, quint16> addr) {
	ServerInfo* pExistingServer = FindServerFromAddress(QHostAddress(addr.first), addr.second);
	if(pExistingServer)
		return;

	ServerInfo* pServer = new ServerInfo;
	pServer->m_hAddress = QHostAddress(addr.first);
	pServer->m_iPort = addr.second;
	pServer->m_iInternalID = m_aServers.size();
	pServer->m_timeQueryStart = std::chrono::system_clock::now();
	pServer->m_bFavorited = true;

	m_aServers.push_back(pServer);

	SendInfoQuery(pServer);
	SendPlayersQuery(pServer);
}

void QueryMaster::LoadCachedServer(std::pair<quint32, quint16> addr) {
	ServerInfo* pExistingServer = FindServerFromAddress(QHostAddress(addr.first), addr.second);
	if(pExistingServer)
		return;

	ServerInfo* pServer = new ServerInfo;
	pServer->m_hAddress = QHostAddress(addr.first);
	pServer->m_iPort = addr.second;
	pServer->m_iInternalID = m_aServers.size();
	pServer->m_timeQueryStart = std::chrono::system_clock::now();
	pServer->m_bCached = true;

	m_aServers.push_back(pServer);

	SendInfoQuery(pServer);
	SendPlayersQuery(pServer);
}

void QueryMaster::QueryAddress(QHostAddress hAddress, quint16 iPort) {
	ServerInfo* pExistingServer = FindServerFromAddress(hAddress, iPort);

	if(pExistingServer) {
		UpdateServer(pExistingServer->m_iInternalID);
		return;
	}

	ServerInfo* pServer = new ServerInfo;

	pServer->m_hAddress = hAddress;
	pServer->m_iPort = iPort;
	pServer->m_iInternalID = m_aServers.size();
	pServer->m_timeQueryStart = std::chrono::system_clock::now();
	SendInfoQuery(pServer);

	m_aServers.push_back(pServer);
}

void QueryMaster::CheckForTimeouts() {
	if(!m_bReceivedMasterPacket) {
		qDebug() << "Master server timed out, querying another...";
		++m_iStoredAddressesIndex;
		if((m_iStoredAddressesIndex) >= m_aStoredMasterAddresses.size())
			m_iStoredAddressesIndex %= m_aStoredMasterAddresses.size();

		QueryMasterServer();
	}
}

// risky
void QueryMaster::RemoveServer(int iServer) {
	for(int i = iServer+1; i < m_aServers.size(); ++i) {
		m_aServers[i]->m_iInternalID -= 1;
	}
	m_aServers.erase(m_aServers.begin() + iServer);
}
