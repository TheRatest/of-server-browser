#ifndef QUERYMASTER_H
#define QUERYMASTER_H
#include <vector>
#include <QUdpSocket>
#include <chrono>
#include <QHostInfo>
#include "datareadwriter.h"

#define HL2MASTER_HOST "hl2master.steampowered.com"
#define HL2MASTER_PORT 27011

// https://developer.valvesoftware.com/wiki/Server_queries#A2S_INFO
// https://developer.valvesoftware.com/wiki/Master_Server_Query_Protocol

class ServerPlayer {
public:
	QString m_strName = "None";
	int m_iScore = 0;
	float m_flDuration = 0.0;

	QString DurationString();
private:
	int m_iIndex = 0;
};

class ServerInfo : private DataReadWriter {
public:
	ServerInfo();

	QHostAddress m_hAddress = QHostAddress("0.0.0.0");
	QString m_strName = "None";
	QString m_strMap = "None";
	QString m_strGame = "None";
	QString m_strGameFolder = "None";
	QString m_strVersion = "None";
	QString m_strOS = "None";
	QString m_strType = "None";
	int m_iRevision = -1;
	qint64 m_iLatencyMs = 0;
	short m_iSteamAppID = 0;
	short m_iProtocol = 0;
	short m_iPlayers = 0;
	short m_iBots = 0;
	short m_iMaxPlayers = 0;
	quint16 m_iPort = 0;
    // can be different from the game port, is important for caching..
    quint16 m_iQueryPort = 0;
	bool m_bPassworded = false;
	bool m_bVAC = false;
	// extra data
	int m_nExtraDataFlags = 0;
	long long m_iSteamID = 0;
	short m_iSourceTVPort = 0;
	QString m_strSourceTVName = "None";
	QString m_strTags = "None";
	long long m_iGameID = 0;

	std::vector<std::pair<QString, QString>> m_aRules;
	std::vector<ServerPlayer> m_aPlayers;

	qint32 m_iInternalID = 0;
	bool m_bFavorited = false;
	bool m_bCached = false;
	bool m_bBogusServer = false;

	bool m_bReadyInfo = false;
	bool m_bReadyPlayers = false;
	bool m_bQueryingRules = false;
	bool m_bReadyRules = false;
	bool m_bReady = false;

	bool IsOpenFortressServer();

	void ParseInfoPacket(QByteArray rawData);
	void ParsePlayersPacket(QByteArray rawData);
	void ParseRulesPacket(QByteArray rawData);
	// multi-packet stuff
	void ParseStitchedRules();
	// returns true if the rules are fully parsed
	bool QueueRulesPacket(QByteArray rawData);

	std::chrono::time_point<std::chrono::system_clock> m_timeQueryStart;
private:
	std::vector<std::pair<qint8, QByteArray>> m_aRulesPackets;
	QByteArray m_rulesRaw;
	// the rules are usually split into multiple packets
	qint8 m_iRulesPacketsCount = 0;
	qint8 m_iRulesPacketsReceived = 0;
	qint32 m_iRulesPacketsID = 0;
};

class QueryMaster : public QObject
{
	// for signal support
	Q_OBJECT;
public:

	QueryMaster(QObject* pParent = nullptr);

    void QueryMasterServer(QString strGameFolder = "open_fortress");
	void UpdateServers();
	void UpdateServer(int);
	void RemoveServer(int);
	void ClearServerList();

	void LoadCachedServer(std::pair<quint32, quint16>);
	void MakeFavoriteFromAddr(std::pair<quint32, quint16>);
	void QueryAddress(QHostAddress, quint16);
	ServerInfo* FindServerFromAddress(QHostAddress hAddr, quint16 iPort);

	std::vector<ServerInfo*> m_aServers;

    QString m_strGameFolder = "open_fortress";
signals:
	void ServerIsReady(ServerInfo*);
	void ServerUpdated(ServerInfo*);
	void ServerNeedsRemoval(ServerInfo*);
public slots:
	void ReadPendingPackets();
	void CheckForTimeouts();
private:
	// if the server list comes in multiple packets
	QString m_strSeed = "0.0.0.0:0";
	void SendInfoQuery(ServerInfo* pServer);
    void SendInfoQuery(ServerInfo* pServer, int iChallenge);
    void SendRulesQuery(ServerInfo* pServer, int iChallenge = -1);
    void SendPlayersQuery(ServerInfo* pServer, int iChallenge = -1);
	QList<QHostAddress> m_aStoredMasterAddresses = QHostInfo::fromName(HL2MASTER_HOST).addresses();
	quint16 m_iStoredAddressesIndex = 0;
	QHostAddress m_hMasterAddress;
	QUdpSocket* m_hSocket;
    const quint32 m_iTimeoutThresholdMs = 1000;

	bool m_bFinishedQueryingMaster = true;
	// to decide which servers are faulty
	bool m_bReceivedMasterPacket = true;

    bool IsCorrectGameServer(ServerInfo*);

	// for debugging purposes so that i dont get rate limited by the valve servers
	const bool m_bLocalOnly = false;
	void QueryLocalServer();
};

enum ResponseType {
	RESPONSE_CHALLENGE = 'A',
	RESPONSE_INFO = 'I',
	RESPONSE_PLAYERS = 'D',
	RESPONSE_RULES = 'E',
	RESPONSE_SERVERS = 'f'
};

#endif // QUERYMASTER_H
