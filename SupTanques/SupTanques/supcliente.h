#ifndef _SUP_CLIENTE_H_
#define _SUP_CLIENTE_H_

#include <string>
#include <ctime>
#include "supdados.h"
/* ACRESCENTAR */
#include <thread>
#include <mutex>
#include "../MySocket/mysocket.h"

class SupCliente
{
// Funcoes protegidas
protected:
  // Construtor default
  SupCliente();
  // Destrutor
  virtual ~SupCliente();

  // Funcoes de consulta
  // Cliente conectado (true) ou desconectado (false)
  bool isConnected() const {return sock.connected();} //MODIFICADO
  // Cliente administrador (true) ou visualizador (false)
  bool isAdmin() const {return is_admin;}
  // Ultimo estado da planta
  const SupState& lastState() const {return last_S;}
  // Instante de leitura do ultimo dado apos inicio das leituras
  int deltaT() const {return last_t-start_t;}

  // Conectar com o servidor
  void conectar(const std::string& IP,
                const std::string& Login,
                const std::string& Senha);

  // Desconectar do servidor
  void desconectar();

  // Espera pelo fim da thread de solicitacao de dados
  void join_if_joinable() {if (thr.joinable()) thr.join();} //MODIFICADO

  // As funcoes de comunicacao com o servidor
  // Fixa o estado da valvula 1: aberta (true) ou fechada (false)
  void setV1Open(bool Open) {setValvOpen(true,Open);}
  // Fixa o estado da valvula 2: aberta (true) ou fechada (false)
  void setV2Open(bool Open) {setValvOpen(false,Open);}
  // Fixa a entrada da bomba: 0 a 65535
  void setPumpInput(uint16_t Input);

  // As funcoes de gerenciamento da interface.
  // Altera o periodo de solicitacao de novos dados
  void setTimeRefresh(int T) {if (T>=10 && T<=200) timeRefresh=T;}
  // As funcoes virtuais de gerenciamento dos dados armazenados na interface,
  // que serao complementadas nas classes derivadas de acordo com a interface em uso.
  // Armazena o ultimo estado da planta
  virtual void storeState(const SupState& LastS);
  // Limpa todos os estados da planta armazenados
  virtual void clearState();

// Dados protegidos
protected:
  // O nome do usuario do cliente
  std::string meuUsuario;

  // Indica se a interface encerrou o cliente
  bool encerrarCliente;

// Funcoes privadas
private:
  // Construtores e operadores de atribuicao suprimidos (nao existem na classe)
  SupCliente(const SupCliente& other) = delete;
  SupCliente(SupCliente&& other) = delete;
  SupCliente& operator=(const SupCliente& other) = delete;
  SupCliente& operator=(SupCliente&& other) = delete;

  // As funcoes virtuais puras de exibicao de dados que sao chamadas pela thread.
  // Para cada cliente, serao implementadas de acordo com a interface em uso.

  // Exibe informacao de erro
  virtual void virtExibirErro(const std::string& msg) const = 0;
  // Redesenha toda a interface (chegada de dados, desconexao, etc)
  virtual void virtExibirInterface() const = 0;

  // Funcao auxiliar para evitar repeticao de codigo.
  // Fixa o estado da valvula 1 (isV1==true) ou 2 (isV1==false)
  // como sendo aberta (Open==true) ou fechada (Open==false)
  void setValvOpen(bool isV1, bool Open);

  // Thread de solicitacao periodica de dados
  void main_thread(void);

// Dados privados
private:
  // Cliente eh administrador
  bool is_admin;

  // Ultimo estado lido da planta
  SupState last_S;
  // Instante de tempo da primeira leitura de estado da planta
  std::time_t start_t;
  // Instante de tempo da ultima leitura de estado da planta
  std::time_t last_t;
  // Periodo de solicitacao de novos dados
  int timeRefresh;

  // Socket de comunicacaco
  /* ACRESCENTAR */
  tcp_mysocket sock;

  // Exclusao mutua para nao enviar novo comando antes de
  // receber a resposta do comando anterior
  /* ACRESCENTAR */
  std::mutex mtx;

  // Identificador da thread de solicitacao periodica de dados
  /* ACRESCENTAR */
  std:: thread thr;
};

#endif // _SUP_CLIENTE_H_
