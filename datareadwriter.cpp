#include "datareadwriter.h"
#include <QDebug>

DataReadWriter::DataReadWriter() {}

qint8 DataReadWriter::ReadInt8(QByteArray& rawData, quint32& iCursor) {
	qint8 iOut = *reinterpret_cast<qint8*>(&rawData[iCursor]);
	iCursor += sizeof(qint8);
	return iOut;
}

qint16 DataReadWriter::ReadInt16(QByteArray& rawData, quint32& iCursor) {
	qint16 iOut = *reinterpret_cast<qint16*>(&rawData[iCursor]);
	iCursor += sizeof(qint16);
	return iOut;
}

qint32 DataReadWriter::ReadInt32(QByteArray& rawData, quint32& iCursor) {
	qint32 iOut = *reinterpret_cast<qint32*>(&rawData[iCursor]);
	iCursor += sizeof(qint32);
	return iOut;
}

qint64 DataReadWriter::ReadInt64(QByteArray& rawData, quint32& iCursor) {
	qint64 iOut = *reinterpret_cast<qint64*>(&rawData[iCursor]);
	iCursor += sizeof(qint64);
	return iOut;
}

float DataReadWriter::ReadFloat(QByteArray& rawData, quint32& iCursor) {
	float iOut = *reinterpret_cast<float*>(&rawData[iCursor]);
	iCursor += sizeof(float);
	return iOut;
}

QString DataReadWriter::ReadString(QByteArray& rawData, quint32& iCursor) {
	QString strOut = QString::fromUtf8(reinterpret_cast<char*>(&rawData[iCursor]));
	std::string fuckYouUnicode = strOut.toStdString();
	iCursor += fuckYouUnicode.size() + 1;
	return strOut;
}

void DataReadWriter::WriteString(QByteArray& pData, QString& str) {
	char* pEnd = pData.end();
	std::string strUnicodeless = str.toStdString();
	pData.append(strUnicodeless.c_str(), strUnicodeless.length()+1);
}

void DataReadWriter::WriteInt8(QByteArray& pData, qint8 iVal) {
	pData.append((char*)&iVal, sizeof(qint8));
}

void DataReadWriter::WriteInt16(QByteArray& pData, qint16 iVal) {
	pData.append((char*)&iVal, sizeof(qint16));
}

void DataReadWriter::WriteInt32(QByteArray& pData, qint32 iVal) {
	pData.append((char*)&iVal, sizeof(qint32));
}

void DataReadWriter::WriteInt64(QByteArray& pData, qint64 iVal) {
	pData.append((char*)&iVal, sizeof(qint64));
}

void DataReadWriter::WriteFloat(QByteArray& pData, float flVal) {
	pData.append((char*)&flVal, sizeof(float));
}
