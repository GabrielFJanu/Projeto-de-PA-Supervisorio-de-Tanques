#include <iostream>     /* cerr */
#include <algorithm>
#include "supservidor.h"

using namespace std;

/* ========================================
   CLASSE SUPSERVIDOR
   ======================================== */

/// Construtor
SupServidor::SupServidor()
  : Tanks()
  , server_on(false)
  , LU()
  /*ACRESCENTAR*/
  , thr_server()
  , sock_server()
{
  // Inicializa a biblioteca de sockets
  /*ACRESCENTAR*/
  mysocket_status iResult = mysocket::init();
  // Em caso de erro, mensagem e encerra
  if (iResult != mysocket_status::SOCK_OK) //MODIFICADO
  {
    cerr <<  "Biblioteca mysocket nao pode ser inicializada";
    exit(-1);
  }
}

/// Destrutor
SupServidor::~SupServidor()
{
  // Deve parar a thread do servidor
  server_on = false;

  // Fecha todos os sockets dos clientes
  for (auto& U : LU) U.close();
  // Fecha o socket de conexoes
  /*ACRESCENTAR*/
  sock_server.close();

  // Espera o fim da thread do servidor
  /*ACRESCENTAR*/
  if (thr_server.joinable()) thr_server.join();

  // Encerra a biblioteca de sockets
  /*ACRESCENTAR*/
  mysocket::end();
}

/// Liga o servidor
bool SupServidor::setServerOn()
{
  // Se jah estah ligado, nao faz nada
  if (server_on) return true;

  // Liga os tanques
  setTanksOn();

  // Indica que o servidor estah ligado a partir de agora
  server_on = true;

  try
  {
    // Coloca o socket de conexoes em escuta
    /*ACRESCENTAR*/
    mysocket_status iResult = sock_server.listen(SUP_PORT);
    // Em caso de erro, gera excecao
    if (iResult != mysocket_status::SOCK_OK) throw 1; //MODIFICADO

    // Lanca a thread do servidor que comunica com os clientes
    /*ACRESCENTAR*/
    thr_server = thread( [this]()
    {
      this->thr_server_main();
    } );
    // Em caso de erro, gera excecao
    if (!thr_server.joinable()) throw 2; //MODIFICADO
  }
  catch(int i)
  {
    cerr << "Erro " << i << " ao iniciar o servidor\n";

    // Deve parar a thread do servidor
    server_on = false;

    // Fecha o socket do servidor
    /*ACRESCENTAR*/
    sock_server.close();

    return false;
  }

  // Tudo OK
  return true;
}

/// Desliga o servidor
void SupServidor::setServerOff()
{
  // Se jah estah desligado, nao faz nada
  if (!server_on) return;

  // Deve parar a thread do servidor
  server_on = false;

  // Fecha todos os sockets dos clientes
  for (auto& U : LU) U.close();
  // Fecha o socket de conexoes
  /*ACRESCENTAR*/
  sock_server.close();

  // Espera pelo fim da thread do servidor
  /*ACRESCENTAR*/
  if (thr_server.joinable()) thr_server.join();
  // Faz o identificador da thread apontar para thread vazia
  /*ACRESCENTAR*/
  thr_server = thread();

  // Desliga os tanques
  setTanksOff();
}

/// Leitura do estado dos tanques
void SupServidor::readStateFromSensors(SupState& S) const
{
  // Estados das valvulas: OPEN, CLOSED
  S.V1 = v1isOpen();
  S.V2 = v2isOpen();
  // Niveis dos tanques: 0 a 65535
  S.H1 = hTank1();
  S.H2 = hTank2();
  // Entrada da bomba: 0 a 65535
  S.PumpInput = pumpInput();
  // Vazao da bomba: 0 a 65535
  S.PumpFlow = pumpFlow();
  // Estah transbordando (true) ou nao (false)
  S.ovfl = isOverflowing();
}

/// Leitura e impressao em console do estado da planta
void SupServidor::readPrintState() const
{
  if (tanksOn())
  {
    SupState S;
    readStateFromSensors(S);
    S.print();
  }
  else
  {
    cout << "Tanques estao desligados!\n";
  }
}

/// Impressao em console dos usuarios do servidor
void SupServidor::printUsers() const
{
  for (const auto& U : LU)
  {
    cout << U.login << '\t'
         << "Admin=" << (U.isAdmin ? "SIM" : "NAO") << '\t'
         << "Conect=" << (U.isConnected() ? "SIM" : "NAO") << '\n';
  }
}

