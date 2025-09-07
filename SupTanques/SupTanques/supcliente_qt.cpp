#include <QMessageBox>
#include <cmath>          // pow, round
#include <chrono>         // std::chrono::seconds
#include "tanques-param.h"
#include "supcliente_qt.h"
#include "ui_supcliente_qt.h"


// Arredonda um numero para N casas decimais apos o ponto
static double roundN(double val, int N)
{
  double potencia = std::pow(10.0, N);
  return std::round(potencia*val)/potencia;
}

SupClienteQt::SupClienteQt(QWidget *parent)
  : QMainWindow(parent)
  /* ACRESCENTAR */
  , SupCliente()
  , ui(new Ui::SupClienteQt)
  , loginWindow(new SupLogin(this))
  , statusMsg(new QLabel(this))
  , image(new SupImg(this))
{
  ui->setupUi(this);

  // A imagem
  ui->horizontalLayout->insertWidget(0,image);

  // Os titulos das secoes do supervisorio
  ui->labelActuators->setStyleSheet("background-color: white");
  ui->labelSensors->setStyleSheet("background-color: white");
  ui->labelVisualization->setStyleSheet("background-color: white");

  // Os botoes push botton das valvulas

  // Sao checaveis (alternam on/off)
  ui->buttonV1->setCheckable(true);
  ui->buttonV2->setCheckable(true);
  // Valvula fechada (nao checado) - cinza esverdeado, valvula aberta (checado) - cinza avermelhado
  ui->buttonV1->setStyleSheet("QPushButton {background-color: #F0FFF0;"
                              "border-style: outset;"
                              "border-width: 2px;"
                              "padding: 3px}"
                              "QPushButton:checked { background-color: #FFF0F0;"
                              "border-style: inset;"
                              "border-width: 2px;"
                              "padding: 3px}");
  ui->buttonV2->setStyleSheet("QPushButton {background-color: #F0FFF0;"
                              "border-style: outset;"
                              "border-width: 2px;"
                              "padding: 3px}"
                              "QPushButton:checked { background-color: #FFF0F0;"
                              "border-style: inset;"
                              "border-width: 2px;"
                              "padding: 3px}");

  // O slider da bomba

  ui->sliderPump->setMinimum(0);
  ui->sliderPump->setMaximum(65535);
  ui->sliderPump->setPageStep(1000);
  // Soh gera um unico sinal valueChanged ao final do movimento do slider
  ui->sliderPump->setTracking(false);

  // Os displays LCD

  // Cor de fundo branco
  qApp->setStyleSheet("QLCDNumber { background-color: white }");
  // Paleta de cores para todos os displays (exceto overflow)
  auto paleta = ui->lcdH1Cm->palette();
  paleta.setColor(paleta.WindowText, Qt::red);
  // Numeros nao ressaltados (flat), 5 digitos, ponto ocupa um digito, paleta letra vermelha
  // H1
  ui->lcdH1Cm->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdH1Cm->setDigitCount(5);
  ui->lcdH1Cm->setSmallDecimalPoint(false);
  ui->lcdH1Cm->setPalette(paleta);
  ui->lcdH1Perc->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdH1Perc->setDigitCount(5);
  ui->lcdH1Perc->setSmallDecimalPoint(false);
  ui->lcdH1Perc->setPalette(paleta);
  // H2
  ui->lcdH2Cm->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdH2Cm->setDigitCount(5);
  ui->lcdH2Cm->setSmallDecimalPoint(false);
  ui->lcdH2Cm->setPalette(paleta);
  ui->lcdH2Perc->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdH2Perc->setDigitCount(5);
  ui->lcdH2Perc->setSmallDecimalPoint(false);
  ui->lcdH2Perc->setPalette(paleta);
  // Bomba
  ui->lcdPumpVal->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdPumpVal->setDigitCount(5);
  ui->lcdPumpVal->setSmallDecimalPoint(false);
  ui->lcdPumpVal->setPalette(paleta);
  ui->lcdPumpPerc->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdPumpPerc->setDigitCount(5);
  ui->lcdPumpPerc->setSmallDecimalPoint(false);
  ui->lcdPumpPerc->setPalette(paleta);
  // Vazao
  ui->lcdFlowLMin->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdFlowLMin->setDigitCount(5);
  ui->lcdFlowLMin->setSmallDecimalPoint(false);
  ui->lcdFlowLMin->setPalette(paleta);
  ui->lcdFlowPerc->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdFlowPerc->setDigitCount(5);
  ui->lcdFlowPerc->setSmallDecimalPoint(false);
  ui->lcdFlowPerc->setPalette(paleta);
  // overflow
  // Fundo vermelho, numeros nao ressaltados (flat), 5 digitos, paleta letra branca,
  // conteudo texto (nao numeros), widget normalmente oculto
  ui->lcdOverflow->setStyleSheet("background-color: red");
  ui->lcdOverflow->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdOverflow->setDigitCount(5);
  paleta.setColor(paleta.WindowText, Qt::white);
  ui->lcdOverflow->setPalette(paleta);
  ui->lcdOverflow->display("O-FLO");

  // A barra de status
  statusBar()->addWidget(statusMsg);

  // A conexao dos sinais e slots

  // Os sinais da SupClienteQt
  connect(this, &SupClienteQt::signExibirErro,
          this, &SupClienteQt::slotExibirErro);
  connect(this, &SupClienteQt::signExibirInterface,
          this, &SupClienteQt::slotExibirInterface);
  connect(this, &SupClienteQt::signStoreState,
          this, &SupClienteQt::slotStoreState);
  connect(this, &SupClienteQt::signClearState,
          this, &SupClienteQt::slotClearState);

  // Os sinais da SupLogin
  connect(loginWindow, &SupLogin::signConectar,
          this, &SupClienteQt::slotConectar);

  // O icone da aplicacao
  QPixmap pixIcon;
  if (pixIcon.load("scada_icon.png","PNG"))
  {
    setWindowIcon(QIcon(pixIcon));
  }
  else if (pixIcon.load("..\\SupTanques\\scada_icon.png","PNG"))
  {
    setWindowIcon(QIcon(pixIcon));
  }

  // Exibe a interface
  slotExibirInterface();
}

