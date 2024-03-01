#include "queryipdialog.h"
#include "ui_queryipdialog.h"
#include <QRegularExpression>
#include "mainwindow.h"

QueryIPDialog::QueryIPDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::QueryIPDialog)
{
	ui->setupUi(this);
}

QueryIPDialog::~QueryIPDialog()
{
	delete ui;
}

void QueryIPDialog::on_buttonBox_rejected()
{
	m_hAddrOut.setAddress((quint32)0);
	m_iPortOut = 0;
	this->close();
}


void QueryIPDialog::on_buttonBox_accepted()
{
	QueryMaster* queryMaster = ((MainWindow*)this->parent())->m_QueryMaster;

	QString strAddr = this->findChild<QLineEdit*>("inputAddress")->text();
	qDebug() << strAddr;
	if(strAddr.length() < 5) {
		on_buttonBox_rejected();
		return;
	}
	static QRegularExpression regexIPPort("(\\b25[0-5]|\\b2[0-4][0-9]|\\b[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}:[0-9]{1,6}");
	static QRegularExpression regexIP("(\\b25[0-5]|\\b2[0-4][0-9]|\\b[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}");
	if(regexIPPort.match(strAddr).hasMatch()) {
		qDebug() << "ip:port match";
		QString strIP = strAddr.split(":")[0];
		quint16 iPort = strAddr.split(":")[1].toUShort();
		qDebug() << strIP << iPort;
		m_hAddrOut.setAddress(strIP);
		m_iPortOut = iPort;
		queryMaster->QueryAddress(m_hAddrOut, m_iPortOut);
		this->close();
		return;
	}
	if(regexIP.match(strAddr).hasMatch()) {
		qDebug() << "ip match";
		m_hAddrOut.setAddress(strAddr);
		m_iPortOut = 27015;
		qDebug() << m_hAddrOut << 27015;
		queryMaster->QueryAddress(m_hAddrOut, m_iPortOut);
		this->close();
		return;
	}

	m_hAddrOut = QHostInfo::fromName(strAddr).addresses().first();
	qDebug() << m_hAddrOut.toString();
	m_iPortOut = 27015;
	queryMaster->QueryAddress(m_hAddrOut, m_iPortOut);

	this->close();
}

