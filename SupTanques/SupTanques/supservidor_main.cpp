#include <iostream>
#include <cmath>      /* round */
#include "supservidor.h"

using namespace std;

int main()
{
  // O servidor do sistema de tanques
  SupServidor ST_Server;

  // Relogio interno: primeira leitura, delta_t desde entao
  time_t first_t,delta_t;

  // Usuario a ser adicionado/removido
  string Login, Senha;

  // Variaveis auxiliares para digitacao de dados
  uint16_t BInput;     // Entrada da bomba: 0 a 65535
  double BInput_perc;  // Entrada % da bomba: 0 a 100.0
  string texto;
  int opcao;
  char C;

  first_t = time(nullptr);
  do
  {
    do
    {
      cout << "\n=================\n";
      if (!ST_Server.serverOn())
      {
        cout << " 0 - Ligar o servidor\n";
        cout << "=================\n";
      }
      if (ST_Server.serverOn())
      {
        cout << " 1 - Ler e imprimir o estado atual da planta\n";
        cout << "=================\n";
        cout << "11 - Alterar a entrada da bomba\n";
        cout << "12 - Abrir a valvula do tanque 1\n";
        cout << "13 - Abrir a valvula do tanque 2\n";
        cout << "14 - Fechar a valvula do tanque 1\n";
        cout << "15 - Fechar a valvula do tanque 2\n";
        cout << "=================\n";
        cout << "21 - Listar usuarios\n";
        cout << "22 - Adicionar usuario\n";
        cout << "23 - Remover usuario\n";
        cout << "=================\n";
        cout << "98 - Desligar o servidor\n";
      }
      cout << "99 - Sair\n";
      cout << "=================\n";
      cout << "Opcao: ";
      cin >> texto;
      try
      {
        opcao = stoi(texto);
      }
      catch(...)
      {
        opcao = -1;
      }
    }
    while (opcao<0 || opcao>99);

    if (!ST_Server.serverOn()) // Servidor nao estah ligado
    {
      if (opcao == 0)
      {
        if (!ST_Server.setServerOn()) cerr << "Erro ao iniciar o servidor!\n";
        else first_t = time(nullptr);
      }
      else
      {
        if (opcao != 99) cout << "Servidor estah desligado!\n";
      }
    }
    else               // Servidor estah ligado
    {
      // Executa a opcao escolhida
      switch(opcao)
      {
      case 0:
        cout << "Servidor jah estah ligado!\n";
        break;
      case 1:
        // Calcula e imprime o tempo decorrido desde que o servidor foi ligado
        delta_t = time(nullptr)-first_t;
        cout << "T=";
        if (delta_t >= 3600)
        {
          int horas = delta_t/3600;
          delta_t -= 3600*horas;
          cout << horas << 'h';
        }
        if (delta_t >= 60)
        {
          int minutos = delta_t/60;
          delta_t -= 60*minutos;
          cout << minutos << 'm';
        }
        cout << delta_t << "s ";
        // Leh e imprime o estado da planta
        ST_Server.readPrintState();
        break;
      case 11:
        do
        {
          cout << "Entrada % da bomba [0.0 a 100.0]: ";
          cin >> texto;
          try
          {
            BInput_perc = stof(texto);
          }
          catch(...)
          {
            BInput_perc = -1.0;
          }
        }
        while (BInput_perc<0.0 || BInput_perc>100.0);
        BInput = round(UINT16_MAX*BInput_perc/100.0);
        ST_Server.setPumpInput(BInput);
        break;
      case 12:
        ST_Server.setV1Open(true);
        break;
      case 13:
        ST_Server.setV2Open(true);
        break;
      case 14:
        ST_Server.setV1Open(false);
        break;
      case 15:
        ST_Server.setV2Open(false);
        break;
      case 21:
        cout << "USUARIOS CADASTRADOS:\n";
        ST_Server.printUsers();
        break;
      case 22:
        do
        {
          cout << "Login do novo usuario [6-12 caracteres]: ";
          cin >> Login;
        }
        while (Login.size()<6 || Login.size()>12);
        do
        {
          cout << "Senha do novo usuario [6-12 caracteres]: ";
          cin >> Senha;
        }
        while (Senha.size()<6 || Senha.size()>12);
        do
        {
          cout << "Eh administrador [S/N]? ";
          cin >> C;
          C = toupper(C);
        }
        while (C!='S' && C!='N');
        if (ST_Server.addUser(Login, Senha, (C=='S')))
        {
          cout << "Usuario " << Login << " inserido\n";
        }
        else
        {
          cout << "Usuario " << Login << " invalido (nao inserido)\n";
        }
        break;
      case 23:
        do
        {
          cout << "Login do usuario a ser removido: ";
          cin >> Login;
        }
        while (Login.size()<6 || Login.size()>12);
        if (ST_Server.removeUser(Login)) cout << "Usuario " << Login << " removido\n";
        else cout << "Usuario " << Login << " inexistente (nao removido)\n";
        break;
      case 98:
      case 99:
        first_t = time(nullptr);
        ST_Server.setServerOff();
        break;
      default:
        // Opcao inexistente: nao faz nada
        break;
      }
    }
  }
  while (opcao!=99);

  return 0;
}
