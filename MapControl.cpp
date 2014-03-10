#include "MapControl.h"

MapControl::MapControl(const World &w, QWidget *parent)
    : MapView(parent), world(w), player(0), armies(0)
{
    update();
}

void MapControl::enable(int player, int armies)
{
    this->player = player;
    this->armies = armies;
}

void MapControl::disable()
{
    player = 0;
    armies = 0;
    placements.clear();
    movements.clear();
}

void MapControl::update()
{
    updateWorld(world, placements, movements);
}

bool MapControl::incNode(int i)
{
    // Verify that the source node belongs to us:
    if (world.occupations[i].owner != player) return false;

    // Verify that we have armies left to place:
    if (armies <= 0) return false;

    // Verify that the clicked node has not yet been moved from:
    for ( std::vector<Movement>::iterator it = movements.begin();
          it != movements.end(); ++it )
    {
        if (it->src == i) return false;
    }

    // Find an existing placement to update:
    for ( std::vector<Placement>::iterator it = placements.begin();
          it != placements.end(); ++it )
    {
        if (it->dst == i)
        {
            ++it->armies;
            --armies;
            return true;
        }
    }
    Placement place = { i, 1 };
    placements.push_back(place);
    --armies;
    return true;
}

bool MapControl::decNode(int i)
{
    // Verify that the clicked node has not yet been moved from:
    for ( std::vector<Movement>::iterator it = movements.begin();
            it != movements.end(); ++it )
    {
        if (it->src == i) return false;
    }

    // Find an existing placement to update:
    for ( std::vector<Placement>::iterator it = placements.begin();
          it != placements.end(); ++it )
    {
        if (it->dst == i)
        {
            --it->armies;
            if (it->armies == 0) placements.erase(it);
            ++armies;
            return true;
        }
    }
    return false;
}

bool MapControl::incEdge(int i, int j)
{
    // Verify that the source node belongs to us:
    if (world.occupations[i].owner != player) return false;

    // Check how many armies the source node has left:
    int left = world.occupations[i].armies - 1;
    for ( std::vector<Placement>::iterator it = placements.begin();
          it != placements.end(); ++it )
    {
        if (it->dst == i) left += it->armies;
    }
    for ( std::vector<Movement>::iterator it = movements.begin();
          it != movements.end(); ++it )
    {
        if (it->src == i) left -= it->armies;
    }
    if (left <= 0) return false;

    // Search for an existing movement to increment:
    for ( std::vector<Movement>::iterator it = movements.begin();
          it != movements.end(); ++it )
    {
        if (it->src == i && it->dst == j)
        {
            ++it->armies;
            return true;
        }
    }
    Movement movement = { i, j, 1 };
    movements.push_back(movement);
    return true;
}

bool MapControl::decEdge(int i, int j)
{
    // Find existing movement:
    for ( std::vector<Movement>::iterator it = movements.begin();
            it != movements.end(); ++it )
    {
        if (it->src == i && it->dst == j)
        {
            --it->armies;
            if (it->armies == 0) movements.erase(it);
            return true;
        }
    }
    return false;
}

void MapControl::wheelEvent(QWheelEvent *event)
{
    int diff = (event->delta() > 0) - (event->delta() < 0);
    if (player == 0 || diff == 0) return;

    QList<QGraphicsItem*> item_list = items(event->pos());
    for ( QList<QGraphicsItem *>::iterator it = item_list.begin();
          it != item_list.end(); ++it )
    {
        QGraphicsItem *item = *it;

        NodeEllipse *node = dynamic_cast<NodeEllipse*>(item);
        if (node && ( (diff > 0 && incNode(node->index)) ||
                      (diff < 0 && decNode(node->index)) ) )
        {
            update();
            break;
        }

        EdgePolygon *edge = dynamic_cast<EdgePolygon*>(item);
        if (edge && ( (diff > 0 && incEdge(edge->src, edge->dst)) ||
                      (diff < 0 && decEdge(edge->src, edge->dst)) ) )
        {
            update();
            break;
        }
    }
}
