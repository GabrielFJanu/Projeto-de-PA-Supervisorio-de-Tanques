#include <iostream>
#include "supcliente.h"

/// Construtor default
SupCliente::SupCliente()
  : meuUsuario("")
  , encerrarCliente(true)
  , is_admin(false)
  , last_S()
  , start_t(time_t(-1))
  , last_t(time_t(-1))
  , timeRefresh(20)
  /* ACRESCENTAR */
  , sock()
  , mtx()
  , thr()
{
  // Inicializa a biblioteca de sockets
  /* ACRESCENTAR */
  if (mysocket::init() != mysocket_status::SOCK_OK) // MODIFICADO
  {
    std::cerr <<  "Biblioteca mysocket nao pode ser inicializada";
    exit(-666);
  }
}

/// Destrutor
SupCliente::~SupCliente()
{
  // Nao pode chamar a funcao "desconectar" pois ela chama uma funcao virtual pura,
  // o que nao deve ocorrer no destrutor

  // Testa se estah conectado
  if (isConnected())
  {
    // Envia o comando de logout para o servidor
    /* ACRESCENTAR */
    sock.write_uint16(CMD_LOGOUT);
    // Espera 1 segundo para dar tempo ao servidor de ler a msg de LOGOUT
    // antes de fechar o socket
    /* ACRESCENTAR */
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Fecha o socket e, consequentemente, deve
    // encerrar a thread de leitura de dados do socket
    /* ACRESCENTAR */
    sock.close();
  }

  // Aguarda pelo fim da thread de recepcao
  join_if_joinable();

  // Encerra a biblioteca de sockets
  /* ACRESCENTAR */
  mysocket::end();
}

/// Conecta com o servidor.
/// Esta funcao soh pode ser chamada do programa principal,
/// jah que ela lanca a thread do cliente
void SupCliente::conectar(const std::string& IP,
                          const std::string& Login,
                          const std::string& Senha)
{
  // Comando recebido
  uint16_t cmd;
  // Resultado das operacoes com socket
  mysocket_status iResult;

  try
  {
    // Soh conecta se nao estiver conectado
    if (isConnected()) throw 101;

    // Conecta o socket
    // Em caso de erro, throw 102
    /* ACRESCENTAR */
    iResult = sock.connect(IP, SUP_PORT);
    if (iResult != mysocket_status::SOCK_OK) throw 102;

    // Envia o comando CMD_LOGIN.
    // Nao precisa bloquear o mutex para garantir exclusao mutua
    // pq nesse momento ainda nao foi lancada a thread.
    // Entao, essa funcao eh a unica enviando dados pelo socket.
    // Em caso de erro, throw 103
    /* ACRESCENTAR */
    iResult = sock.write_uint16(CMD_LOGIN);
    if (iResult != mysocket_status::SOCK_OK) throw 103;

    // Envia 1o parametro do comando (login)
    // Em caso de erro, throw 104
    /* ACRESCENTAR */
    iResult = sock.write_string(Login);
    if (iResult != mysocket_status::SOCK_OK) throw 104;

    // Envia 2o parametro do comando (senha)
    // Em caso de erro, throw 105
    /* ACRESCENTAR */
    iResult = sock.write_string(Senha);
    if (iResult != mysocket_status::SOCK_OK) throw 105;

    // Leh a resposta (cmd) do servidor ao pedido de conexao
    // Em caso de erro, throw 106
    /* ACRESCENTAR */
    iResult = sock.read_uint16(cmd);
    if (iResult != mysocket_status::SOCK_OK) throw 106;

    // Se a resposta nao for CMD_ADMIN_OK ou CMD_OK, throw 107
    if (cmd!=CMD_ADMIN_OK && cmd!=CMD_OK) throw 107;

    // Armazena o nome do usuario
    meuUsuario = Login;
    // Eh administrador (de acordo com resposta do servidor)?
    is_admin = (cmd==CMD_ADMIN_OK);
    // Cliente em funcionamento
    encerrarCliente = false;

    // Lanca a thread de solicitacao periodica de dados
    // Em caso de erro, throw 108
    /* ACRESCENTAR */
    thr = std::thread([this]()
    {
      this->main_thread(); //thread de solicitacao periodica de dados
    });
    if (!thr.joinable()) throw 108;
  }
  catch (int err)
  {
    // Encerra o cliente
    encerrarCliente = true;
    // Fecha o socket
    /* ACRESCENTAR */
    sock.close();

    // Msg de erro para debug
    std::string msg_err("Erro na conexao com o servidor ");
    msg_err += IP + " usuario " + Login + ": " + std::to_string(err);

    // Exibe msg de erro
    virtExibirErro(msg_err);
  }

  // Reexibe a interface
  virtExibirInterface();
}

