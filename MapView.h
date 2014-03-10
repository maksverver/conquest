#ifndef MAP_VIEW_H_INCLUDED
#define MAP_VIEW_H_INCLUDED

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QWheelEvent>
#include <vector>
#include <map>
#include <utility>

#include "world.h"

class MapView : public QGraphicsView
{
    Q_OBJECT

public:
    MapView(QWidget *parent = 0);

    void updateWorld(const World &world);
    void updateWorld(const World &world, const Placement &placement);
    void updateWorld(const World &world, const Movement &movement);
    void updateWorld(const World &world, const std::vector<Placement> &placements);
    void updateWorld(const World &world, const std::vector<Movement> &movements);
    void updateWorld(const World &world, const std::vector<Placement> &placements,
                                         const std::vector<Movement> &movements);

protected:
    void updateWorld( const World &world,
                      const Placement *placements, size_t nplacement,
                      const Movement *movements, size_t nmovement );

    class NodeEllipse : public QGraphicsEllipseItem
    {
    public:
        const int index;

        NodeEllipse(int i, QRectF rect)
            : QGraphicsEllipseItem(rect), index(i) { }
    };

    class EdgePolygon : public QGraphicsPolygonItem
    {
    public:
        const int src, dst;

        EdgePolygon(int i, int j, const QPolygonF &poly)
            : QGraphicsPolygonItem(poly), src(i), dst(j) { }
    };

private:
    QGraphicsScene *scene;

    std::vector<NodeEllipse*>             country_circles;
    std::vector<QGraphicsSimpleTextItem*> country_texts;
    std::map<std::pair<int,int>, int>     edge_index;
    std::vector<EdgePolygon*>             edge_polygons;
    std::vector<QGraphicsSimpleTextItem*> edge_texts;
};

#endif /* ndef MAP_VIEW_H_INCLUDED */