SupClienteQt::~SupClienteQt()
{
  delete ui;
}

/// As funcoes virtuais que sao chamadas pela thread e tambem pelo programa principal.
/// Como podem ser chamadas pela thread, nao podem alterar diretamente a interface.
/// Devem emitir sinais que serao processados no programa principal.

/// Exibe informacao de erro
void SupClienteQt::virtExibirErro(const std::string& msg) const
{
  emit signExibirErro(msg);
}

/// Reexibe interface
void SupClienteQt::virtExibirInterface() const
{
  emit signExibirInterface();
}

/// As funcoes virtuais de armazenamento de dados.
/// Precisam complementar as funcoes existentes na classe base,
/// para acrescentar o armazenamento no historico de dados para o grafico.

/// Armazena o ultimo estado atual da planta
void SupClienteQt::storeState(const SupState& lastS)
{
    // Chama a funcao da classe base
    SupCliente::storeState(lastS);
    // Acrescenta o ponto no grafico.
    emit signStoreState(lastS);
}

/// Limpa todos os estados armazenados da planta
void SupClienteQt::clearState()
{
    // Chama a funcao da classe base
    SupCliente::clearState();
    // Limpa o grafico.
    emit signClearState();
}

void SupClienteQt::on_actionLogin_triggered()
{
  loginWindow->clear();
  loginWindow->show();
}

void SupClienteQt::on_actionLogout_triggered()
{
  // Desconecta caso esteja conectado
  // Esta funcao jah testa se estah conectado ou nao
  // e redesenha toda a interface.
  /* ACRESCENTAR */
  SupCliente::desconectar();
}

void SupClienteQt::on_actionQuit_triggered()
{
  // Desconecta caso esteja conectado.
  /* ACRESCENTAR */
  // Encerra a janela
  SupCliente::desconectar();
  QCoreApplication::quit();
}

void SupClienteQt::on_buttonV1_clicked(bool open)
{
  // Chama funcao que altera estado da valvula 1.
  // Jah exibe msg de erro em caso de insucesso.
  /* ACRESCENTAR */
  SupCliente::setV1Open(open);
}

void SupClienteQt::on_buttonV2_clicked(bool open)
{
  // Chama funcao que altera estado da valvula 2.
  // Jah exibe msg de erro em caso de insucesso.
  /* ACRESCENTAR */
  SupCliente::setV2Open(open);
}

void SupClienteQt::on_sliderPump_valueChanged(int value)
{
  // Chama funcao que altera entrada da bomba.
  // Jah exibe msg de erro em caso de insucesso.
  /* ACRESCENTAR */
  SupCliente::setPumpInput(static_cast<uint16_t>(value));

  // Exibe valores nos displays LCD
  ui->lcdPumpVal->display(value);
  double PumpPerc = roundN( (100.0*value)/UINT16_MAX, 2 );
  ui->lcdPumpPerc->display(PumpPerc);
}

