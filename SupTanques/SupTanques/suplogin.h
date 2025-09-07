#ifndef SUP_LOGIN_H
#define SUP_LOGIN_H

#include <QDialog>
#include <QString>

namespace Ui {
class SupLogin;
}

class SupLogin : public QDialog
{
  Q_OBJECT

public:
  explicit SupLogin(QWidget *parent = nullptr);
  ~SupLogin();

  void clear();

signals:
  void signConectar(QString, QString, QString);

private slots:
  void on_buttonBox_accepted();

private:
  Ui::SupLogin *ui;
};

#endif // SUP_LOGIN_H