/// Desconecta do servidor.
/// Esta funcao soh pode ser chamada do programa principal,
/// jah que ela espera (join) pelo fim da thread do cliente.
void SupCliente::desconectar()
{
  // Cliente encerrado
  encerrarCliente = true;

  // Testa se estah conectado
  if (isConnected())
  {
    // Envia o comando de logout para o servidor
    /* ACRESCENTAR */
    sock.write_uint16(CMD_LOGOUT);
    // Espera 1 segundo para dar tempo ao servidor de ler a msg de LOGOUT
    // antes de fechar o socket
    /* ACRESCENTAR */
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Fecha o socket e, consequentemente, deve
    // encerrar a thread de leitura de dados do socket
    /* ACRESCENTAR */
    sock.close();
  }

  // Aguarda fim da thread
  join_if_joinable();

  // Limpa o nome do usuario
  meuUsuario = "";
  // Limpa os dados armazenados de conexao anterior
  clearState();

  // Reexibe a interface
  virtExibirInterface();
}

/// Fixa o estado da valvula 1 (isV1==true) ou 2 (isV1==false)
/// como sendo aberta (Open==true) ou fechada (Open==false)
void SupCliente::setValvOpen(bool isV1, bool Open)
{
  // Comando enviado/recebido
  uint16_t cmd = (isV1 ? CMD_SET_V1 : CMD_SET_V2);

  // Resultado das operacoes com socket
  mysocket_status iResult;

  // Bloqueia o mutex para garantir exclusao mutua no envio pelo socket
  // de comandos que ficam aguardando resposta, para evitar que a resposta
  // de um comando seja recebida por outro comando em outra thread.
  /* ACRESCENTAR */
  mtx.lock();

  try
  {
    // Testa se estah conectado e eh administrador
    if (!isConnected() || !isAdmin()) throw 201;

    // Escreve o comando CMD_SET_V1 ou CMD_SET_V2
    // Em caso de erro, throw 202
    /* ACRESCENTAR */
    iResult = sock.write_uint16(cmd);
    if (iResult != mysocket_status::SOCK_OK) throw 202;

    // Escreve o parametro do comando (==0 se fechada !=0 se aberta)
    // Em caso de erro, throw 203
    /* ACRESCENTAR */
    cmd = static_cast<uint16_t>(Open); // cmd nesse caso equivale ao estado (uint16_t)
    iResult = sock.write_uint16(cmd);
    if (iResult != mysocket_status::SOCK_OK) throw 203;

    // Leh a resposta (cmd) do servidor ao comando
    // Em caso de erro, throw 204
    /* ACRESCENTAR */
    iResult = sock.read_uint16(cmd);
    if (iResult != mysocket_status::SOCK_OK) throw 204;

    // Se resposta nao for CMD_OK, throw 205
    if (cmd != CMD_OK) throw 205;
  }
  catch(int err)
  {
    // Libera o mutex para sair da zona de exclusao mutua.
    /* ACRESCENTAR */
    mtx.unlock();

    // Msg de erro para debug
    std::string msg_err = "Erro na atuacao sobre a valvula ";
    msg_err += (isV1 ? "1: " : "2: ");
    msg_err += std::to_string(err);
    virtExibirErro(msg_err);

    // Desconecta do servidor (reexibe a interface desconectada)
    desconectar();
  }

  // Libera o mutex para sair da zona de exclusao mutua.
  /* ACRESCENTAR */
  mtx.unlock();

  // Nao reexibe a interface com novo estado.
  // Serah reexibida quando chegar o proximo dado do servidor, que
  // deverah conter o novo estado da valvula.
}

