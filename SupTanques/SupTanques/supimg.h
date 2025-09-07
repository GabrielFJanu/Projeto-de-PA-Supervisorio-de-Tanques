#ifndef SUP_IMG_H
#define SUP_IMG_H

#include <QLabel>
#include "supdados.h"
#include <deque>

/// Numero maximo de pontos a serem exibidos no grafico.
#define NumMaxGraphPoints 181 // Se 1pt cada 20s, entao 0 a 180 = 60min = 1h

/// A imagem do supervisorio
/// Vai exibir o desenho do nivel atual dos tanques ou o grafico dos niveis
class SupImg: public QLabel
{
public:
  // Construtor default
  SupImg(QWidget *parent = nullptr);

  // Escolhe exibir o nivel atual dos tanques (true) ou o grafico (false)
  void setDisplayMode(bool show_level);

  // Limpa a imagem
  void clear();

  // Adiciona um ponto ao historico de dados recebidos
  void addPoint(int T, const SupState& S);

  // Desenha a imagem
  void drawImg();

private:
  // Exibe o nivel atual dos tanques (true) ou o grafico (false)
  bool displayLevel;

  // Estado das valvulas (a ser desenhado na imagem se estiver em modo "level")
  bool V1Open, V2Open;

  // Estado do transbordamento (a ser desenhado na imagem se estiver em modo "level")
  bool overflow;

  // O periodo de amostragem (refresh) dos dados
  int deltaT;

  // Um ponto a ser exibido na imagem, se estiver em modo "graph"
  struct Point
  {
    int t;  // Em segundos
    double h1,h2;
  };

  // Os pontos a serem exibidos na imagem, se estiver em modo "graph"
  std::deque<Point> points;

  // A imagem do icone de advertencia do transbordamento
  QPixmap alert;

  // A imagem a ser exibida
  QPixmap img;

  // Quando o objeto for redimensionado
  void resizeEvent(QResizeEvent *event) override;
};

#endif // SUP_IMG_H
