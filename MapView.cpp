#include <math.h>
#include <assert.h>
#include "MapView.h"
#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsPolygonItem>

static const QBrush circle_brush_inactive(QColor(160,160,160));
static const QBrush circle_brush_neutral(QColor(240,240,240));
static const QBrush circle_brush_player1(QColor(255,160,160));
static const QBrush circle_brush_player2(QColor(160,150,255));
static const QBrush edge_brush_neutral(QColor(192,192,192));
static const QBrush edge_brush_player1(QColor(255,128,128));
static const QBrush edge_brush_player2(QColor(128,128,255));

static const int num_nodes = 42;

const QPointF node_locations[num_nodes] = {

    /* North America */
    QPointF( 210.0,   0.0 ),
    QPointF( 210.0,  60.0 ),
    QPointF( 210.0, 120.0 ),
    QPointF( 120.0,   0.0 ),
    QPointF( 120.0,  60.0 ),
    QPointF( 120.0, 120.0 ),
    QPointF(  60.0,   0.0 ),
    QPointF(  60.0, 120.0 ),
    QPointF(   0.0,  60.0 ),

    /* South America */
    QPointF(   0.0, 480.0 ),
    QPointF( -60.0, 540.0 ),
    QPointF(  60.0, 540.0 ),
    QPointF(   0.0, 600.0 ),

    /* Europe */
    QPointF( 210.0,240.0 ),
    QPointF( 150.0,300.0 ),
    QPointF( 210.0,300.0 ),
    QPointF( 270.0,300.0 ),
    QPointF( 150.0,420.0 ),
    QPointF( 210.0,360.0 ),
    QPointF( 270.0,420.0 ),

    /* Africa */
    QPointF( 360.0,540.0 ),
    QPointF( 420.0,480.0 ),
    QPointF( 480.0,540.0 ),
    QPointF( 420.0,600.0 ),
    QPointF( 480.0,660.0 ),
    QPointF( 540.0,600.0 ),

    /* Asia */
    QPointF( 420.0, 180.0 ),
    QPointF( 420.0, 120.0 ),
    QPointF( 360.0,  60.0 ),
    QPointF( 420.0,   0.0 ),
    QPointF( 420.0,  60.0 ),
    QPointF( 480.0, 180.0 ),
    QPointF( 480.0, 120.0 ),
    QPointF( 480.0,  60.0 ),
    QPointF( 480.0,   0.0 ),
    QPointF( 480.0, 240.0 ),
    QPointF( 540.0, 180.0 ),
    QPointF( 540.0, 120.0 ),

    /* Australia */
    QPointF( 630.0, 120.0 ),
    QPointF( 690.0, 120.0 ),
    QPointF( 630.0, 180.0 ),
    QPointF( 690.0, 180.0 ) };

MapView::MapView(QWidget *parent)
    : QGraphicsView(parent), scene(new QGraphicsScene())
{
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);

    Map map = Map::getDefault();

    for (int i = 0; i < num_nodes; ++i)
    {
        NodeEllipse *circle = new NodeEllipse(i, QRectF(-15, -15, 30, 30));
        circle->setPos(node_locations[i]);
        circle->setBrush(circle_brush_neutral);
        country_circles.push_back(circle);

        QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem("2");
        QFont font = QApplication::font();
        font.setPixelSize(14);
        label->setFont(font);
        label->setPos(circle->pos() - label->boundingRect().center());
        country_texts.push_back(label);
    }

    for (size_t i = 0; i < map.countries.size(); ++i)
    {
        for (size_t n = 0; n < map.countries[i].neighbours.size(); ++n)
        {
            int j = map.countries[i].neighbours[n];
            edge_index[std::make_pair((int)i, (int)j)] = edge_polygons.size();

            QPointF p = node_locations[i];
            QPointF q = node_locations[j];
            QPointF dir = q - p;
            dir /= hypot(dir.x(), dir.y());

            p += dir*15;
            q -= dir*15;
            QPointF lead = QPointF(-dir.y(), dir.x());

            QPointF r = (p + q)/2;
            QPolygonF polygon;
            polygon.push_back(p +  4*lead);
            polygon.push_back(r +  4*lead -  8*dir);
            polygon.push_back(r +  8*lead -  8*dir);
            polygon.push_back(r);
            polygon.push_back(r -  8*lead -  8*dir);
            polygon.push_back(r -  4*lead -  8*dir);
            polygon.push_back(p -  4*lead);

            EdgePolygon *poly = new EdgePolygon(i, j, polygon);
            poly->setPen(Qt::NoPen);
            poly->setBrush(edge_brush_neutral);
            edge_polygons.push_back(poly);

            QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem("0");
            QFont font = QApplication::font();
            font.setPixelSize(15);
            label->setFont(font);
            QPointF offset = p + 10*dir - label->boundingRect().center();
            label->moveBy(offset.x(), offset.y());
            label->setText("");
            edge_texts.push_back(label);
        }
    }

    // Add everything to the scene (make sure stuff is added in the correct
    // order, so nodes/edges appear below labels etc.)
    for (size_t i = 0; i < edge_polygons.size(); ++i)
    {
        scene->addItem(edge_polygons[i]);
    }
    for (size_t i = 0; i < edge_texts.size(); ++i)
    {
        scene->addItem(edge_texts[i]);
    }
    for (size_t i = 0; i < country_circles.size(); ++i)
    {
        scene->addItem(country_circles[i]);
    }
    for (size_t i = 0; i < country_texts.size(); ++i)
    {
        scene->addItem(country_texts[i]);
    }
}

