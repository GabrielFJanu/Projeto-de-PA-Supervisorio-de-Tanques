#ifndef _MYSOCKET_H_
#define _MYSOCKET_H_

#include <cstdint>
#include <string>

/* #############################################################
   ##  ATENCAO: VOCE DEVE DESCOMENTAR UM DOS BLOCOS ABAIXO    ##
   ##  PARA PODER COMPILAR NO WINDOWS OU NO LINUX             ##
   ############################################################# */

/// Descomente o bloco a seguir para compilar no Windows

///*

// #############################################################
// ##  ATENCAO: PARA PODER COMPILAR NO WINDOWS VOCÊ DEVE:     ##
// ##  a) linkar com a biblioteca Ws2_32                      ##
// ##  b) ativar no compilador a opcao -std=c++11 ou c++14,   ##
// ##     caso jah nao seja a opcao por default               ##
// ############################################################# */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00  // Windows 10
#endif // _WIN32_WINNT

/// Os arquivos de inclusao para utilizacao dos sockets basicos
#include <winsock2.h>
#include <ws2tcpip.h>

/// O tipo que representa o socket basico do sistema operacional
/// O Windows jah define um tipo SOCKET;

/// O valor que representa um erro no socket basico
/// O Windows jah define a constante SOCKET_ERROR

/// O valor que representa um socket basico invalido
/// O Windows jah define a constante INVALID_SOCKET

//*/

/// Descomente o bloco a seguir para compilar no Linux

/*

// Os arquivos de inclusao para utilizacao dos sockets basicos
#include <sys/socket.h>

// O tipo que representa o socket basico do sistema operacional
typedef int SOCKET;

// O valor que representa um erro no socket basico
#define SOCKET_ERROR -1

// O valor que representa um socket basico invalido
#define INVALID_SOCKET -1

*/

/// Os status de retorno da classe mysocket
enum class mysocket_status
{
  // O valor de retorno em caso de sucesso
  SOCK_OK=0,
  // O valor de retorno em caso de erro
  SOCK_ERROR=-1,
  // Valor de retorno quando o outro socket encerrou a conexao corretamente
  SOCK_DISCONNECTED=-666,
  // Valor de retorno em caso de timeout
  SOCK_TIMEOUT=-999
};

/// Definicao de um byte (sinonimo de inteiro de 8 bits sem sinal)
using mybyte = uint8_t;

/// Predeclaracao das classes
class mysocket_queue;
class tcp_mysocket;
class tcp_mysocket_server;

/* #############################################################
   ##  A classe base dos sockets                              ##
   ############################################################# */

class mysocket
{
private:
  SOCKET id;

public:
  // A funcao de inicializacao da biblioteca de sockets
  static mysocket_status init();

  // A funcao de encerramento da biblioteca de sockets
  static void end();

  // Construtor default
  mysocket(): id(INVALID_SOCKET) {}
  // Construtor por movimento
  mysocket(mysocket&& S);
  // Operator de atribuicao por movimento
  void operator=(mysocket&& S);

  // Destrutor
  ~mysocket() {close();}

  // Fecha (caso esteja aberto) um socket
  void close();

  // Permuta dois sockets
  // Deve ser utilizado ao inves do operador de atribuicao
  void swap(mysocket& S);

  // Testa se um socket estah fechado
  bool closed() const;
  // Testa se um socket estah aberto (aceitando conexoes)
  bool accepting() const;
  // Testa se um socket estah conectado (pronto para ler e escrever)
  bool connected() const;

  // As classes amigas
  friend class tcp_mysocket;
  friend class tcp_mysocket_server;
  friend class mysocket_queue;

private:
  // Desabilita o construtor por copia
  mysocket(const mysocket& S) = delete;
  // Desabilita o operator de atribuicao por copia
  void operator=(const mysocket& S) = delete;

};

/* #############################################################
   ##  As classes dos sockets orientados a conexao (TCP)      ##
   ############################################################# */

/// Os sockets clientes
class tcp_mysocket: public mysocket
{
public:
  // Construtor default
  tcp_mysocket(): mysocket() {}
  // Construtor por movimento
  tcp_mysocket(tcp_mysocket&& S): mysocket(std::move(S)) {}
  // Operator de atribuicao por movimento
  void operator=(tcp_mysocket&& S) {mysocket::operator=(std::move(S));}

