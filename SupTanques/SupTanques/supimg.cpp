#include <QPen>
#include <QPainter>
#include <QFont>
#include <QPolygonF>
#include <QResizeEvent>
#include <cmath>          // round
#include "tanques-param.h"
#include "supimg.h"

SupImg::SupImg(QWidget *parent)
  : QLabel(parent)
  , displayLevel(true)
  , V1Open(false)
  , V2Open(false)
  , overflow(false)
  , deltaT(10)
  , points()
  , alert()
  , img()
{
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setGeometry(0,0,600,600);
  setMinimumSize(600,600);
  setBaseSize(600,600);
  setFrameShape(QFrame::Box);
  setAlignment(Qt::AlignCenter);
  setScaledContents(false);
  setStyleSheet("background-color: white");

  alert = QPixmap(61,53);
  alert.fill(QColor(0,255,255,0));  // Fundo ciano transparente

  // Os elementos para desenho
  QPen pen;
  QPainter painter;
  QPolygon poly;

  painter.begin(&alert);
  painter.setRenderHint(QPainter::Antialiasing);

  // Linhas arredondadas
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);

  // O triangulo
  pen.setColor(Qt::black);
  pen.setWidth(4);
  painter.setPen(pen);
  painter.setBrush(Qt::yellow);
  poly.setPoints(3, 2,50 , 58,50 , 30,2);
  painter.drawPolygon(poly);

  // A barra do ponto de exclamacao
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  pen.setWidth(3);
  painter.setPen(pen);
  painter.setBrush(Qt::black);
  poly.setPoints(4, 30,35 , 33,20 , 30,17, 27,20);
  painter.drawConvexPolygon(poly);
  // O circulo do ponto de exclamacao
  painter.drawEllipse(QPoint(30,42), 3,3);

  painter.end();
}

/// Escolhe desenho do nivel atual dos tanques (true) ou o grafico (false)
void SupImg::setDisplayMode(bool show_level)
{
  if (displayLevel != show_level)
  {
    displayLevel = show_level;

    // Redesenha a imagem
    drawImg();
  }
}

void SupImg::clear()
{
  points.clear();
  drawImg();
}

/// Adiciona um ponto ao historico de dados recebidos
void SupImg::addPoint(int T, const SupState& S)
{
  V1Open = (S.V1 != 0);
  V2Open = (S.V2 != 0);
  overflow = (S.ovfl != 0);

  Point P;
  P.t = T;
  P.h1 = MaxTankLevelMeasurement*S.H1/UINT16_MAX;
  P.h2 = MaxTankLevelMeasurement*S.H2/UINT16_MAX;
  if (!points.empty())
  {
    int last_deltaT = T - points.back().t;
    if (points.size()>1) deltaT = (2*deltaT + last_deltaT)/3;
    else deltaT = last_deltaT;
  }
  points.push_back(P);
  if (points.size() > NumMaxGraphPoints) points.pop_front();

  // Redesenha a imagem
  drawImg();
}