void SupClienteQt::on_showLevel_toggled(bool checked)
{
  image->setDisplayMode(checked);
}

void SupClienteQt::on_spinRefresh_valueChanged(int arg1)
{
  // Altera o intervalo entre solicitacoes de dados
  /* ACRESCENTAR */
  SupCliente::setTimeRefresh(arg1);
}

/// Conectar ao servidor
void SupClienteQt::slotConectar(QString IP, QString Login, QString Senha)
{
  // Chama funcao que conecta com o servidor.
  // Jah reexibe interface em qualquer caso e exibe msg em caso de erro.
  /* ACRESCENTAR */
  SupCliente::conectar(IP.toStdString(), Login.toStdString(), Senha.toStdString());
}

// Exibir informacao de erro
void SupClienteQt::slotExibirErro(const std::string& msg)
{
  QMessageBox::critical(this, "SupTanques error", QString::fromStdString(msg));
}

void SupClienteQt::slotExibirInterface()
{
  // Guarda o estado do cliente na ultima vez que foi chamada essa funcao:
  //   =0 se eh a primeira vez que a funcao eh chamada;
  //   >0 se estava conectado;
  //   <0 se estava desconectado;
  // Se, na chamada atual, o estado for o mesmo que na chamada anterior,
  // entao algumas acoes nao precisam ser reexecutadas, tais como
  // habilitar ou desabilitar os widgets.
  static int estavaConectado=0;

  // Booleano que indica se cliente estah conectado ou nao.
  // Chama funcao que retorna essa informacao.
  bool estahConectado = SupCliente::isConnected(); //MODIFICADO
  // Booleano que indica se cliente eh administrador ou nao.
  // Chama funcao que retorna essa informacao.
  bool ehAdministrador = SupCliente::isAdmin(); //MODIFICADO

  // Redesenha toda a interface.
  // O redesenho eh diferente caso o cliente esteja conectado ou nao.
  if (estahConectado)
  {
    // Testa se nao estava conectado na execucao anterior
    if (estavaConectado <=0)
    {
      // Memoriza que estah conectado;
      estavaConectado = 1;

      // Desabilita opcao conectar
      ui->actionLogin->setEnabled(false);
      // Habilita opcao desconectar
      ui->actionLogout->setEnabled(true);

      // Habilita atuadores na planta, se for administrador
      if (ehAdministrador)
      {
        // Os botoes push botton
        ui->buttonV1->setEnabled(true);
        ui->buttonV2->setEnabled(true);
        // O slider da bomba
        ui->sliderPump->setEnabled(true);
      }

      // Habilita os botoes para escolher o modo de visualizacao
      ui->showLevel->setChecked(true);
      ui->showLevel->setEnabled(true);
      ui->showGraph->setEnabled(true);

      // Habilita o spin para escolher o periodo de visualizacao
      ui->spinRefresh->setEnabled(true);

      // Barra de status
      // Texto da mensagem varia se for administrador ou nao
      QString msg = QString(" CONNECTED: ") + QString::fromStdString(SupCliente::meuUsuario/* MODIFICADO */) +
          " (" + (ehAdministrador ? "admin" : "viewer") + ")";
      statusMsg->setText(msg);
    }

    // Exibe nos visualizadores o ultimo estado lido da planta.
    showValves(SupCliente::lastState().V1/* MODIFICADO */, SupCliente::lastState().V2/* MODIFICADO */);
    showPump(SupCliente::lastState().PumpInput/* MODIFICADO */);
    showH(SupCliente::lastState().H1/* MODIFICADO */, SupCliente::lastState().H2/* MODIFICADO */, SupCliente::lastState().ovfl/* MODIFICADO */);
    showFlow(SupCliente::lastState().PumpFlow/* MODIFICADO */);

    // Exibe a imagem
    image->drawImg();
  }
  else // Nao estah conectada
  {
    // Testa se nao estava desconectado na execucao anterior
    if (estavaConectado >=0)
    {
      // Memoriza que estah desconectado;
      estavaConectado = -1;

      // Habilita opcao conectar
      ui->actionLogin->setEnabled(true);
      // Desabilita opcao desconectar
      ui->actionLogout->setEnabled(false);

      // Limpa os visualizadores.
      // Feito feito antes de desabilitar os widgets.
      showValves(0,0);
      showPump(0);
      showH(0,0,0);
      showFlow(0);

      // Desabilita atuadores na planta
      // Os botoes push botton
      ui->buttonV1->setEnabled(false);
      ui->buttonV2->setEnabled(false);
      // O slider da bomba
      ui->sliderPump->setEnabled(false);

      // Esconde mostrador de overflow
      ui->lcdOverflow->hide();

      // Desabilita os botoes para escolher o modo de visualizacao
      ui->showLevel->setChecked(true);
      ui->showLevel->setEnabled(false);
      ui->showGraph->setEnabled(false);

      // Desabilita o spin para escolher o periodo de visualizacao
      ui->spinRefresh->setEnabled(false);

      // A barra de status
      statusMsg->setText(" DISCONNECTED");

      // Limpa a imagem
      image->clear();
    }
  }
}

