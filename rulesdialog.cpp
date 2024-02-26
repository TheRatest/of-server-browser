#include "rulesdialog.h"
#include "ui_rulesdialog.h"

RulesDialog::RulesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RulesDialog)
{
    ui->setupUi(this);
}

RulesDialog::~RulesDialog()
{
    delete ui;
}
