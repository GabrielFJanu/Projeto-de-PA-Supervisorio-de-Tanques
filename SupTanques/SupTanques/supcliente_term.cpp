#include <iostream>
#include <cmath>      /* round */
#include "supcliente_term.h"

using namespace std;

void SupClienteTerm::main()
{
  // Parametros da conexao com servidor
  string IP,Login,Senha;
  // Variaveis auxiliares para entrada de dados
  string ST;
  uint16_t input; // Entrada da bomba: 0 a 65535
  double perc;    // Entrada % da bomba: 0 a 100.0
  int opcao;
  bool opcaoValida;

  // Imprimir numeros em ponto flutuante sempre com 1 casa decimal
  cout.precision(1);
  cout << fixed;

  do
  {
    // Pode ser que estivesse conectado antes e nao estah mais pq a thread
    // fechou o socket. Entao, eh preciso fazer join na thread.
    if (!isConnected()) join_if_joinable();

    // Escolha da opcao
    cout << "\n=================\n";
    if (!isConnected()) // Cliente nao estah conectado
    {
      cout << "NAO CONECTADO\n";
      cout << " 1 - Conectar cliente ao servidor\n";
      cout << "=================\n";
    }
    else // Cliente estah conectado
    {
      cout << "USUARIO: " << meuUsuario << endl;
      cout << "11 - Alterar o periodo de amostragem dos dados\n";
      if (isAdmin())
      {
        cout << "=================\n";
        cout << "21 - Alterar a entrada da bomba\n";
        cout << "22 - Abrir a valvula do tanque 1\n";
        cout << "23 - Abrir a valvula do tanque 2\n";
        cout << "24 - Fechar a valvula do tanque 1\n";
        cout << "25 - Fechar a valvula do tanque 2\n";
      }
      cout << "=================\n";
      cout << "98 - Desconectar cliente do servidor\n";
    }
    cout << "99 - Sair\n";
    cout << "=================\n";
    cout << "Opcao: ";

    do
    {
      // Assume inicialmente que a opcao eh valida
      opcaoValida = true;

      // Leh a opcao desejada pelo usuario
      getline(cin,ST);
      try
      {
        opcao = stoi(ST);
      }
      catch(...)
      {
        // Texto invalido (nao conversivel para int)
        opcao = -1;
      }

      // Testa se a opcao eh valida de acordo com o contexto
      if (opcao==99) continue;
      if (!isConnected())
      {
        if (opcao==1) continue;
      }
      else // Estah conectado
      {
        // Opcoes validas quando estah conectado
        if (opcao==11 || opcao==98) continue;
        // Opcoes validas quando estah conectado como administrador
        if (isAdmin() && opcao>=21 && opcao<=25) continue;
      }
      // Opcao invalida
      opcaoValida = false;
      cout << "Opcao invalida. Nova opcao: ";
    }
    while (!opcaoValida); // Repete entrada de dados se opcao invalida

    // Executa a opcao escolhida
    switch(opcao)
    {
    case 1:
      do
      {
        cout << "IP: ";
        getline(cin,IP);
      }
      while (IP.size() < 7);
      do
      {
        cout << "Login [6-12 caracteres]: ";
        getline(cin,Login);
      }
      while (Login.size()<6 || Login.size()>12);
      do
      {
        cout << "Senha [6-12 caracteres]: ";
        getline(cin,Senha);
      }
      while (Senha.size()<6 || Senha.size()>12);
      // Jah reexibe interface em qualquer caso e exibe msg em caso de erro
      conectar(IP, Login, Senha);
      break;
    case 11:
      do
      {
        cout << "Periodo de amostragem (em s) [10 a 200]: ";
        getline(cin,ST);
        try
        {
          input = stoi(ST);
        }
        catch(...)
        {
          input = 0;
        }
      }
      while (input<10 || input>200);
      setTimeRefresh(input);
      break;
    case 21:
      do
      {
        cout << "Entrada % da bomba [0.0 a 100.0]: ";
        getline(cin,ST);
        try
        {
          perc = stof(ST);
        }
        catch(...)
        {
          perc = -1.0;
        }
      }
      while (perc<0.0 || perc>100.0);
      input = round(UINT16_MAX*perc/100.0);
      // Jah exibe msg em caso de erro
      setPumpInput(input);
      break;
    case 22:
      // Jah exibe msg em caso de erro
      setV1Open(true);
      break;
    case 23:
      // Jah exibe msg em caso de erro
      setV2Open(true);
      break;
    case 24:
      // Jah exibe msg em caso de erro
      setV1Open(false);
      break;
    case 25:
      // Jah exibe msg em caso de erro
      setV2Open(false);
      break;
    case 98:
    case 99:
      desconectar();
      break;
    default:
      // Opcao inexistente: nao faz nada
      // Nunca deve entrar aqui...
      cerr << "Opcao inexistente!\n";
      break;
    }  // fim switch
  }  // fim do
  while (opcao!=99);
}

/// As funcoes virtuais de exibicao de dados que sao chamadas pela thread.
/// Imprime mensagem de texto no console.

/// Exibe informacao de erro
void SupClienteTerm::virtExibirErro(const std::string& msg) const
{
  cerr << "\n=================\n";
  cerr << msg;
  cerr << "\n=================\n";
}

/// Reexibe a interface
void SupClienteTerm::virtExibirInterface() const
{
  if (!isConnected())
  {
    cout << "\nNAO CONECTADO";
  }
  else
  {
    cout << "\nt=" << deltaT() << " segundos\n";
    lastState().print();
  }
  cout << endl;
}
