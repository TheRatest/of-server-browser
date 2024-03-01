#ifndef QUERYIPDIALOG_H
#define QUERYIPDIALOG_H

#include <QDialog>
#include <QHostAddress>
#include <QHostInfo>
#include <QAbstractButton>

namespace Ui {
class QueryIPDialog;
}

class QueryIPDialog : public QDialog
{
	Q_OBJECT

public:
	explicit QueryIPDialog(QWidget *parent = nullptr);
	~QueryIPDialog();

	QHostAddress m_hAddrOut;
	quint16 m_iPortOut;
private slots:
	void on_buttonBox_rejected();

	void on_buttonBox_accepted();

private:
	Ui::QueryIPDialog *ui;
};

#endif // QUERYIPDIALOG_H
