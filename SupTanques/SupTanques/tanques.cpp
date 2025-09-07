#include <iostream>     /* cerr */
#include <cmath>        /* sin, cos, log, sqrt, round */
#include <chrono>       /* std::chrono::seconds */
#include "tanques.h"
#include "supdados.h"

/// Constantes gerais: PI
const static double M_PI=3.14159265359;
/// Amplitudes dos ruidos de simulacao:
/// Ruido dinamico: percentual do valor sem ruido: 0.0 a 1.0
const static double percDynamicNoise=0.01;
/// Ruido de medicao: percentual do valor maximo medido: 0.0 a 1.0
const static double percMeasureNoise=0.005;

/// Gerador de variavel aleatoria com distribuicao normal, media 0.0, desvio padrao 1.0
/// Aproximacao pela transformada de Box–Muller
static double normal()
{
  double u1,z1;
  static double u2=-1.0, z2=0.0;
  int n_rand;

  if (u2 < 0.0)
  {
    do
    {
      n_rand = rand();     // 0 a RAND_MAX
    }
    while (n_rand == 0);   // 1 a RAND_MAX
    u1 = double(n_rand)/RAND_MAX;   // maior que 0.0 ateh igual a 1.0
    do
    {
      n_rand = rand();     // 0 a RAND_MAX
    }
    while (n_rand == 0);   // 1 a RAND_MAX
    u2 = double(n_rand)/RAND_MAX;   // maior que 0.0 ateh igual a 1.0

    double raio=sqrt(-2.0*log(u1)); // 0.0 a <inf
    double angulo=2.0*M_PI*u2;      // >0.0 a 2PI
    z1 = raio*cos(angulo);
    z2 = raio*sin(angulo);
    return z1;
  }
  /* else */
  u2 = -1.0;  // Recalcula na proxima vez
  return z2;
}

/// Construtor default
Tanks::Tanks():
  tanks_on(false),
  h1(0.0),
  h2(0.0),
  v1_open(false),
  v2_open(false),
  pump_input(0),
  flow_pump(0.0),
  is_overflowing(false),
  last_t(0),
  thr_simul()
{
  srand(time(nullptr));
}

/// Destrutor
Tanks::~Tanks()
{
  tanks_on = false;  // Deve parar a thread de simulacao
  // Espera o fim da thread
  if (thr_simul.joinable()) thr_simul.join();
}

/// Tanques ligados (true) ou desligados (false)
bool Tanks::tanksOn() const
{
  return tanks_on;
}

/// Estado da valvula 1: OPEN, CLOSED
uint16_t Tanks::v1isOpen() const
{
  if (!tanks_on) return 0; // Valvula fechada
  return uint16_t(v1_open);
}

/// Estado da valvula 2: OPEN, CLOSED
uint16_t Tanks::v2isOpen() const
{
  if (!tanks_on) return 0; // Valvula fechada
  return uint16_t(v2_open);
}

/// Funcao auxiliar (privada) para medicao do nivel de um dos tanques: 1 ou 2.
/// Valor real mais ruido, quantizado para 16 bits.
uint16_t Tanks::getH(int I) const
{
  if (!tanks_on) return 0;

  // Simula os tanques ateh o instante da leitura
  simulate();

  // Retorna a saida com ruido e quantizada
  double h_medida = (I==2 ? h2 : h1) +
                    MaxTankLevelMeasurement*percMeasureNoise*normal();
  if (h_medida<0.0) h_medida = 0.0;
  else if (h_medida>MaxTankLevelMeasurement) h_medida = MaxTankLevelMeasurement;
  return uint16_t(round(UINT16_MAX*(h_medida/MaxTankLevelMeasurement)));
}

/// Valor retornado pelo sensor do nivel do tanque 1
uint16_t Tanks::hTank1() const
{
  return getH(1);
}

/// Valor retornado pelo sensor do nivel do tanque 2
uint16_t Tanks::hTank2() const
{
  return getH(2);
}

/// Entrada da bomba: 0 a 65535
uint16_t Tanks::pumpInput() const
{
  if (!tanks_on) return 0;
  return pump_input;
}

