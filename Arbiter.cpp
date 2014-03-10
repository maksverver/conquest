#include "Arbiter.h"
#include <assert.h>
#include <stdlib.h>

static const int timeout_ms     = 2000;
static const int starting_bonus =    5;

// Returns a random Boolean which is 1 with probability num/den (approximately).
static bool prob(int num, int den)
{
    assert(0 <= num && num <= den);  // probability should be between 0 and 1
    return rand() < ((long long)RAND_MAX + 1)*num/den;
}

static const char *player_name(int player)
{
    return (player > 0) ? "player1" : (player < 0) ? "player2" : "neutral";
}

Arbiter::Arbiter(const World &w, Player &p1, Player &p2, std::ostream *l)
    : world(w), player1(p1), player2(p2), log(l)
{
    total_countries[0] = 0;
    total_countries[1] = 0;
    continent_countries = new int[world.continents.size()][2]();
    bonus[0] = starting_bonus;
    bonus[1] = starting_bonus;

    // Calculate initial country/continent occupation:
    for (size_t i = 0; i < world.countries.size(); ++i)
    {
        int o = world.countries[i].owner;
        if (o == 0) continue;  // ignore neutral countries
        ++total_countries[o < 0];
        int c = world.countries[i].continent;
        if (c < 0) continue;
        if ( ++continent_countries[c][o < 0] ==
                (int)world.continents[c].countries.size() )
        {
            // Add continent bonus:
            bonus[o < 0] += world.continents[c].bonus;
        }
    }

    log_map();
}

Arbiter::~Arbiter()
{
    delete[] continent_countries;
}

int Arbiter::winner() const
{
    return (total_countries[0] > 0) - (total_countries[1] > 0);
}

int Arbiter::play_game(int max_turns)
{
    for (int turn = 1; turn <= max_turns; ++turn)
    {
        int w = winner();
        if (w != 0) return w;
        if (log) *log << "round" << ' ' << turn << '\n';
        play_turn();
    }
    return 0;  // it's a tie.
}

void Arbiter::play_turn()
{
    play_placement_phase();
    play_movement_phase();
}

void Arbiter::play_placement_phase()
{
    World world1 = world, world2 = world;
    world1.applyFog(+1);
    world2.applyFog(-1);
    world2.dualize();

    place_armies(+1, bonus[0], player1.place_armies(world1, bonus[0], timeout_ms));
    place_armies(-1, bonus[1], player2.place_armies(world2, bonus[1], timeout_ms));
}

void Arbiter::place_armies( int player, int armies,
                            const std::vector<Placement> &placements )
{
    for ( std::vector<Placement>::const_iterator it = placements.begin();
          it != placements.end(); ++it )
    {
        Country &cy = world.countries[it->dst];
        if (log)
        {
            *log << player_name(player) << " place_armies "
                 << cy.id << ' ' << it->armies << '\n';
        }
        assert(cy.owner == player);
        cy.armies += it->armies;
        armies -= it->armies;
        log_map();
    }
    assert(armies >= 0);
}

std::vector<Movement> Arbiter::get_attack_transfer_moves(int player)
{
    assert(player != 0);
    World foggy_world = world;
    foggy_world.applyFog(player);
    if (player < 0) foggy_world.dualize();
    return ((player > 0) ? player1 : player2).attack_transfer(foggy_world, timeout_ms);
}

void Arbiter::play_movement_phase()
{
    std::vector<Movement> movements1 = get_attack_transfer_moves(+1),
                          movements2 = get_attack_transfer_moves(-1);
    std::vector<int> reinforcements(world.countries.size());
    for (size_t n = 0; n < movements1.size() || n < movements2.size(); ++n)
    {
        if (n >= movements2.size())  // only player 1 has moves left
        {
            attack_transfer(+1, movements1[n], reinforcements);
        }
        else
        if (n >= movements1.size())  // only player 2 has moves left
        {
            attack_transfer(-1, movements2[n], reinforcements);
        }
        else
        if (prob(1, 2))  // player 1 moves first
        {
            attack_transfer(+1, movements1[n], reinforcements);
            attack_transfer(-1, movements2[n], reinforcements);
        }
        else  // player 2 moves first
        {
            attack_transfer(-1, movements2[n], reinforcements);
            attack_transfer(+1, movements1[n], reinforcements);
        }
    }
    for (size_t n = 0; n < world.countries.size(); ++n)
    {
        world.countries[n].armies += reinforcements[n];
    }
}