/// Adicionar um novo usuario
bool SupServidor::addUser(const string& Login, const string& Senha,
                             bool Admin)
{
  // Testa os dados do novo usuario
  if (Login.size()<6 || Login.size()>12) return false;
  if (Senha.size()<6 || Senha.size()>12) return false;

  // Testa se jah existe usuario com mesmo login
  auto itr = find(LU.begin(), LU.end(), Login);
  if (itr != LU.end()) return false;

  // Insere
  LU.push_back( User(Login,Senha,Admin) );

  // Insercao OK
  return true;
}

/// Remover um usuario
bool SupServidor::removeUser(const string& Login)
{
  // Testa se existe usuario com esse login
  auto itr = find(LU.begin(), LU.end(), Login);
  if (itr == LU.end()) return false;

  // Remove
  LU.erase(itr);

  // Remocao OK
  return true;
}

/// A thread que implementa o servidor.
/// Comunicacao com os clientes atraves dos sockets.
void SupServidor::thr_server_main(void)
{
  // Fila de sockets para aguardar chegada de dados
  /*ACRESCENTAR*/
  mysocket_queue f;
  // Socket temporario
  tcp_mysocket t;
  // O comando recebido/enviado
  uint16_t cmd;
  // Dados da nova conexao
  string login, password;
  // Variaveis auxiliares
  // O status de retorno das funcoes do socket
  mysocket_status iResult;
  // Eventuais parametros de comandos lidos do socket
  uint16_t param;
  // SupState enviado
  SupState S;
  // Iteradores para percurso nos conteineres
  list<User>::iterator iU;

  while (server_on)
  {
    // Erros mais graves que encerram o servidor
    // Parametro do throw e do catch eh uma const char* = "texto"
    try
    {
      // Encerra se o socket de conexoes estiver fechado
      if (!sock_server.accepting()) //MODIFICADO
      {
        throw "socket de conexoes fechado";
      }

      // Inclui na fila de sockets todos os sockets que eu
      // quero monitorar para ver se houve chegada de dados

      // Limpa a fila de sockets
      /*ACRESCENTAR*/
      f.clear();
      // Inclui na fila o socket de conexoes
      /*ACRESCENTAR*/
      f.include(sock_server);
      // Inclui na fila todos os sockets dos clientes conectados
      /*ACRESCENTAR*/
      for (auto& U : LU)
      {
        if (U.isConnected()) f.include(U.sock);
      }

      // Espera ateh que chegue dado em algum socket (com timeout)
      /*ACRESCENTAR*/
      iResult = f.wait_read(SUP_TIMEOUT*1000);

      switch (iResult) // resultado do wait_read
      {
      case mysocket_status::SOCK_ERROR: // resultado do wait_read
      default:
        // Erro no select
        throw "fila de espera"; // Erro grave: encerra o servidor
        break;
      case mysocket_status::SOCK_TIMEOUT: // resultado do wait_read
        // Nao faz nada
        break;
      case mysocket_status::SOCK_OK: // resultado do wait_read
        // Houve atividade em algum socket da fila
        // Testa em qual socket houve atividade.
        try // Erros nos clientes: catch fecha a conexao com esse cliente
        {
          // Primeiro, testa os sockets dos clientes
          for (iU=LU.begin(); server_on && iU!=LU.end(); ++iU)
          {
            if (server_on && iU->isConnected() && f.had_activity(iU->sock))
            {
              // Leh o comando recebido do cliente
              iResult = iU->sock.read_uint16(cmd);
              if (iResult != mysocket_status::SOCK_OK) throw 1;

              // Executa o comando lido
              switch(cmd)
              {
              case CMD_LOGIN:
              case CMD_ADMIN_OK:
              case CMD_OK:
              case CMD_ERROR:
              case CMD_DATA:
              default:
                // Comando invalido
                throw 2;
                break;
              case CMD_GET_DATA: // GET DATA
                //leh o SupState S
                readStateFromSensors(S);
                //responde para o cliente CMD_DATA
                iResult = iU->sock.write_uint16(CMD_DATA);
                if (iResult != mysocket_status::SOCK_OK) throw 3;
                //escreve o parametro do CMD_DATA (SupState)
                iResult = iU->sock.write_uint16(S.V1);
                if (iResult != mysocket_status::SOCK_OK) throw 4;
                iResult = iU->sock.write_uint16(S.V2);
                if (iResult != mysocket_status::SOCK_OK) throw 4;
                iResult = iU->sock.write_uint16(S.H1);
                if (iResult != mysocket_status::SOCK_OK) throw 4;
                iResult = iU->sock.write_uint16(S.H2);
                if (iResult != mysocket_status::SOCK_OK) throw 4;
                iResult = iU->sock.write_uint16(S.PumpInput);
                if (iResult != mysocket_status::SOCK_OK) throw 4;
                iResult = iU->sock.write_uint16(S.PumpFlow);
                if (iResult != mysocket_status::SOCK_OK) throw 4;
                iResult = iU->sock.write_uint16(S.ovfl);
                if (iResult != mysocket_status::SOCK_OK) throw 4;

                break; // Fim do case CMD_GET_DATA
              case CMD_SET_V1: // SET V1
                //verifica se eh admin
                if(!(iU->isAdmin)) throw 5;
                //leh o paramtetro de CMD_SET_V1, param (uint16_t)
                iResult = iU->sock.read_uint16(param, SUP_TIMEOUT*1000);
                if (iResult != mysocket_status::SOCK_OK) throw 6;
                //set v1
                setV1Open(static_cast<bool>(param));
                //o servidor imprime uma mensagem de debug
                cout << "CMD_SET_V1 " << param << " de " << iU->login << " (OK)" << endl;
                //responde para o cliente CMD_OK
                iResult = iU->sock.write_uint16(CMD_OK);
                if (iResult != mysocket_status::SOCK_OK) throw 7;

                break; // Fim do case CMD_SET_V1
              case CMD_SET_V2: // SET V2
                //verifica se eh admin
                if(!(iU->isAdmin)) throw 8;
                //leh o paramtetro de CMD_SET_V2, param (uint16_t)
                iResult = iU->sock.read_uint16(param, SUP_TIMEOUT*1000);
                if (iResult != mysocket_status::SOCK_OK) throw 9;
                //set v2
                setV2Open(static_cast<bool>(param));
                //o servidor imprime uma mensagem de debug
                cout << "CMD_SET_V2 " << param << " de " << iU->login << " (OK)" << endl;
                //responde para o cliente CMD_OK
                iResult = iU->sock.write_uint16(CMD_OK);
                if (iResult != mysocket_status::SOCK_OK) throw 10;

                break; // Fim do case CMD_SET_V2
              case CMD_SET_PUMP: // SET PUMP
                //verifica se eh admin
                if(!(iU->isAdmin)) throw 11;
                //leh o paramtetro de CMD_SET_PUMP, param (uint16_t)
                iResult = iU->sock.read_uint16(param, SUP_TIMEOUT*1000);
                if (iResult != mysocket_status::SOCK_OK) throw 12;
                //set pump
                setPumpInput(param);
                //o servidor imprime uma mensagem de debug
                cout << "CMD_SET_PUMP " << param << " de " << iU->login << " (OK)" << endl;
                //responde para o cliente CMD_OK
                iResult = iU->sock.write_uint16(CMD_OK);
                if (iResult != mysocket_status::SOCK_OK) throw 13;

                break; // Fim do case CMD_SET_PUMP
              case CMD_LOGOUT: // LOGOUT
                //Fecha o socket
                iU->close();
                //o servidor imprime uma mensagem de debug
                cout << "CMD_LOGOUT " << iU->login << endl;

                break; // Fim do case CMD_LOGOUT
              }// Fim do switch(cmd)
            }// Fim do if (... && had_activity) no socket do cliente
          }// Fim do for para todos os clientes
        }// Fim do try para erros nos clientes
        catch (int erro) // Erros na leitura do socket de algum cliente
        {
          switch(erro)
          {
          case 5: //nao eh admin
            //responde CMD_ERROR para o cliente
            iU->sock.write_uint16(CMD_ERROR);
            //o servidor imprime uma mensagem de debug
            cout << "CMD_SET_V1 " << iU->login << " (ERROR)" << endl;
            break;
          case 8: //nao eh admin
            //responde CMD_ERROR para o cliente
            iU->sock.write_uint16(CMD_ERROR);
            //o servidor imprime uma mensagem de debug
            cout << "CMD_SET_V2 " << iU->login << " (ERROR)" << endl;
            break;
          case 11: // nao eh admin
            //responde CMD_ERROR para o cliente
            iU->sock.write_uint16(CMD_ERROR);
            //o servidor imprime uma mensagem de debug
            cout << "CMD_SET_PUMP " << iU->login << " (ERROR)" << endl;
            break;
          case 1: //erro na leitura do comando
          case 2: //comando invalido
          case 3: //erro ao escrever
          case 4: //erro ao escrever
          case 6: //erro na leitura
          case 7: //erro ao escrever
          case 9: //erro na leitura
          case 10: //erro ao escrever
          case 12: //erro na leitura
          case 13: //erro ao escrever
          default:
            //Fecha o socket
            iU->close();
            // Informa o erro na comunicacao ou comando invalido
            cerr << "Erro " << erro << " na comunicacao ou comando invalido do cliente "<< iU->login << endl;
            break;
          } // Fim do switch(erro)
        } // Fim do catch (int erro)
        // Depois de testar os sockets dos clientes,
        // testa se houve atividade no socket de conexao
        if (server_on && sock_server.connected() && f.had_activity(sock_server))
        {
          // Aceita provisoriamente a nova conexao
          iResult = sock_server.accept(t);
          if (iResult != mysocket_status::SOCK_OK) throw "aceitar provisoriamente a conexao"; // Erro grave: encerra o servidor

          try // Erros na conexao de cliente: fecha socket temporario
          {
            // Leh o comando
            iResult = t.read_uint16(cmd, SUP_TIMEOUT*1000);
            if (iResult != mysocket_status::SOCK_OK) throw 1;

            // Testa o comando
            if (cmd!=CMD_LOGIN) throw 2;

            // Leh o login do usuario que deseja se conectar
            iResult = t.read_string(login, SUP_TIMEOUT*1000);
            if (iResult != mysocket_status::SOCK_OK) throw 3;

            // Leh a senha do usuario que deseja se conectar
            iResult = t.read_string(password, SUP_TIMEOUT*1000);
            if (iResult != mysocket_status::SOCK_OK) throw 4;

            // Testa os dados do novo usuario
            if (login.size()<6 || login.size()>12) throw 5;
            if (password.size()<6 || password.size()>12) throw 5;

            // Verifica se jah existe um usuario cadastrado com esse login
            iU = find(LU.begin(), LU.end(), login);
            if(iU==LU.end()) throw 6; // Erro se nao existir

            // Testa se a senha confere
            if (iU->password != password) throw 7; // Senha nao confere

            // Testa se o cliente jah estah conectado
            if (iU->isConnected()) throw 8; // User jah conectado

            // Associa o socket que se conectou a um usuario cadastrado
            iU->sock.swap(t);

            // Envia a confirmacao de conexao para o novo cliente
            iResult = (iU->isAdmin ? iU->sock.write_uint16(CMD_ADMIN_OK) : iU->sock.write_uint16(CMD_OK));
            if (iResult != mysocket_status::SOCK_OK) throw 9;

            //o servidor imprime uma mensagem de debug
            cout << "CMD_LOGIN " << iU->login << " (OK)" << endl;
          } // Fim do try para erros na conexao de cliente
          catch (int erro) // Erros na conexao do cliente
          {
            if (erro>=5 && erro<=8)
            {
              // Comunicacao com socket temporario OK, login invalido
              // Envia comando informando login invalido
              t.write_uint16(CMD_ERROR);
              // Espera 1 segundo para dar tempo ao cliente de ler a msg de CMD_ERROR
              // antes de fechar o socket
              this_thread::sleep_for(chrono::seconds(1));
              // Erro na comunicacao com socket temporario
              t.close();
              //o servidor imprime uma mensagem de debug
              cout << "CMD_LOGIN " << login << " (ERROR)" << endl;
            }
            else // erro 1-4 ou 9
            {
              if(erro==9) // erro 9
              {
                // Erro na comunicacao com socket do cliente
                iU->close();
              }
              else // erro 1-4
              {
                // Erro na comunicacao com socket temporario
                t.close();
              }
              // Informa erro nao previsto
              cerr << "Erro " << erro << " na conexao do cliente" << endl;
            }
          } // fim catch
        } // fim if (had_activity) no socket de conexoes
        break; // fim do case mysocket_status::SOCK_OK - resultado do wait_read
      } // fim do switch (iResult) - resultado do wait_read

      // De acordo com o resultado da espera:
      // SOCK_TIMEOUT:
      // Saiu por timeout: nao houve atividade em nenhum socket
      // Aproveita para salvar dados ou entao nao faz nada
      // SOCK_ERROR:
      // Erro no select: encerra o servidor
      // SOCK_OK:
      // Houve atividade em algum socket da fila:
      //   Testa se houve atividade nos sockets dos clientes. Se sim:
      //   - Leh o comando
      //   - Executa a acao
      //   = Envia resposta
      //   Depois, testa se houve atividade no socket de conexao. Se sim:
      //   - Estabelece nova conexao em socket temporario
      //   - Leh comando, login e senha
      //   - Testa usuario
      //   - Se deu tudo certo, faz o socket temporario ser o novo socket
      //     do cliente e envia confirmacao

    } // fim try - Erros mais graves que encerram o servidor
    catch(const char* err)  // Erros mais graves que encerram o servidor
    {
      cerr << "Erro no servidor: " << err << endl;

      // Sai do while e encerra a thread
      server_on = false;

      // Fecha todos os sockets dos clientes
      for (auto& U : LU) U.close();
      // Fecha o socket de conexoes
      /*ACRESCENTAR*/
      sock_server.close();

      // Os tanques continuam funcionando

    } // fim catch - Erros mais graves que encerram o servidor
  } // fim while (server_on)
}



