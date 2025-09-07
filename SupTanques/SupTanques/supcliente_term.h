#ifndef _SUP_CLIENT_TERM_H_
#define _SUP_CLIENT_TERM_H_

#include "supcliente.h"

/* *******************************
   * CLASS SUPCLIENTE_TERM     *
   ******************************* */

class SupClienteTerm: public SupCliente
{
public:
  // Construtor default
  SupClienteTerm(): SupCliente() {}
  // Destrutor
  ~SupClienteTerm() {}

  // Laco principal da interface, executado na funcao main
  void main(void);

private:
  // Construtores e operadores de atribuicao suprimidos (nao existem na classe)
  SupClienteTerm(const SupClienteTerm& other) = delete;
  SupClienteTerm(SupClienteTerm&& other) = delete;
  SupClienteTerm& operator=(const SupClienteTerm& other) = delete;
  SupClienteTerm& operator=(SupClienteTerm&& other) = delete;

  // As funcoes virtuais puras de exibicao de dados que sao chamadas pela thread.
  // Para cada cliente, serao implementadas de acordo com a interface em uso.
  // Exibe informacao de erro
  void virtExibirErro(const std::string& msg) const override;
  // Redesenha toda a interface (chegada de dados, desconexao, etc)
  void virtExibirInterface() const override;

  // As funcoes virtuais de armazenamento de dados.
  // Nao precisam ser complementadas na interface console.
};

#endif // _SUP_CLIENT_TERM_H_