/// Valor retornado pelo sensor de vazao da bomba: 0 a 65535
uint16_t Tanks::pumpFlow() const
{
  if (!tanks_on) return 0;

  // Simula os tanques ateh o instante da leitura
  simulate();

  // Retorna a saida com ruido e quantizada
  double flow_medido = flow_pump +
                       MaxPumpFlowMeasurement*percMeasureNoise*normal();
  if (flow_medido<0.0) flow_medido = 0.0;
  else if (flow_medido>MaxPumpFlowMeasurement) flow_medido = MaxPumpFlowMeasurement;
  return uint16_t(round(UINT16_MAX*(flow_medido/MaxPumpFlowMeasurement)));
}

/// Estado do transbordamento: true, false
uint16_t Tanks::isOverflowing() const
{
  if (!tanks_on) return 0; // Nao estah transbordando

  // Simula os tanques ateh o instante da leitura
  simulate();

  return uint16_t(is_overflowing);
}

/// Liga os tanques
void Tanks::setTanksOn()
{
  if (tanks_on) return;
  tanks_on = true;

  // Leh o instante atual como inicio da simulacao
  last_t = time(nullptr);

  // Lanca a thread de simulacao
  thr_simul = std::thread( [this]()
  {
    this->periodically_simulate();
  } );
  if (!thr_simul.joinable())
  {
    std::cerr << "Nao foi possivel lancar a thread de simulacao\n";
    return;
  }
}

/// Desliga os tanques
void Tanks::setTanksOff()
{
  if (!tanks_on) return;

  // Simula os tanques ateh o instante atual de desligamento
  simulate();

  tanks_on = false;            // Deve parar a thread
  v1_open = false;
  v2_open = false;
  pump_input = 0;

  // Espera pelo fim da thread
  if (thr_simul.joinable()) thr_simul.join();
  thr_simul = std::thread();
}

/// Fixa o estado da valvula 1: OPEN, CLOSED
void Tanks::setV1Open(bool Open)
{
  if (!tanks_on) return;

  // Simula os tanques ateh o instante atual com estado anterior da valvula
  simulate();
  // Fixa o novo estado da valvula para simular a partir de agora
  v1_open = Open;
}

/// Fixa o estado da valvula 2: OPEN, CLOSED
void Tanks::setV2Open(bool Open)
{
  if (!tanks_on) return;

  // Simula os tanques ateh o instante atual com estado anterior da valvula
  simulate();
  // Fixa o novo estado da valvula para simular a partir de agora
  v2_open = Open;
}

/// Fixa a entrada da bomba: 0 a 65535
void Tanks::setPumpInput(uint16_t Input)
{
  if (!tanks_on) return;

  // Simula os tanques ateh o instante atual com entrada anterior da bomba
  simulate();
  // Fixa a nova entrada da bomba para simular a partir de agora
  pump_input = Input;
}

inline double pow2(double x)
{
  return x*x;
}