  // Se conecta a um socket aberto
  // Soh pode ser usado em sockets "virgens" ou explicitamente fechados
  // Retorna mysocket_status::SOCK_OK, se tudo deu certo, ou mysocket_status::SOCK_ERROR
  mysocket_status connect(const std::string& name, const std::string& port);

  // Leh uma sequencia de mybytes de um socket conectado
  // Soh pode ser usado em socket para o qual tenha sido feito um "connect" antes
  // Ou entao em um socket retornado pelo "accept" de um socket servidor
  // O ultimo parametro eh o tempo maximo (em milisegundos) para esperar
  // por dados; se for <0, que eh o default, espera indefinidamente.
  // Retorna:
  // - mysocket_status::SOCK_OK, em caso de sucesso;
  // - mysocket_status::SOCK_TIMEOUT, se retornou por timeout;
  // - mysocket_status::SOCK_DISCONNECTED, se a conexao foi fechada corretamente; ou
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status read_bytes(mybyte* buff, int len, long milisec=-1) const;

  // Escreve uma sequencia de mybytes em um socket conectado
  // Soh pode ser usado em socket para o qual tenha sido feito um "connect" antes
  // Ou entao em um socket retornado pelo "accept" de um socket servidor
  // Retorna:
  // - mysocket_status::SOCK_OK, em caso de sucesso;
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status write_bytes(const mybyte* buff, int len) const;

  // Leh um inteiro com sinal (int8_t, int16_t, int32_t, int64_t) ou sem sinal
  // (uint8_t, uint16_t, uint32_t, uint64_t) de um socket conectado.
  // O ultimo parametro eh o tempo maximo (em milisegundos) para esperar
  // por dados; se for <0, que eh o default, espera indefinidamente.
  // Retorna:
  // - mysocket_status::SOCK_OK, em caso de sucesso;
  // - mysocket_status::SOCK_TIMEOUT, se retornou por timeout;
  // - mysocket_status::SOCK_DISCONNECTED, se a conexao foi fechada corretamente; ou
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status read_int8(int8_t& num, long milisec=-1) const;
  mysocket_status read_int16(int16_t& num, long milisec=-1) const;
  mysocket_status read_int32(int32_t& num, long milisec=-1) const;
  mysocket_status read_int64(int64_t& num, long milisec=-1) const;
  mysocket_status read_uint8(uint8_t& num, long milisec=-1) const;
  mysocket_status read_uint16(uint16_t& num, long milisec=-1) const;
  mysocket_status read_uint32(uint32_t& num, long milisec=-1) const;
  mysocket_status read_uint64(uint64_t& num, long milisec=-1) const;

  // Escreve um inteiro com sinal (int8_t, int16_t, int32_t, int64_t) ou sem sinal
  // (uint8_t, uint16_t, uint32_t, uint64_t) em um socket conectado.
  // Retorna:
  // - mysocket_status::SOCK_OK, em caso de sucesso;
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status write_int8(const int8_t num) const;
  mysocket_status write_int16(const int16_t num) const;
  mysocket_status write_int32(const int32_t num) const;
  mysocket_status write_int64(const int64_t num) const;
  mysocket_status write_uint8(const uint8_t num) const;
  mysocket_status write_uint16(const uint16_t num) const;
  mysocket_status write_uint32(const uint32_t num) const;
  mysocket_status write_uint64(const uint64_t num) const;

  // Leh uma string de um socket conectado
  // Primeiro leh o numero de bytes da string, depois os caracteres
  // O ultimo parametro eh o tempo maximo (em milisegundos) para esperar
  // por dados; se for <0, que eh o default, espera indefinidamente.
  // Retorna:
  // - mysocket_status::SOCK_OK, em caso de sucesso;
  // - mysocket_status::SOCK_TIMEOUT, se retornou por timeout;
  // - mysocket_status::SOCK_DISCONNECTED, se a conexao foi fechada corretamente; ou
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status read_string(std::string& msg, long milisec=-1) const;

