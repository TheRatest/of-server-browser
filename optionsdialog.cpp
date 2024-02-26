#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "mainwindow.h"

OptionsDialog::OptionsDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::OptionsDialog)
{
	ui->setupUi(this);

	m_Config = &((reinterpret_cast<MainWindow*>(parent))->m_Config);

	QCheckBox* pCheckUseSteamconnect = this->findChild<QCheckBox*>("checkSteamconnect");
	QCheckBox* pCheckCloseAfterConnect = this->findChild<QCheckBox*>("checkCloseAfterConnect");
	QDoubleSpinBox* pSpinInterval = this->findChild<QDoubleSpinBox*>("spinRefreshInterval");
	QComboBox* pComboStyle = this->findChild<QComboBox*>("comboStyle");

	pCheckUseSteamconnect->setCheckState(m_Config->m_bUseSteamConnectme ? Qt::Checked : Qt::Unchecked);
	pSpinInterval->setValue(m_Config->m_flAutoRefreshInterval);
	pComboStyle->setCurrentIndex(m_Config->m_iStyleIndex);
	pCheckCloseAfterConnect->setCheckState(m_Config->m_bCloseAfterConnect ? Qt::Checked : Qt::Unchecked);
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
	m_Config->m_iStyleIndex = index;
	m_Config->m_strStyle = pComboStyle->itemText(index);
	((MainWindow*)parent())->OnStyleChanged(m_Config->m_strStyle);
	m_Config->SaveConfig();
}

void OptionsDialog::on_checkCloseAfterConnect_stateChanged(int arg1)
{
	m_Config->m_bCloseAfterConnect = arg1;
	m_Config->SaveConfig();
}

