#include "suplogin.h"
#include "ui_suplogin.h"

SupLogin::SupLogin(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SupLogin)
{
  ui->setupUi(this);
}

SupLogin::~SupLogin()
{
  delete ui;
}

void SupLogin::clear()
{
  ui->lineEditServer->setText("127.0.0.1");
  ui->lineEditLogin->clear();
  ui->lineEditPassword->clear();
}

void SupLogin::on_buttonBox_accepted()
{
  emit signConectar(ui->lineEditServer->text(), ui->lineEditLogin->text(), ui->lineEditPassword->text());
}

