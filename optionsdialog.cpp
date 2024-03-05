#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "mainwindow.h"
#include <QFileDialog>

OptionsDialog::OptionsDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::OptionsDialog)
{
	ui->setupUi(this);

	m_Config = &((reinterpret_cast<MainWindow*>(parent))->m_Config);
	m_ptimerNotif = &((reinterpret_cast<MainWindow*>(parent))->m_timerNotificationsCooldown);
	m_ptimerFullAutoRefresh = &((reinterpret_cast<MainWindow*>(parent))->m_timerFullAutoRefresh);

	QCheckBox* pCheckUseSteamconnect = this->findChild<QCheckBox*>("checkSteamconnect");
	QCheckBox* pCheckTray = this->findChild<QCheckBox*>("checkTray");
	QCheckBox* pCheckCloseAfterConnect = this->findChild<QCheckBox*>("checkCloseAfterConnect");
	QCheckBox* pCheckEnableCaching = this->findChild<QCheckBox*>("checkEnableCache");

	QComboBox* pComboStyle = this->findChild<QComboBox*>("comboStyle");
	QPushButton* pButtonBrowseStyle = this->findChild<QPushButton*>("buttonBrowseStyle");
	QLineEdit* pInputCustomStyle = this->findChild<QLineEdit*>("inputCustomStyle");

	QComboBox* pComboDCAction = this->findChild<QComboBox*>("comboDCAction");
	QDoubleSpinBox* pSpinInterval = this->findChild<QDoubleSpinBox*>("spinRefreshInterval");
	QCheckBox* pCheckFullRefresh = this->findChild<QCheckBox*>("checkFullAutoRefresh");
	QDoubleSpinBox* pSpinFullRefreshInterval = this->findChild<QDoubleSpinBox*>("spinFullRefreshInterval");
	QLabel* pLabelFullRefreshInterval = this->findChild<QLabel*>("labelFullRefreshSpin");

	QCheckBox* pCheckNotifications = this->findChild<QCheckBox*>("checkEnableNotifications");
	QCheckBox* pCheckNotifFavOnly = this->findChild<QCheckBox*>("checkNotifFavOnly");
	QSpinBox* pSpinNotifThreshold = this->findChild<QSpinBox*>("spinNotifThreshold");
	QSpinBox* pSpinNotifCooldown = this->findChild<QSpinBox*>("spinNotifCooldown");
	QLabel* pLabelNotifThreshold = this->findChild<QLabel*>("labelNotifThreshold");
	QLabel* pLabelNotifCooldown = this->findChild<QLabel*>("labelNotifCooldown");


	pCheckUseSteamconnect->setCheckState(m_Config->m_bUseSteamConnectme ? Qt::Checked : Qt::Unchecked);
	pCheckTray->setCheckState(m_Config->m_bUseTray ? Qt::Checked : Qt::Unchecked);
	pCheckCloseAfterConnect->setCheckState(m_Config->m_bCloseAfterConnect ? Qt::Checked : Qt::Unchecked);
	pCheckEnableCaching->setCheckState(m_Config->m_bEnableCaching ? Qt::Checked : Qt::Unchecked);

	pComboStyle->setCurrentIndex(m_Config->m_iStyleIndex);
	pInputCustomStyle->setText(m_Config->m_strStylePath);

	/*if(m_Config->m_strStyle != "Custom") {
		pInputCustomStyle->setEnabled(false);
		pButtonBrowseStyle->setEnabled(false);
	}*/

	pComboDCAction->setCurrentIndex(m_Config->m_iDoubleClickAction);
	pSpinInterval->setValue(m_Config->m_flAutoRefreshInterval);
	pCheckFullRefresh->setCheckState(m_Config->m_bFullAutoRefreshServers ? Qt::Checked : Qt::Unchecked);
	pSpinFullRefreshInterval->setValue(m_Config->m_flFullAutoRefreshInterval);
	pSpinFullRefreshInterval->setEnabled(m_Config->m_bFullAutoRefreshServers);
	pLabelFullRefreshInterval->setEnabled(m_Config->m_bFullAutoRefreshServers);

	pCheckNotifications->setCheckState(m_Config->m_bEnableNotifications ? Qt::Checked : Qt::Unchecked);
	pCheckNotifFavOnly->setCheckState(m_Config->m_bNotificationFavoritesOnly ? Qt::Checked : Qt::Unchecked);
	pSpinNotifThreshold->setValue(m_Config->m_iNotificationPlayerThreshold);
	pSpinNotifCooldown->setValue(m_Config->m_iNotificationCooldownMin);

	pLabelNotifThreshold->setEnabled(m_Config->m_bEnableNotifications);
	pLabelNotifCooldown->setEnabled(m_Config->m_bEnableNotifications);
	pCheckNotifFavOnly->setEnabled(m_Config->m_bEnableNotifications);
	pSpinNotifThreshold->setEnabled(m_Config->m_bEnableNotifications);
	pSpinNotifCooldown->setEnabled(m_Config->m_bEnableNotifications);
}

OptionsDialog::~OptionsDialog()
{
	delete ui;
}

void OptionsDialog::on_checkSteamconnect_stateChanged(int arg1)
{
	m_Config->m_bUseSteamConnectme = arg1;
	m_Config->SaveConfig();
}