/// Inclui o ultimo estado atual da planta (que jah estah armazenado) na imagem
void SupClienteQt::slotStoreState(const SupState& lastS)
{
    image->addPoint(deltaT(), lastS);
}

/// Limpa todos os pontos (que jah foram apagados) da imagem
void SupClienteQt::slotClearState()
{
    image->clear();
}

void SupClienteQt::showValves(uint16_t V1, uint16_t V2)
{
  // Altera os push buttom das valvulas.
  // Como vai alterar o estado do pushButtom com setChecked, precisa anter bloquear a geracao de sinais.
  // Senao, vai executar on_buttonV?_clicked, chamar setValve e enviar um comando para o servidor.
  //
  // V1
  ui->buttonV1->blockSignals(true);
  if (V1 != 0)
  {
    // Valvula aberta
    ui->buttonV1->setChecked(true);
    ui->buttonV1->setText("OPEN");
  }
  else
  {
    // Valvula fechada
    ui->buttonV1->setChecked(false);
    ui->buttonV1->setText("CLOSED");
  }
  ui->buttonV1->blockSignals(false);
  //
  // V2
  ui->buttonV2->blockSignals(true);
  if (V2 != 0)
  {
    // Valvula aberta
    ui->buttonV2->setChecked(true);
    ui->buttonV2->setText("OPEN");
  }
  else
  {
    // Valvula fechada
    ui->buttonV2->setChecked(false);
    ui->buttonV2->setText("CLOSED");
  }
  ui->buttonV2->blockSignals(false);
}

void SupClienteQt::showH(uint16_t H1, uint16_t H2, uint16_t OverFlow)
{
  // Exibe valores nos displays LCD
  // H1
  double H1Perc = roundN( (100.0*H1)/UINT16_MAX, 2 );
  ui->lcdH1Perc->display(H1Perc);
  double H1Cm = roundN( MaxTankLevelMeasurement*H1Perc, 2 );
  ui->lcdH1Cm->display(H1Cm);
  // H2
  double H2Perc = roundN( (100.0*H2)/UINT16_MAX, 2 );
  ui->lcdH2Perc->display(H2Perc);
  double H2Cm = roundN( MaxTankLevelMeasurement*H2Perc, 2 );
  ui->lcdH2Cm->display(H2Cm);
  // Overflow
  if (OverFlow != 0) ui->lcdOverflow->show(); // Estah transbordando
  else ui->lcdOverflow->hide();               // Nao estah transbordando
}

void SupClienteQt::showPump(uint16_t PInput)
{
  // Altera o valor do slider da entrada da bomba.
  // Como vai alterar o estado do slider com setValue, precisa anter bloquear a geracao de sinais.
  // Senao, vai executar on_sliderPump_valueChanged e enviar um comando para o servidor.
  ui->sliderPump->blockSignals(true);
  ui->sliderPump->setValue(PInput);
  ui->sliderPump->blockSignals(false);

  // Exibe valores nos displays LCD
  ui->lcdPumpVal->display(PInput);
  double PumpPerc = roundN( (100.0*PInput)/UINT16_MAX, 2 );
  ui->lcdPumpPerc->display(PumpPerc);
}

void SupClienteQt::showFlow(uint16_t Flow)
{
  double FlowPerc = roundN( (100.0*Flow)/UINT16_MAX, 2 );
  ui->lcdFlowPerc->display(FlowPerc);
  double FlowLMin = roundN( 600.0*MaxPumpFlowMeasurement*FlowPerc, 3 );
  ui->lcdFlowLMin->display(FlowLMin);
}
