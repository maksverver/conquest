#ifndef MAP_CONTROL_H_INCLUDED
#define MAP_CONTROL_H_INCLUDED

#include "MapView.h"

class MapControl : public MapView
{
    Q_OBJECT

public:
    MapControl(const World &world, QWidget *parent = 0);
    void update();
    void enable(int player, int armies);
    void disable();

    const std::vector<Placement> &getPlacements() { return placements; }
    const std::vector<Movement> &getMovements() { return movements; }

protected:
    void wheelEvent(QWheelEvent *event);
    bool incNode(int i);
    bool decNode(int i);
    bool incEdge(int i, int j);
    bool decEdge(int i, int j);

protected:
    const World &world;
    int player, armies;
    std::vector<Placement> placements;
    std::vector<Movement> movements;
};

#endif /* ndef MAP_CONTROL_H_INCLUDED */
