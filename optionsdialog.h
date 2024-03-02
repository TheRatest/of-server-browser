#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "configmaster.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit OptionsDialog(QWidget *parent = nullptr);
	~OptionsDialog();

private slots:
	void on_checkSteamconnect_stateChanged(int arg1);

	void on_spinRefreshInterval_valueChanged(double arg1);

	void on_comboStyle_currentIndexChanged(int index);

	void on_checkCloseAfterConnect_stateChanged(int arg1);

	void on_checkTray_stateChanged(int arg1);

	void on_comboDCAction_currentIndexChanged(int index);

	void on_buttonBrowseStyle_clicked();

	void on_inputCustomStyle_textChanged(const QString &arg1);

	void on_checkEnableCache_stateChanged(int arg1);

private:
	Ui::OptionsDialog *ui;

	ConfigMaster* m_Config;
};

#endif // OPTIONSDIALOG_H
