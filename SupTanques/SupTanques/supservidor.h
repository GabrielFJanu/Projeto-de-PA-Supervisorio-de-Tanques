#ifndef _SUP_SERVIDOR_H_
#define _SUP_SERVIDOR_H_

/* ACRESCENTAR */
#include <string>
#include <list>
#include "tanques.h"
#include "supdados.h"
#include <thread>
#include "../MySocket/mysocket.h"

/// A classe que implementa o servidor do sistema de tanques
class SupServidor: public Tanks
{
private:
  // Subclasse privada para representar os usuarios cadastrados no servidor
  struct User
  {
    // Identificacao do usuario
    std::string login;    // Nome de login
    std::string password; // Senha
    bool isAdmin;         // Pode alterar (true) ou soh consultar (false) o sistema
    // Socket de comunicacao
    /*ACRESCENTAR*/
    tcp_mysocket sock;
    // Construtor default
    User(const std::string& Login, const std::string& Senha, bool Admin)
      :login(Login)
      ,password(Senha)
      ,isAdmin(Admin)
      /*ACRESCENTAR*/
      ,sock()
    {}
    // Comparacao com string (testa se a string eh igual ao login)
    bool operator==(const std::string& S) const {return login==S;}
    // Usuario estah conectado ou nao?
    inline bool isConnected() const {return sock.connected();} //MODIFICADO
    // Desconecta usuario
    inline void close() {sock.close();} //MODIFICADO
  };

public:
  // Construtor default
  SupServidor();
  // Destrutor
  ~SupServidor();

  // Funcoes de consulta
  // Servidor ligado (true) ou desligado (false)
  bool serverOn() const {return server_on;}

  // Funcoes de atuacao
  bool setServerOn();                // Liga o servidor: retorna true se OK
  void setServerOff();               // Desliga o servidor

  // Leitura e impressao em console do estado da planta
  void readPrintState() const;
  // Impressao em console dos usuarios do servidor
  void printUsers() const;

  // Adicionar um novo usuario
  bool addUser(const std::string& Login, const std::string& Senha, bool Admin);
  // Remover um usuario
  bool removeUser(const std::string& Login);

private:
  // Construtores e operadores de atribuicao suprimidos (nao existem na classe)
  SupServidor(const SupServidor& other) = delete;
  SupServidor(SupServidor&& other) = delete;
  SupServidor& operator=(const SupServidor& other) = delete;
  SupServidor& operator=(SupServidor&& other) = delete;

  // Estado do servidor como um todo (ligado/desligado)
  bool server_on;

  // Lista de usuarios do servidor
  std::list<User> LU;
  // Identificador da thread do servidor
  /*ACRESCENTAR*/
  std::thread thr_server;
  // Socket de conexoes
  /*ACRESCENTAR*/
  tcp_mysocket_server sock_server;

  // Leitura do estado dos tanques a partir dos sensores
  void readStateFromSensors(SupState& S) const;

  // A funcao que implementa a thread do servidor
  // Leitura e envio de dados pelos sockets
  void thr_server_main(void);
};

#endif // _SUP_SERVIDOR_H_