  // Escreve uma string em um socket conectado
  // Primeiro escreve o numero de bytes da string, depois os caracteres
  // Retorna:
  // - mysocket_status::SOCK_OK, em caso de sucesso;
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status write_string(const std::string& msg) const;

private:
  // Desabilita o construtor por copia
  tcp_mysocket(const tcp_mysocket& S) = delete;
  // Desabilita a criacao do operator de atribuicao
  void operator=(const tcp_mysocket& S) = delete;
};

/// Os sockets servidores
class tcp_mysocket_server: public mysocket
{
public:
  // Construtor default
  tcp_mysocket_server(): mysocket() {}
  // Construtor por movimento
  tcp_mysocket_server(tcp_mysocket_server&& S): mysocket(std::move(S)) {}
  // Operator de atribuicao por movimento
  void operator=(tcp_mysocket_server&& S) {mysocket::operator=(std::move(S));}

  // Abre um novo socket para esperar conexoes
  // Soh pode ser usado em sockets "virgens" ou explicitamente fechados
  // Retorna mysocket_status::SOCK_OK ou mysocket_status::SOCK_ERROR
  mysocket_status listen(const std::string& port, int nconex=1);

  // Aceita uma conexao que chegou em um socket aberto
  // Soh pode ser usado em socket para o qual tenha sido feito um "listen" antes
  // O socket "a" passado como parametro, em caso de sucesso, estarah conectado
  // (nao-conectado em caso de erro)
  // Retorna mysocket_status::SOCK_OK ou mysocket_status::SOCK_ERROR
  mysocket_status accept(tcp_mysocket& a) const;

private:
  // Desabilita o construtor por copia
  tcp_mysocket_server(const tcp_mysocket_server& S) = delete;
  // Desabilita a criacao do operator de atribuicao
  void operator=(const tcp_mysocket_server& S) = delete;

};

/* #############################################################
   ##  A fila de sockets                                      ##
   ############################################################# */

class mysocket_queue
{
 private:
  // Conjunto de sockets
  fd_set set;

  // O valor da maior_id+1 de todos os sockets inseridos na fila de sockets
  // O select do Unix utiliza esse valor e soh monitora os sockets ateh essa id
  // O select do Windows ignora esse valor
  SOCKET nfds;

  // Desabilita a criacao do construtor por copia
  mysocket_queue(const mysocket_queue& S) = delete;
  // Desabilita a criacao do construtor por movimento
  mysocket_queue(mysocket_queue&& S) = delete;

  // Desabilita a criacao do operator de atribuicao por copia
  void operator=(const mysocket_queue& S) = delete;
  // Desabilita a criacao do operator de atribuicao por movimento
  void operator=(mysocket_queue&& S) = delete;

 public:
  // Limpa a lista de sockets
  void clear();

  // Construtor e destrutor
  mysocket_queue();
  ~mysocket_queue();

  // Adiciona um socket a uma fila de sockets
  // Retorna mysocket_status::SOCK_OK ou mysocket_status::SOCK_ERROR
  mysocket_status include(const mysocket& a);

  // Retira um socket de uma fila de sockets
  // Retorna mysocket_status::SOCK_OK ou mysocket_status::SOCK_ERROR
  mysocket_status exclude(const mysocket& a);

  // Bloqueia ateh haver alguma atividade de leitura em socket da fila
  // Retorna:
  // - mysocket_status::SOCK_OK, caso haja dados a serem lidos (sucesso);
  // - mysocket_status::SOCK_TIMEOUT, se retornou por timeout; ou
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status wait_read(long milisec=-1);

  // Bloqueia ateh haver alguma atividade de conexao em socket da fila
  // Retorna:
  // - mysocket_status::SOCK_OK, caso haja dados a serem lidos (sucesso);
  // - mysocket_status::SOCK_TIMEOUT, se retornou por timeout; ou
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status wait_connect(long milisec=-1);

  // Bloqueia ateh haver alguma atividade de escrita em socket da fila
  // Retorna:
  // - mysocket_status::SOCK_OK, caso haja dados a serem lidos (sucesso);
  // - mysocket_status::SOCK_TIMEOUT, se retornou por timeout; ou
  // - mysocket_status::SOCK_ERROR, em caso de erro
  mysocket_status wait_write(long milisec=-1);

  // Testa se houve atividade em um socket especifico da fila
  bool had_activity(const mysocket& a);

};

#endif
