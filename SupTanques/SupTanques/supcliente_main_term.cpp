#include "supcliente_term.h"

/// ==============================
/// Funcao principal (dialogo com o usuario)
/// ==============================

int main(int argc, char *argv[])
{
  // O objeto que implementa o cliente SupTanques
  SupClienteTerm ST_Client;

  // Lanca o laco (menu) da interface
  ST_Client.main();

  return 0;
}