void OptionsDialog::on_spinRefreshInterval_valueChanged(double arg1)
{
	m_Config->m_flAutoRefreshInterval = arg1;
	m_Config->SaveConfig();
}

void OptionsDialog::on_comboStyle_currentIndexChanged(int index)
{
	QComboBox* pComboStyle = this->findChild<QComboBox*>("comboStyle");
	QLineEdit* pInputCustomStyle = this->findChild<QLineEdit*>("inputCustomStyle");
	QPushButton* pButtonBrowseStyle = this->findChild<QPushButton*>("buttonBrowseStyle");
	m_Config->m_iStyleIndex = index;
	if(pComboStyle->itemText(index) == "Custom") {
		pInputCustomStyle->setEnabled(true);
		pButtonBrowseStyle->setEnabled(true);
		m_Config->m_strStyle = "Custom";
		m_Config->m_strStylePath = pInputCustomStyle->text();
	} else {
		pInputCustomStyle->setEnabled(false);
		pButtonBrowseStyle->setEnabled(false);
		m_Config->m_strStyle = pComboStyle->itemText(index);
	}
	((MainWindow*)parent())->OnStyleChanged(m_Config->m_strStyle);
	m_Config->SaveConfig();
}

void OptionsDialog::on_checkCloseAfterConnect_stateChanged(int arg1)
{
	m_Config->m_bCloseAfterConnect = arg1;
	m_Config->SaveConfig();
}

void OptionsDialog::on_checkTray_stateChanged(int arg1)
{
	m_Config->m_bUseTray = arg1;
	m_Config->SaveConfig();

	((MainWindow*)parent())->m_tray.setVisible(arg1);
}


void OptionsDialog::on_comboDCAction_currentIndexChanged(int index)
{
	m_Config->m_iDoubleClickAction = index;
	m_Config->SaveConfig();
}


void OptionsDialog::on_buttonBrowseStyle_clicked()
{
	QString strFilePath = QFileDialog::getOpenFileName(this, "Open style sheet", "", "QSS Files (*.qss)");
	if(strFilePath.length() < 1)
		return;

	QLineEdit* pInputCustomStyle = this->findChild<QLineEdit*>("inputCustomStyle");
	pInputCustomStyle->setText(strFilePath);
}


void OptionsDialog::on_inputCustomStyle_textChanged(const QString &arg1)
{
	m_Config->m_strStylePath = arg1;
	((MainWindow*)parent())->OnStyleChanged(m_Config->m_strStyle);
	m_Config->SaveConfig();
}


void OptionsDialog::on_checkEnableCache_stateChanged(int arg1)
{
	m_Config->m_bEnableCaching = arg1;
	m_Config->SaveConfig();
}


void OptionsDialog::on_checkEnableNotifications_stateChanged(int arg1)
{
	m_Config->m_bEnableNotifications = arg1;
	this->findChild<QCheckBox*>("checkNotifFavOnly")->setEnabled(arg1);
	this->findChild<QSpinBox*>("spinNotifThreshold")->setEnabled(arg1);
	this->findChild<QSpinBox*>("spinNotifCooldown")->setEnabled(arg1);
	this->findChild<QLabel*>("labelNotifThreshold")->setEnabled(arg1);
	this->findChild<QLabel*>("labelNotifCooldown")->setEnabled(arg1);

	if(m_Config->m_bEnableNotifications && ((MainWindow*)parent())->m_actTrayToggleNotifications)
		((MainWindow*)parent())->m_actTrayToggleNotifications->setText("Disable notifications");
	else
		((MainWindow*)parent())->m_actTrayToggleNotifications->setText("Enable notifications");

	m_Config->SaveConfig();
}


void OptionsDialog::on_checkFullAutoRefresh_stateChanged(int arg1)
{
	m_Config->m_bFullAutoRefreshServers = arg1;
	this->findChild<QDoubleSpinBox*>("spinFullRefreshInterval")->setEnabled(arg1);
	this->findChild<QLabel*>("labelFullRefreshSpin")->setEnabled(arg1);
	m_ptimerFullAutoRefresh->stop();

	if(m_Config->m_bFullAutoRefreshServers)
		m_ptimerFullAutoRefresh->start();

	m_Config->SaveConfig();
}


void OptionsDialog::on_spinFullRefreshInterval_valueChanged(double arg1)
{
	m_Config->m_flFullAutoRefreshInterval = arg1;
	m_ptimerFullAutoRefresh->stop();
	if(m_Config->m_bFullAutoRefreshServers) {
		m_ptimerFullAutoRefresh->setInterval(m_Config->m_flFullAutoRefreshInterval * 1000.0);
		m_ptimerFullAutoRefresh->start();
	}

	m_Config->SaveConfig();
}


void OptionsDialog::on_spinNotifThreshold_valueChanged(int arg1)
{
	m_Config->m_iNotificationPlayerThreshold = arg1;
	m_Config->SaveConfig();
}


void OptionsDialog::on_checkNotifFavOnly_stateChanged(int arg1)
{
	m_Config->m_bNotificationFavoritesOnly = arg1;
	m_Config->SaveConfig();
}


void OptionsDialog::on_spinNotifCooldown_valueChanged(int arg1)
{
	m_Config->m_iNotificationCooldownMin = arg1;
	m_Config->SaveConfig();
}

