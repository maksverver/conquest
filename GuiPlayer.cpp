#include "GuiPlayer.h"
#include <QApplication>
#include <assert.h>
#include <iostream>

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#endif

static int argc = 2;
static char *argv[] = { (char*)"Conquest Player", NULL };

bool compare_armies(const Placement &p, const Placement &q)
{
    return p.armies < q.armies;
}

std::vector<int> GuiPlayer::pick_starting_countries(
    const World      &world,
    std::vector<int> options,
    int              num_countries,
    int              UNUSED timeout_ms )
{
    /* Hack: use the MapControl to get the starting countries.
       The user is supposed to place 1 army on his most preferred country,
       2 on the second-most preferred, and so on.  Countries with 0 armies are
       moved to the end of the list. */
    World w = world;
    Occupation empty = { 0, 0 };
    w.occupations.assign(w.occupations.size(), empty);
    for (size_t n = 0; n < options.size(); ++n)
    {
        w.occupations[options[n]].owner  = +1;
        w.occupations[options[n]].armies =  1;
    }
    QApplication app(argc, argv);
    MapControl control(w);
    control.enable(+1, 2*num_countries*(2*num_countries + 1)/2);
    control.show();
    app.exec();
    std::vector<Placement> placements = control.getPlacements();
    std::sort(placements.begin(), placements.end(), compare_armies);
    std::vector<int> prefs;
    for (size_t n = 0; n < placements.size(); ++n)
    {
        prefs.push_back(placements[n].dst);
    }
    for (size_t n = 0; n < options.size(); ++n)
    {
        if (std::find(prefs.begin(), prefs.end(), options[n]) == prefs.end())
        {
            prefs.push_back(options[n]);
        }
    }
    return prefs;
}

std::vector<Placement> GuiPlayer::place_armies( const World &world,
                                                int num_armies,
                                                int UNUSED timeout_ms )
{
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