void SupImg::drawImg()
{
  if (points.empty())
  {
    img = QPixmap(width(),height());
    img.fill(Qt::white);
    setPixmap(img);
    return;
  }

  // Os limites no eixo horizontal (X) e vertical (Y), em unidades fisicas
  double minX, maxX;
  double /*minY=0.0,*/ maxY;

  // As dimensoes da imagem (pixels)
  int imgWidthPx, imgHeightPx;
  // A margem da imagem (pixels)
  const int imgMarginPx = 50;
  // O tamanho da fonte (pixels)
  const int fontSizePx = imgMarginPx/2;

  if (displayLevel)
  {
    //
    // DIMENSIONA IMAGEM PARA OS TANQUES
    //

    // Limites do desenho
    minX = 0.0;
    maxX = Tank1Width + Tank2Width;
    maxY = TankHeight;
    // Dimensoes da imagem
    if ((width()-2*imgMarginPx)/maxX >= (height()-2*imgMarginPx)/maxY)
    {
      // A janela eh mais larga que alta
      imgHeightPx = height();
      imgWidthPx = std::round((imgHeightPx-2*imgMarginPx)*(maxX/maxY) + 2*imgMarginPx);
    }
    else
    {
      // A janela eh mais alta que larga
      imgWidthPx = width();
      imgHeightPx = std::round((imgWidthPx-2*imgMarginPx)*(maxY/maxX) + 2*imgMarginPx);
    }
  }
  else
  {
    //
    // DIMENSIONA IMAGEM PARA O GRAFICO
    //

    // Limites do desenho
    // Nao precisa testar se points.empty(), pois essa condicao jah eh testada antes e encerra a funcao
    minX = points.front().t;
    if (points.size() < NumMaxGraphPoints)
    {
      maxX = points.back().t + (NumMaxGraphPoints-points.size())*deltaT;
    }
    else
    {
      maxX = points.back().t;
    }
    maxY = MaxTankLevelMeasurement;
    // Dimensoes da imagem
    imgWidthPx = width();
    imgHeightPx = height();
  }

  // A funcao que converte de X para coordenada horizontal
  auto convX = [&](double X) -> double
  {
    return imgMarginPx + ((X-minX)/(maxX-minX))*(imgWidthPx-1 - 2*imgMarginPx);
  };
  // A funcao que converte de deltaX para delta coordenada horizontal
  auto convDeltaX = [&](double DX) -> double
  {
    return (DX/(maxX-minX))*(imgWidthPx-1 - 2*imgMarginPx);
  };
  // A funcao que converte de Y para coordenada vertical
  auto convY = [&](double Y) -> double
  {
    return imgHeightPx-1-imgMarginPx - (Y/maxY)*(imgHeightPx-1 - 2*imgMarginPx);
  };
  // A funcao que converte de deltaY para delta coordenada vertical
  auto convDeltaY = [&](double DY) -> double
  {
    return (DY/maxY)*(imgHeightPx-1 - 2*imgMarginPx);
  };

  // Cria e preenche a imagem
  img = QPixmap(imgWidthPx,imgHeightPx);
  img.fill(Qt::white);

  // Os elementos para desenho
  QPen pen;
  QPainter painter;

  // Inicia o desenho na imagem
  painter.begin(&img);

  if (displayLevel)
  {
    //
    // DESENHA OS TANQUES
    //

    // Os fluidos (conteudo dos tanques)
    // Nao precisa testar se points.empty(), pois essa condicao jah eh testada antes e encerra a funcao
    Point P = points.back();
    pen.setWidth(0);
    pen.setColor(Qt::cyan);
    painter.setPen(pen);
    painter.fillRect(convX(0.0),convY(P.h1),
                     convDeltaX(Tank1Width),convDeltaY(P.h1),Qt::cyan);
    painter.fillRect(convX(Tank1Width),convY(P.h2),
                     convDeltaX(Tank2Width),convDeltaY(P.h2),Qt::cyan);

    // As paredes dos tanques
    pen.setWidth(3);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    // Tanque 1
    painter.drawLine(convX(0.0),convY(0.0), convX(0.0),convY(TankHeight));
    painter.drawLine(convX(0.0),convY(0.0), convX(Tank1Width),convY(0.0));
    painter.drawLine(convX(Tank1Width),convY(0.0), convX(Tank1Width),convY(TankHeight));
    // Tanque 2
    painter.drawLine(convX(Tank1Width),convY(0.0), convX(Tank1Width+Tank2Width),convY(0.0));
    painter.drawLine(convX(Tank1Width+Tank2Width),convY(0.0), convX(Tank1Width+Tank2Width),convY(TankHeight));

    // As aberturas: orificios e valvulas
    pen.setWidth(3);
    // O triangulo para desenhar os escoamentos pelos orificios
    QPolygonF triangle(3);
    // A dimensao das aberturas (pixels)
    const int holePx = 20;
    // O orificio entre os tanques
    if ( (convY(P.h1)<=convY(Hole12Height)-holePx &&
          convY(P.h2)<=convY(Hole12Height)-holePx) )
    {
      // Os dois niveis estao acima da borda superior do orificio
      pen.setColor(Qt::cyan);
      painter.setPen(pen);
    }
    else if ( (P.h1<=Hole12Height &&
               P.h2<=Hole12Height) )
    {
      // Os dois niveis estao abaixo da borda inferior do orificio
      pen.setColor(Qt::white);
      painter.setPen(pen);
    }
    else
    {
      // Um nivel acima e outro abaixo do orificio
      pen.setColor(Qt::cyan);
      painter.setPen(pen);
      painter.setBrush(Qt::cyan);

      double ySup,yInf;
      if (P.h1>P.h2)
      {
        // Escoamento de T1 para T2
        ySup = std::max(convY(P.h1), convY(Hole12Height)-holePx);
        yInf = std::min(convY(P.h2), convY(Hole12Height));
        triangle[0] = QPointF(convX(Tank1Width)+holePx,yInf);
      }
      else
      {
        // Escoamento de T2 para T1
        ySup = std::max(convY(P.h2), convY(Hole12Height)-holePx);
        yInf = std::min(convY(P.h1), convY(Hole12Height));
        triangle[0] = QPointF(convX(Tank1Width)-holePx, yInf);
      }
      triangle[1] = QPointF(convX(Tank1Width), yInf);
      triangle[2] = QPointF(convX(Tank1Width), ySup);
      painter.drawPolygon(triangle);
    }
    painter.drawLine(convX(Tank1Width), convY(Hole12Height),
                     convX(Tank1Width), convY(Hole12Height)-holePx);
    // O orificio de transbordamento
    if (P.h1>OverflowHeight)
    {
      pen.setColor(Qt::cyan);
      painter.setPen(pen);
      painter.setBrush(Qt::cyan);
      triangle[0] = QPointF(convX(0.0)-holePx, convY(OverflowHeight));
      triangle[1] = QPointF(convX(0.0), convY(OverflowHeight));
      triangle[2] = QPointF(convX(0.0), std::max(convY(P.h1),convY(OverflowHeight)-holePx));
      painter.drawPolygon(triangle);
    }
    else
    {
      pen.setColor(Qt::white);
      painter.setPen(pen);
    }
    painter.drawLine(convX(0.0), convY(OverflowHeight),
                     convX(0.0), convY(OverflowHeight)-holePx);
    if (overflow)
    {
      // Desenha o icone de advertencia de transbordamento
      painter.drawPixmap(convX(0.0)+imgMarginPx/2, convY(OverflowHeight), alert);
    }
    // A valvula 1
    if (V1Open)
    {
      if (P.h1>0.0)
      {
        pen.setColor(Qt::cyan);
        painter.setPen(pen);
        painter.setBrush(Qt::cyan);
        triangle[0] = QPointF(convX(Tank1Width/2.0)-holePx/2.0, convY(0.0));
        triangle[1] = QPointF(convX(Tank1Width/2.0)+holePx/2.0, convY(0.0));
        triangle[2] = QPointF(convX(Tank1Width/2.0), convY(0.0)+holePx);
        painter.drawPolygon(triangle);
      }
      else
      {
        pen.setColor(Qt::white);
        painter.setPen(pen);
      }
      painter.drawLine(convX(Tank1Width/2.0)-holePx/2.0, convY(0.0),
                       convX(Tank1Width/2.0)+holePx/2.0, convY(0.0));
    }
    // A valvula 2
    if (V2Open)
    {
      if (P.h2>0.0)
      {
        pen.setColor(Qt::cyan);
        painter.setPen(pen);
        painter.setBrush(Qt::cyan);
        triangle[0] = QPointF(convX(Tank1Width+Tank2Width/2.0)-holePx/2.0, convY(0.0));
        triangle[1] = QPointF(convX(Tank1Width+Tank2Width/2.0)+holePx/2.0, convY(0.0));
        triangle[2] = QPointF(convX(Tank1Width+Tank2Width/2.0), convY(0.0)+holePx);
        painter.drawPolygon(triangle);
      }
      else
      {
        pen.setColor(Qt::white);
        painter.setPen(pen);
      }
      painter.drawLine(convX(Tank1Width+Tank2Width/2.0)-holePx/2.0, convY(0.0),
                       convX(Tank1Width+Tank2Width/2.0)+holePx/2.0, convY(0.0));
    }

    // Os titulos dos tanques
    QFont font = painter.font();
    font.setPixelSize(fontSizePx);
    painter.setFont(font);
    pen.setWidth(1);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    // Tanque 1
    painter.drawText(QRectF(convX(0.0),convY(TankHeight)-imgMarginPx,
                            convDeltaX(Tank1Width), imgMarginPx),
                     Qt::AlignCenter,
                     "Tank 1");
    // Tanque 2
    painter.drawText(QRectF(convX(Tank1Width),convY(TankHeight)-imgMarginPx,
                            convDeltaX(Tank2Width), imgMarginPx),
                     Qt::AlignCenter,
                     "Tank 2");
  }
  else
  {
    //
    // DESENHA O GRAFICO
    //

    // Os eixos
    pen.setWidth(2);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    // Eixo horixontal
    painter.drawLine(convX(minX),convY(0.0), convX(maxX),convY(0.0));
    // Eixo vertical
    painter.drawLine(convX(minX),convY(0.0), convX(minX),convY(maxY));

    // A indicacao das alturas dos orificios (linha tracejada no grafico)
    pen.setStyle(Qt::DashLine);
    pen.setWidth(1);
    painter.setPen(pen);
    // O orificio entre os tanques
    painter.drawLine(convX(minX),convY(Hole12Height), convX(maxX),convY(Hole12Height));
    // O orificio de transbordamento
    painter.drawLine(convX(minX),convY(OverflowHeight), convX(maxX),convY(OverflowHeight));

    // As curvas dos niveis dos tanques
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    if (points.size() > 1)
    {
      // O nivel H1
      pen.setColor(Qt::blue);
      painter.setPen(pen);
      for (unsigned i=1; i<points.size(); ++i)
      {
        painter.drawLine(convX(points[i-1].t),convY(points[i-1].h1), convX(points[i].t),convY(points[i].h1));
      }

      // O nivel H2
      pen.setColor(Qt::red);
      painter.setPen(pen);
      for (unsigned i=1; i<points.size(); ++i)
      {
        painter.drawLine(convX(points[i-1].t),convY(points[i-1].h2), convX(points[i].t),convY(points[i].h2));
      }
    }

    // Os titulos dos eixos
    QFont font = painter.font();
    font.setPixelSize(fontSizePx);
    painter.setFont(font);
    pen.setWidth(1);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    // Eixo horixontal
    painter.drawText(QRectF(convX(maxX)+imgMarginPx/5.0, convY(0.0)-imgMarginPx/2.0,
                            4.0*imgMarginPx/5.0, imgMarginPx),
                     Qt::AlignVCenter|Qt::AlignLeft,
                     "t");
    // Eixo vertical
    painter.drawText(QRectF(convX(minX)-imgMarginPx/2.0, 0.0,
                            imgMarginPx, 4.0*imgMarginPx/5.0),
                     Qt::AlignHCenter|Qt::AlignBottom,
                     "h");
    // Os limites dos eixos
    font.setPixelSize(2*fontSizePx/3);
    painter.setFont(font);
    // Eixo horixontal
    painter.drawText(QRectF(0, convY(0.0)-imgMarginPx,
                            4.0*imgMarginPx/5.0, imgMarginPx),
                     Qt::AlignBottom|Qt::AlignRight,
                     QString::number(0.0));
    painter.drawText(QRectF(0, convY(maxY),
                            4.0*imgMarginPx/5.0, imgMarginPx),
                     Qt::AlignTop|Qt::AlignRight,
                     QString::number(100.0*maxY)); // maxY em m; 100*maxY em cm
    painter.drawText(QRectF(0, 0,
                            4.0*imgMarginPx/5.0, 4.0*imgMarginPx/5.0),
                     Qt::AlignBottom|Qt::AlignRight,
                     "cm");
    // Eixo vertical
    painter.drawText(QRectF(convX(minX), convY(0.0)+imgMarginPx/5.0,
                            imgMarginPx, 4.0*imgMarginPx/5.0),
                     Qt::AlignTop|Qt::AlignLeft,
                     QString::number(int(round(minX/60.0)))); // minX em s; minX/60 em min
    painter.drawText(QRectF(convX(maxX)-imgMarginPx, convY(0.0)+imgMarginPx/5.0,
                            imgMarginPx, 4.0*imgMarginPx/5.0),
                     Qt::AlignTop|Qt::AlignRight,
                     QString::number(int(round(maxX/60.0)))); // maxX em s; maxX/60 em min
    painter.drawText(QRectF(convX(maxX)+imgMarginPx/5.0, convY(0.0)+imgMarginPx/5.0,
                            4.0*imgMarginPx/5.0, 4.0*imgMarginPx/5.0),
                     Qt::AlignTop|Qt::AlignLeft,
                     "min");
    // Legenda das curvas
    // H1
    pen.setColor(Qt::blue);
    painter.setPen(pen);
    painter.drawText(QRectF(convX((maxX+minX)/2.0)-3.0*imgMarginPx, 0.0,
                            3.0*imgMarginPx, 4.0*imgMarginPx/5.0),
                     Qt::AlignRight|Qt::AlignBottom,
                     "h1 ");
    // H2
    pen.setColor(Qt::red);
    painter.setPen(pen);
    painter.drawText(QRectF(convX((maxX+minX)/2.0), 0.0,
                            3.0*imgMarginPx, 4.0*imgMarginPx/5.0),
                     Qt::AlignLeft|Qt::AlignBottom,
                     " h2");
  }

  // Conclui o desenho
  painter.end();

  // Exibe a imagem no QLabel
  setPixmap(img);
}

void SupImg::resizeEvent(QResizeEvent* event)
{
  if (event->size() != event->oldSize())
  {
    drawImg();
  }
}
