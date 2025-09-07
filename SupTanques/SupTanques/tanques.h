#ifndef _TANKS_H_
#define _TANKS_H_

#include <ctime>        /* time_t */
#include <thread>       /* std::thread */
#include <mutex>        /* std::mutex */
#include "tanques-param.h"

/// Classe que representa o sistema com 2 tanques
class Tanks
{
public:
  // Construtor
  Tanks();

  // Destrutor
  ~Tanks();

  // Funcoes de consulta
  bool tanksOn() const;              // Tanques ligados (true) ou desligados (false)
  uint16_t v1isOpen() const;         // Estado da valvula 1: aberta (!=0) ou fechada (==0)
  uint16_t v2isOpen() const;         // Estado da valvula 2: aberta (!=0) ou fechada (==0)
  uint16_t hTank1() const;           // Medida do sensor de nivel tanque 1: 0 a 65535
  uint16_t hTank2() const;           // Medida do sensor de nivel tanque 2: 0 a 65535
  uint16_t pumpInput() const;        // Entrada da bomba: 0 a 65535
  uint16_t pumpFlow() const;         // Medida do sensor de vazao da bomba: 0 a 65535
  uint16_t isOverflowing() const;    // Estah transbordando: sim (!=0) ou nao (==0)

  // Funcoes de atuacao
  void setTanksOn();                 // Liga os tanques
  void setTanksOff();                // Desliga os tanques
  void setV1Open(bool Open);         // Fixa o estado da valvula 1: aberta (true) ou fechada (false)
  void setV2Open(bool Open);         // Fixa o estado da valvula 2: aberta (true) ou fechada (false)
  void setPumpInput(uint16_t Input); // Fixa a entrada da bomba: 0 a 65535

private:
  // Construtores e operadores de atribuicao suprimidos (nao existem na classe)
  Tanks(const Tanks& other) = delete;
  Tanks(Tanks&& other) = delete;
  Tanks& operator=(const Tanks& other) = delete;
  Tanks& operator=(Tanks&& other) = delete;

  // Estado dos tanques como um todo (ligado/desligado)
  bool tanks_on;
  // Nivel dos tanques 1 e 2
  double h1,h2;                      // Niveis dos tanques (em metros)
  // Valvulas 1 e 2
  bool v1_open,v2_open;              // Estados das valvulas: aberta (true) ou fechada (false)
  // Bomba
  uint16_t pump_input;               // Entrada da bomba (% da vazao maxima): 0 a 65535
  double flow_pump;                  // Vazao da bomba para tanque 1 (em m3/s)
  // Transbordamento
  bool is_overflowing;

  // Funcao privada de consulta
  uint16_t getH(int I) const;        // Medida do sensor I (1 ou 2) de nivel: 0 a 65535

  // Instante da ultima simulacao
  time_t last_t;
  // Identificador da thread de simulação
  std::thread thr_simul;

  // Funcoes privadas de simulacao
  void simulate() const;             // Simula os tanques ateh o instante atual
  void periodically_simulate() const;// Chama periodicamente a funcao "simulate"
};

#endif // _TANKS_H_