static int count_kills(int prob_num, int prob_den, int armies)
{
    int killed = 0;
    while (armies-- > 0)
    {
        if (prob(prob_num, prob_den)) ++killed;
    }
    return killed;
}

void Arbiter::attack_transfer( int player, const Movement &movement,
                               std::vector<int> &reinforcements )
{
    Country &src = world.countries[movement.src];
    Country &dst = world.countries[movement.dst];

    // Verify country still belongs to moving player, otherwise skip this move:
    if (src.owner != player) return;

    // Calculate how many armies will be used in the movement:
    int armies = std::min(movement.armies, src.armies - 1);
    if (armies <= 0) return;  // no armies to move left!

    if (log)
    {
        *log << player_name(player) << " attack/transfer "
             << src.id << ' ' << dst.id << ' ' << armies << '\n';
    }

    // Remove armies from source country.
    src.armies -= armies;

    if (dst.owner == player)
    {
        // Transfer to destination country.
        reinforcements[movement.dst] += armies;
    }
    else  // attack!
    {
        const int defenders = dst.armies + reinforcements[movement.dst];
        int defenders_killed = count_kills(6, 10, armies);
        int attackers_killed = count_kills(7, 10, defenders);

        if (defenders_killed >= defenders)
        {
            // Attack succeeded.
            if (attackers_killed < armies)
            {
                dst.armies = 0;
                reinforcements[movement.dst] = armies - attackers_killed;
                change_owner(dst, player);
            }
            else  // special case: everyone is dead!
            {
                dst.armies = 1;  // resurrect a lone defender.
                reinforcements[movement.dst] = 0;
            }
        }
        else
        {
            // Attack failed.  Repatriate attackers.
            if (attackers_killed < armies)
            {
                /* N.B. these are added to the reinforcements to prevent them
                        from being re-used in a leter move! */
                reinforcements[movement.src] += armies - attackers_killed;
            }

            /* Update defenders, killing reinforcements first, which allows
               the remaining defenders to attack in a later move. */
            if (defenders_killed < reinforcements[movement.dst])
            {
                reinforcements[movement.dst] -= defenders_killed;
            }
            else
            {
                dst.armies -= defenders_killed - reinforcements[movement.dst];
                reinforcements[movement.dst] = 0;
            }
        }
    }
    log_map(&reinforcements);
}

void Arbiter::change_owner(Country &country, int new_owner)
{
    int old_owner = country.owner;
    country.owner = new_owner;

    // Update total country count.
    if (old_owner != 0) --total_countries[old_owner < 0];
    if (new_owner != 0) ++total_countries[new_owner < 0];

    if (country.continent >= 0)
    {
        // Update continent country counts (and possibly bonus):
        int (&cc)[2] = continent_countries[country.continent];
        int size     = world.continents[country.continent].countries.size();
        int extra    = world.continents[country.continent].bonus;
        if (old_owner != 0 && cc[old_owner < 0]-- == size)
        {
            bonus[old_owner < 0] -= extra;
        }
        if (new_owner != 0 && ++cc[new_owner < 0] == size)
        {
            bonus[new_owner < 0] += extra;
        }
    }
}

void Arbiter::log_map(std::vector<int> *reinforcements)
{
    if (log)
    {
        *log << "map";
        for (size_t i = 0; i < world.countries.size(); ++i)
        {
            const Country &cy = world.countries[i];
            int armies = cy.armies;
            if (reinforcements) armies += (*reinforcements)[i];
            *log << ' ' << cy.id
                 << ';' << player_name(cy.owner)
                 << ';' << armies;
        }
        *log << '\n';
    }
}
