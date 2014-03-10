#include "GuiPlayer.h"
#include <QApplication>
#include <assert.h>
#include <iostream>

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#endif

std::vector<int> GuiPlayer::pick_starting_countries(
    const World      UNUSED &world,
    std::vector<int> UNUSED options,
    int              UNUSED num_countries,
    int              UNUSED timeout_ms )
{
    assert(false);  // NOT IMPLEMENTED!
    return std::vector<int>();
}

std::vector<Placement> GuiPlayer::place_armies( const World &world,
                                                int num_armies,
                                                int UNUSED timeout_ms )
{
    int argc = 2;
    char *argv[] = { (char*)"Conquest Player", NULL };
    QApplication app(argc, argv);
    MapControl control(world);
    control.enable(+1, num_armies);
    control.show();
    app.exec();
    movements = control.getMovements();
    return control.getPlacements();
}

std::vector<Movement> GuiPlayer::attack_transfer( const World UNUSED &world,
                                                  int UNUSED timeout_ms )
{
    return movements;
}
