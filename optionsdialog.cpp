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

	QCheckBox* pCheckUseSteamconnect = this->findChild<QCheckBox*>("checkSteamconnect");
	QCheckBox* pCheckCloseAfterConnect = this->findChild<QCheckBox*>("checkCloseAfterConnect");
	QCheckBox* pCheckEnableCaching = this->findChild<QCheckBox*>("checkEnableCache");
	QCheckBox* pCheckTray = this->findChild<QCheckBox*>("checkTray");
	QDoubleSpinBox* pSpinInterval = this->findChild<QDoubleSpinBox*>("spinRefreshInterval");
	QComboBox* pComboStyle = this->findChild<QComboBox*>("comboStyle");
	QLineEdit* pInputCustomStyle = this->findChild<QLineEdit*>("inputCustomStyle");
	QComboBox* pComboDCAction = this->findChild<QComboBox*>("comboDCAction");
	QPushButton* pButtonBrowseStyle = this->findChild<QPushButton*>("buttonBrowseStyle");

	pCheckUseSteamconnect->setCheckState(m_Config->m_bUseSteamConnectme ? Qt::Checked : Qt::Unchecked);
	pSpinInterval->setValue(m_Config->m_flAutoRefreshInterval);
	pComboStyle->setCurrentIndex(m_Config->m_iStyleIndex);
	pInputCustomStyle->setText(m_Config->m_strStylePath);
	pCheckCloseAfterConnect->setCheckState(m_Config->m_bCloseAfterConnect ? Qt::Checked : Qt::Unchecked);
	pCheckTray->setCheckState(m_Config->m_bUseTray ? Qt::Checked : Qt::Unchecked);
	pCheckEnableCaching->setCheckState(m_Config->m_bEnableCaching ? Qt::Checked : Qt::Unchecked);
	pComboDCAction->setCurrentIndex(m_Config->m_iDoubleClickAction);

	if(m_Config->m_strStyle != "Custom") {
		pInputCustomStyle->setEnabled(false);
		pButtonBrowseStyle->setEnabled(false);
	}
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