void MapView::updateWorld(const World &world)
{
    updateWorld(world, NULL, 0, NULL, 0);
}

void MapView::updateWorld(const World &world, const Placement &placement)
{
    updateWorld(world, &placement, 1, NULL, 0);
}

void MapView::updateWorld(const World &world, const std::vector<Placement> &placements)
{
    updateWorld(world,(placements.empty() ? 0 : &placements[0]), placements.size(), NULL, 0);
}

void MapView::updateWorld(const World &world, const Movement &movement)
{
    updateWorld(world, NULL, 0, &movement, 1);
}

void MapView::updateWorld(const World &world, const std::vector<Movement> &movements)
{
    updateWorld(world, NULL, 0, (movements.empty() ? 0 : &movements[0]),  movements.size());
}

void MapView::updateWorld( const World &world,
                           const std::vector<Placement> &placements,
                           const std::vector<Movement> &movements )
{
    updateWorld( world, (placements.empty() ? 0 : &placements[0]), placements.size(),
                        (movements.empty()  ? 0: &movements[0]),  movements.size() );
}

void MapView::updateWorld( const World &world,
                           const Placement *placements, size_t nplacement,
                           const Movement *movements, size_t nmovement )
{
    assert(world.map.countries.size() == num_nodes);
    for (int i = 0; i < num_nodes; ++i)
    {
        int o = world.occupations[i].owner;
        int a = world.occupations[i].armies;
        country_circles[i]->setBrush( a == 0 ? circle_brush_inactive :
                                      o > 0  ? circle_brush_player1 :
                                      o < 0  ? circle_brush_player2 :
                                               circle_brush_neutral );
        int extra = 0;
        for (size_t n = 0; n < nplacement; ++n)
        {
            if (placements[n].dst == i) extra += placements[n].armies;
        }
        for (size_t n = 0; n < nmovement; ++n)
        {
            if (movements[n].src == i)
            {
                a += extra;
                extra = 0;
                a -= movements[n].armies;
            }
        }
        QString text = QString::number(a);
        if (extra > 0) text += "+" + QString::number(extra);
        country_texts[i]->setText(text);
        country_texts[i]->setPos( country_circles[i]->pos()
            - country_texts[i]->boundingRect().center() );
    }
    for (size_t k = 0; k < edge_polygons.size(); ++k)
    {
        EdgePolygon *edge = edge_polygons[k];
        int armies = 0;
        for (size_t n = 0; n < nmovement; ++n)
        {
            if ( movements[n].src == edge->src && 
                 movements[n].dst == edge->dst )
            {
                armies += movements[n].armies;
            }
        }
        if (armies == 0)
        {
            edge_polygons[k]->setBrush(edge_brush_neutral);
            edge_texts[k]->setText("");
        }
        else
        {
            int o = world.occupations[edge->src].owner;
            edge_polygons[k]->setBrush( o > 0 ? edge_brush_player1 :
                                        o < 0 ? edge_brush_player2 :
                                                edge_brush_neutral );
            edge_texts[k]->setText(QString::number(armies));
        }
    }
}