/// Fixa a entrada da bomba: 0 a 65535
void SupCliente::setPumpInput(uint16_t Input)
{
  // Comando recebido
  uint16_t cmd;
  // Msg de erro para debug
  std::string msg_err;

  // Resultado das operacoes com socket
  mysocket_status iResult;

  // Bloqueia o mutex para garantir exclusao mutua no envio pelo socket
  // de comandos que ficam aguardando resposta, para evitar que a resposta
  // de um comando seja recebida por outro comando em outra thread.
  /* ACRESCENTAR */
  mtx.lock();

  try
  {
    // Testa se estah conectado e eh administrador
    if (!isConnected() || !isAdmin()) throw 301;

    // Escreve o comando CMD_SET_PUMP
    // Em caso de erro, throw 302
    /* ACRESCENTAR */
    iResult = sock.write_uint16(CMD_SET_PUMP);
    if (iResult != mysocket_status::SOCK_OK) throw 302;

    // Escreve o paramentro do comando CMD_SET_PUMP (Input = 0 a 65535)
    // Em caso de erro, throw 303
    /* ACRESCENTAR */
    iResult = sock.write_uint16(Input);
    if (iResult != mysocket_status::SOCK_OK) throw 303;

    // Leh a resposta do servidor ao comando
    // Em caso de erro, throw 304
    /* ACRESCENTAR */
    iResult = sock.read_uint16(cmd);
    if (iResult != mysocket_status::SOCK_OK) throw 304;

    // Se resposta nao for CMD_OK, throw 305
    if (cmd != CMD_OK) throw 305;
  }
  catch(int err)
  {
    // Libera o mutex para sair da zona de exclusao mutua.
    /* ACRESCENTAR */
    mtx.unlock();

    // Msg de erro para debug
    msg_err = "Erro na atuacao sobre a bomba: "+ std::to_string(err);
    virtExibirErro(msg_err);

    // Desconecta do servidor (reexibe a interface desconectada)
    desconectar();
  }

  // Libera o mutex para sair da zona de exclusao mutua.
  /* ACRESCENTAR */
  mtx.unlock();

  // Nao reexibe a interface com novo estado.
  // Serah reexibida quando chegar o proximo dado do servidor, que
  // deverah conter o novo estado da bomba.
}

/// Armazena o ultimo estado atual da planta.
/// Esta funcao pode ser complementada em uma interface especifica para
/// armazenar outros dados alem do ultimo estado da planta.
void SupCliente::storeState(const SupState& LastS)
{
  // Armazena o estado
  last_S = LastS;
  // Armazena o instante de recebimento de dados
  last_t = std::time(nullptr);
  // Inicializa o instante inicial, se for o primeiro ponto
  if (start_t == time_t(-1)) start_t = last_t;
}

/// Limpa todos os estados armazenados da planta
/// Esta funcao pode ser complementada em uma interface especifica para
/// limpar outros dados alem do ultimo estado da planta.
void SupCliente::clearState()
{
  last_S = SupState();
  // Limpa o instante em que iniciou a coleta de dados
  // e o instante de leitura do ultimo estado.
  start_t = last_t = time_t(-1);
}

