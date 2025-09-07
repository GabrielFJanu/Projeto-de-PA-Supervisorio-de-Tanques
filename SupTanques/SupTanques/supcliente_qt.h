#ifndef SUPCLIENTE_QT_H
#define SUPCLIENTE_QT_H

#include <QMainWindow>
#include "suplogin.h"
#include "supimg.h"
#include "supcliente.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SupClienteQt; }
QT_END_NAMESPACE

class SupClienteQt : public QMainWindow, public SupCliente /* ACRESCENTAR */
{
  Q_OBJECT

public:
  explicit SupClienteQt(QWidget *parent = nullptr);
  ~SupClienteQt();

// As funcoes virtuais que precisam ser implementadas na interface
private:

  // Exibe informacao de erro
  void virtExibirErro(const std::string& msg) const override;
  // Redesenha toda a interface (chegada de dados, desconexao, etc)
  void virtExibirInterface() const override;

  // As funcoes virtuais de armazenamento de dados.
  // Armazena o ultimo estado atual da planta
  void storeState(const SupState& lastS) override;
  // Limpa todos os estados armazenados da planta
  void clearState() override;

signals:
  // Sinaliza a necessidade de exibir informacao de erro
  void signExibirErro(const std::string& msg) const;
  // Sinaliza a necessidade de exibir dados recebidos
  void signExibirInterface() const;

  // Sinaliza a necessidade de incluir o ultimo ponto (que jah estah armazenado) na imagem
  void signStoreState(const SupState& lastS) const;
  // Sinaliza a necessidade de limpar todos os pontos (qhe jah foram apagados) da imagem
  void signClearState() const;

private slots:
  void on_actionLogin_triggered();
  void on_actionLogout_triggered();
  void on_actionQuit_triggered();
  void on_buttonV1_clicked(bool open);
  void on_buttonV2_clicked(bool open);
  void on_sliderPump_valueChanged(int value);
  void on_showLevel_toggled(bool checked);
  void on_spinRefresh_valueChanged(int arg1);

  // Conectar ao servidor
  void slotConectar(QString IP, QString Login, QString Senha);

  // Exibe uma janela pop-up com mensagem de erro
  void slotExibirErro(const std::string& msg);

  // Redesenha a interface
  void slotExibirInterface();

  // Inclui o ultimo estado atual da planta (que jah estah armazenado) na imagem
  void slotStoreState(const SupState& lastS);
  // Limpa todos os pontos (que jah foram apagados) da imagem
  void slotClearState();

// As funcoes privadas da classe
private:
  // Exibir os dados recebidos
  void showValves(uint16_t V1, uint16_t V2);
  void showH(uint16_t H1, uint16_t H2, uint16_t OverFlow);
  void showPump(uint16_t PInput);
  void showFlow(uint16_t Flow);

// Os dados privados da classe
private:
  Ui::SupClienteQt *ui;

  // A janela de login
  SupLogin *loginWindow;

  // O widget da barra de status
  QLabel *statusMsg;

  // A imagem para exibicao do grafico
  SupImg* image;

};
#endif // SUPCLIENTE_QT_H
