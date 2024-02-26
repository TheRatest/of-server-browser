#ifndef DATAREADWRITER_H
#define DATAREADWRITER_H

#include <QByteArray>
#include <QString>

class DataReadWriter
{
public:
	DataReadWriter();

	QString ReadString(QByteArray&, quint32&);
	qint8 ReadInt8(QByteArray&, quint32&);
	qint16 ReadInt16(QByteArray&, quint32&);
	qint32 ReadInt32(QByteArray&, quint32&);
	qint64 ReadInt64(QByteArray&, quint32&);
	float ReadFloat(QByteArray&, quint32&);

	void WriteString(QByteArray&, QString&);
	void WriteInt8(QByteArray&, qint8);
	void WriteInt16(QByteArray&, qint16);
	void WriteInt32(QByteArray&, qint32);
	void WriteInt64(QByteArray&, qint64);
	void WriteFloat(QByteArray&, float);
};

#endif // DATAREADWRITER_H