/// Thread de solicitacao periodica de dados
void SupCliente::main_thread(void)
{
  // Comando recebido
  uint16_t cmd;
  // Estado recebido
  SupState S;
  // Resultado das operacoes com socket
  mysocket_status iResult;

  while (!encerrarCliente && isConnected())
  {

    // Bloqueia o mutex para garantir exclusao mutua no envio pelo socket
    // de comandos que ficam aguardando resposta, para evitar que a resposta
    // de um comando seja recebida por outro comando em outra thread.
    /* ACRESCENTAR */
    mtx.lock();

    try
    {
      // Escreve o comando CMD_GET_DATA
      // Em caso de erro, throw 401
      /* ACRESCENTAR */
      iResult = sock.write_uint16(CMD_GET_DATA);
      if (iResult != mysocket_status::SOCK_OK) throw 401;

      // Leh a resposta do servidor ao pedido de dados (com timeout)
      // Em caso de erro, throw 402
      /* ACRESCENTAR */
      iResult = sock.read_uint16(cmd, 1000*SUP_TIMEOUT);
      if (iResult != mysocket_status::SOCK_OK) throw 402;

      // Se resposta nao for CMD_OK, throw 403
      if (cmd != CMD_DATA) throw 403;
      // Leh os dados (com timeout)
      // Em caso de erro, throw 404
      /* ACRESCENTAR */
      iResult = sock.read_uint16(S.V1, 1000*SUP_TIMEOUT);
      if (iResult != mysocket_status::SOCK_OK) throw 404;
      iResult = sock.read_uint16(S.V2, 1000*SUP_TIMEOUT);
      if (iResult != mysocket_status::SOCK_OK) throw 404;
      iResult = sock.read_uint16(S.H1, 1000*SUP_TIMEOUT);
      if (iResult != mysocket_status::SOCK_OK) throw 404;
      iResult = sock.read_uint16(S.H2, 1000*SUP_TIMEOUT);
      if (iResult != mysocket_status::SOCK_OK) throw 404;
      iResult = sock.read_uint16(S.PumpInput, 1000*SUP_TIMEOUT);
      if (iResult != mysocket_status::SOCK_OK) throw 404;
      iResult = sock.read_uint16(S.PumpFlow, 1000*SUP_TIMEOUT);
      if (iResult != mysocket_status::SOCK_OK) throw 404;
      iResult = sock.read_uint16(S.ovfl, 1000*SUP_TIMEOUT);
      if (iResult != mysocket_status::SOCK_OK) throw 404;

      // Libera o mutex para sair da zona de exclusao mutua
      /* ACRESCENTAR */
      mtx.unlock();

      // Armazena os dados
      storeState(S);
      // Reexibe a interface
      virtExibirInterface();

      // Espera "timeRefresh" segundos
      /* ACRESCENTAR */
      std::this_thread::sleep_for(std::chrono::seconds(timeRefresh));
    }
    catch(int err)
    {
      // Libera o mutex para sair da zona de exclusao mutua
      /* ACRESCENTAR */
      mtx.unlock();

      // Nao pode chamar "desconectar" pq "desconectar" faz join na thread.
      // Como esta funcao main_thread eh executada na thread,
      // ela nao pode esperar pelo fim de si mesma.

      // Testa se estah conectado
      if (isConnected())
      {
        // Envia o comando de logout para o servidor
        /* ACRESCENTAR */
        sock.write_uint16(CMD_LOGOUT);
        // Espera 1 segundo para dar tempo ao servidor de ler a msg de LOGOUT
        // antes de fechar o socket
        /* ACRESCENTAR */
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // Fecha o socket
        /* ACRESCENTAR */
        sock.close();
      }

      // Testa se o usuario desconectou na interface.
      // Se nao, emite msg de erro.
      if (!encerrarCliente)
      {
        // Msg de erro para debug
        std::string msg_err = std::string("Erro na requisicao de dados: ") + std::to_string(err);

        // Exibe msg de erro
        virtExibirErro(msg_err);

        // Reexibe a interface
        virtExibirInterface();
      }
    } // Fim do catch

  } // Fim do while (!encerrarCliente && isConnected())
}

