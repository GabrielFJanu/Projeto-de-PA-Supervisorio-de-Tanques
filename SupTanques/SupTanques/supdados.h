#ifndef _SUP_DADOS_H_
#define _SUP_DADOS_H_

#include <cstdint>

/// Porta de comunicacao cliente-servidor.
#define SUP_PORT "23456"

/// Timeout (em segundos) para esperar o envio pelo socket
/// de um parametro ou resposta de um comando enviado anteriormente
#define SUP_TIMEOUT 10

/// Os comandos do SupTanques.
enum SupCommands: uint16_t
{
  CMD_LOGIN=1001,
  CMD_ADMIN_OK=1002,
  CMD_OK=1003,
  CMD_ERROR=1004,
  CMD_GET_DATA=1005,
  CMD_DATA=1006,
  CMD_SET_V1=1007,
  CMD_SET_V2=1008,
  CMD_SET_PUMP=1009,
  CMD_LOGOUT=1010
};

/// O estado atual da planta.
struct SupState
{
  // Estados das valvulas: aberta (diferente de 0) ou fechada (0)
  uint16_t V1=0,V2=0;
  // Niveis dos tanques: 0 a 65535
  uint16_t H1=0,H2=0;
  // Entrada da bomba: 0 a 65535
  uint16_t PumpInput=0;
  // Vazao da bomba: 0 a 65535
  uint16_t PumpFlow=0;
  // Estah transbordando: sim (diferente de 0) ou nao (0)
  uint16_t ovfl=0;

  // Impressao em console do estado da planta
  void print() const;
};

#endif // _SUP_DADOS_H_