/// Simula os tanques do instante da ultima simulacao ateh o instante atual
void Tanks::simulate() const
{
  // Constantes gerais
  const static double G=9.81;               // Aceleracao da gravidade (em m/s2)
  const static double Cd=0.61;              // Coeficiente de descarga padrao (adimensional)
  // Caracteristicas dos tanques
  const static double Tank1Area=Tank1Width*TankDepth;
  const static double Tank2Area=Tank2Width*TankDepth;
  // Caracteristicas das valvulas
  const static double Valv1Radius=0.005;    // Raio do orificio da valvula 1 (em m)
  const static double Valv1Area=M_PI*pow2(Valv1Radius);
  const static double Valv2Radius=0.0045;   // Raio do orificio da valvula 2 (em m)
  const static double Valv2Area=M_PI*pow2(Valv2Radius);
  // Caracteristicas do orificio entre tanques
  const static double Hole12Radius=0.006;   // Raio do orificio entre tanques (em m)
  const static double Hole12Area=M_PI*pow2(Hole12Radius);
  // Caracteristicas do orificio de transbordamento
  const static double OverflowRadius=0.008; // Raio do orificio de transbordamento (em m)
  const static double OverflowArea=M_PI*pow2(OverflowRadius);
  // Caracteristicas da bomba
  const static double FlowPumpMax=6.3E-5;   // Vazao maxima da bomba (em m3/s)
  // Simulacao
  const static double eps=1.0;              // Passo de simulacao (sempre 1 segundo)

  // Numero de passos de simulacao com transbordamento
  static int NStepsOverflow=0;
  // Variaveis para calculo do comportamento com histerese da bomba
  static double last_pump_input_perc=0.0;   // Entrada % anterior da bomba: 0 a 1.0
  static double last_flow_pump_perc=0.0;    // Vazao % anterior da bomba: 0 a 1.0

  // Mutex para simular como regiao critica
  static std::mutex mtx;

  // Soh simula se os tanques estiverem ligados
  if (!tanks_on) return;

  // As variaveis de simulacao que sao copias dos dados membros da classe.
  // A simulacao serah feita com essas copias.
  // Depois, os novos valores simulados serao copiados para os dados membros.
  // Esse artificio eh feito para poder alterar valores em uma funcao "const"
  bool is_over_internal = is_overflowing;
  double flow_pump_internal = flow_pump;
  double h1_internal = h1;
  double h2_internal = h2;
  time_t last_t_internal = last_t;

  // As variaveis exclusivamente da simulacao (nao sao dados da classe)
  // As vazoes
  double flow1,flow2;      // Vazao de escoamento pelas valvulas (se abertas)
  double flow12;           // Vazao entre tanques 1 e 2
  double flow_over;        // Vazao de transbordamento
  // A bomba
  double pump_input_perc;  // Entrada da bomba (em percentual: 0.0 a 1.0)
  double flow_pump_perc;   // Vazao da bomba (em percentual: 0.0 a 1.0)
  // As derivadas dos niveis
  double dh1, dh2;

  // Entra na regiao critica: bloqueia o semaforo
  mtx.lock();

  // Quando for iniciar a simulacao, mede o instante de tempo atual
  time_t current_t = time(nullptr);
  if (current_t <= last_t_internal)
  {
    // Libera o semaforo
    mtx.unlock();
    return;
  }

  while (last_t_internal < current_t)
  {
    // Calculo das vazoes

    // Escoamento do tanque 1 pela valvula 1
    if (v1_open)
    {
      flow1 = Cd*Valv1Area*sqrt(2.0*G*h1_internal);
      flow1 += fabs(flow1)*percDynamicNoise*normal();
      if (flow1<0.0) flow1 = 0.0;

    }
    else
    {
      flow1 = 0.0;
    }

    // Escoamento do tanque 2 pela valvula 2
    if (v2_open)
    {
      flow2 = Cd*Valv2Area*sqrt(2.0*G*h2_internal);
      flow2 += fabs(flow2)*percDynamicNoise*normal();
      if (flow2<0.0) flow2 = 0.0;
    }
    else
    {
      flow2 = 0.0;
    }

    // Fluxo entre os tanques pelo orificio 12
    if (h1_internal>Hole12Height)
    {
      if (h2_internal>Hole12Height)
      {
        if (h1_internal>=h2_internal)
        {
          flow12 = Cd*Hole12Area*sqrt(2.0*G*(h1_internal-h2_internal));
        }
        else
        {
          flow12 = -Cd*Hole12Area*sqrt(2.0*G*(h2_internal-h1_internal));
        }
      }
      else
      {
        flow12 = Cd*Hole12Area*sqrt(2.0*G*(h1_internal-Hole12Height));
      }
    }
    else
    {
      if (h2_internal>Hole12Height)
      {
        flow12 = -Cd*Hole12Area*sqrt(2.0*G*(h2_internal-Hole12Height));
      }
      else
      {
        flow12 = 0.0;
      }
    }
    flow12 += fabs(flow12)*percDynamicNoise*normal();

    // Escoamento por transbordamento
    if (h1_internal>OverflowHeight)
    {
      flow_over = Cd*OverflowArea*sqrt(2.0*G*(h1_internal-OverflowHeight));
      flow_over += fabs(flow_over)*percDynamicNoise*normal();
      if (flow_over<0.0) flow_over = 0.0;
    }
    else
    {
      flow_over = 0.0;
    }

    // Seta o booleano que indica transbordamento.
    // Se nao estava, passa a true apos 3 passos consecutivos de simulacao com transbordamento
    // Se estava, passa a false apos 3 passos consecutivos de simulacao sem transbordamento
    if (flow_over > 0.0)
    {
      if (is_over_internal)
      {
        NStepsOverflow = 3;
      }
      else
      {
        // Incrementa o numero de passos transbordando
        ++NStepsOverflow;
        // Muda estado se transbordou por 3 passos
        is_over_internal = (NStepsOverflow >= 3);
      }
    }
    else
    {
      if (!is_over_internal)
      {
        NStepsOverflow = 0;
      }
      else
      {
        // Decrementa o numero de passos transbordando
        --NStepsOverflow;
        // Muda estado se deixou de transbordar por 3 passos
        is_over_internal = !(NStepsOverflow <= 0);
      }
    }

    // Vazao de entrada da bomba para tanque 1
    if (pump_input > 0)
    {
      pump_input_perc = double(pump_input)/UINT16_MAX;
      // Histerese da bomba
      if (pump_input_perc == last_pump_input_perc)
      {
        flow_pump_perc = last_flow_pump_perc;
      }
      else if (pump_input_perc > last_pump_input_perc)
      {
        // Calcula vazao pela curva inferior do grafico de histerese
        if (pump_input_perc <= 0.05)
        {
          flow_pump_perc = 0.0;  // Zona morta
        }
        else
        {
          flow_pump_perc = (pump_input_perc-0.05)/0.95;
        }
        // Permanece com valor anterior se estiver entre as curvas
        if (flow_pump_perc < last_flow_pump_perc)
        {
          flow_pump_perc = last_flow_pump_perc;
        }
      }
      else  // pump_input_perc < last_pump_input_perc
      {
        // Calcula vazao pela curva superior do grafico de histerese
        if (pump_input_perc >= 0.95)
        {
          flow_pump_perc = 1.0;  // Zona morta
        }
        else
        {
          flow_pump_perc = pump_input_perc/0.95;
        }
        // Permanece com valor anterior se estiver entre as curvas
        if (flow_pump_perc > last_flow_pump_perc)
        {
          flow_pump_perc = last_flow_pump_perc;
        }
      }
      flow_pump_internal = FlowPumpMax*flow_pump_perc;
      flow_pump_internal += fabs(flow_pump_internal)*percDynamicNoise*normal();
      if (flow_pump_internal<0.0) flow_pump_internal = 0.0;
    }
    else
    {
      pump_input_perc = 0.0;
      flow_pump_perc = 0.0;
      flow_pump_internal = 0.0;
    }
    // Guarda os valores atuais como anteriores para o passo seguinte
    last_pump_input_perc = pump_input_perc;
    last_flow_pump_perc = flow_pump_perc;

    // Taxa de enchimento dos tanques: vazao de entrada menos vazao de saida
    // dividido pela area do tanque
    dh1 = (flow_pump_internal-flow1-flow12-flow_over)/Tank1Area;
    dh2 = (flow12-flow2)/Tank2Area;

    // Simulacao (integracao numerica)
    h1_internal += dh1*eps;
    h2_internal += dh2*eps;
    if (h1_internal<0.0)
    {
      h1_internal=0.0;
    }
    if (h2_internal<0.0)
    {
      h2_internal=0.0;
    }

    // Incrementa o instante da ultima simulacao
    ++last_t_internal;
  }  // FIM do while (last_t_internal < current_t)

  // Altera os dados membros da classe utilizando ponteiros, para
  // contornar a proibicao de alteracao, jah que a funcao eh "const"
  bool* pt_bool = (bool*)&is_overflowing;
  *pt_bool = is_over_internal;
  double* pt_double = (double*)&flow_pump;
  *pt_double = flow_pump_internal;
  pt_double = (double*)&h1;
  *pt_double = h1_internal;
  pt_double = (double*)&h2;
  *pt_double = h2_internal;
  time_t* pt_time = (time_t*)&last_t;
  *pt_time = last_t_internal;

  // Sai da regiao critica: libera o semaforo
  mtx.unlock();
}

/// Chama periodicamente a funcao "simulate" enquanto os tanques estiverem ligados
void Tanks::periodically_simulate() const
{
  while (tanks_on)
  {
    // Executa a simulacao
    simulate();
    // Espera 5 segundos
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
}


