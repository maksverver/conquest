#include "Arbiter.h"
#include <algorithm>
#include <random>
#include <assert.h>
#include <stdlib.h>

static std::mt19937 rng((std::random_device())());

static const int timeout_ms             = 2000;
static const int starting_bonus         =    5;
static const int num_starting_countries =    3;

// Returns a random Boolean which is 1 with probability num/den (approximately).
static bool prob(int num, int den)
{
    std::uniform_int_distribution<> distribution(0, den - 1);
    return distribution(rng) < num;
}

static const char *player_name(int player)
{
    return (player > 0) ? "player1" : (player < 0) ? "player2" : "neutral";
}

Arbiter::Arbiter(const Map &m, Player &p1, Player &p2, std::ostream *l)
    : world(m), player1(p1), player2(p2), log(l)
{
    pick_starting_countries();
    initialize_bonus();
    log_map();
}

Arbiter::Arbiter(const World &w, Player &p1, Player &p2, std::ostream *l)
    : world(w), player1(p1), player2(p2), log(l)
{
    initialize_bonus();
    log_map();
}

Arbiter::~Arbiter()
{
    delete[] continent_countries;
}

void Arbiter::initialize_bonus()
{
    total_countries[0] = 0;
    total_countries[1] = 0;
    continent_countries = new int[world.map.continents.size()][2]();
    bonus[0] = starting_bonus;
    bonus[1] = starting_bonus;

    // Calculate initial country/continent occupation:
        for (size_t i = 0; i < world.map.countries.size(); ++i)
    {
        int o = world.occupations[i].owner;
        if (o == 0) continue;  // ignore neutral countries
        ++total_countries[o < 0];
        int c = world.map.countries[i].continent;
        if (c < 0) continue;
        if ( ++continent_countries[c][o < 0] ==
                (int)world.map.continents[c].countries.size() )
        {
            // Add continent bonus:
            bonus[o < 0] += world.map.continents[c].bonus;
        }
    }
}

void Arbiter::pick_starting_countries()
{
    // Pick list of starting options; two per continent
    std::vector<int> options;
    for (size_t i = 0; i < world.map.continents.size(); ++i)
    {
        std::vector<int> countries = world.map.continents[i].countries;
        assert(countries.size() >= 2);
        std::shuffle(countries.begin(), countries.end(), rng);
        options.push_back(countries[0]);
        options.push_back(countries[1]);
    }
    std::shuffle(options.begin(), options.end(), rng);
    assert(options.size() >= 2*num_starting_countries);

    // Get preferences from players:
    std::vector<int> prefs[2] = {
        player1.pick_starting_countries(world, options, num_starting_countries, timeout_ms),
        player2.pick_starting_countries(world, options, num_starting_countries, timeout_ms) };

    // Assign initial countries, breaking conflicts randomly:
    int idx[2] = { 0, 0 };
    for (int n = 0; n < num_starting_countries; ++n)
    {
        while (world.occupations[prefs[0][idx[0]]].owner != 0) ++idx[0];
        while (world.occupations[prefs[1][idx[1]]].owner != 0) ++idx[1];
        if (prefs[0][idx[0]] == prefs[1][idx[1]])
        {
            int i = prob(1, 2);  // player i gives up his choice
            while ( prefs[i][idx[i]] == prefs[1 - i][idx[1 - i]] ||
                    world.occupations[prefs[i][idx[i]]].owner != 0) ++idx[i];
        }
        world.occupations[prefs[0][idx[0]++]].owner = +1;
        world.occupations[prefs[1][idx[1]++]].owner = -1;
    }
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
    place_armies(+1, bonus[0], player1.place_armies(World(world, +1), bonus[0], timeout_ms));
    place_armies(-1, bonus[1], player2.place_armies(World(world, -1), bonus[1], timeout_ms));
}

void Arbiter::place_armies( int player, int armies,
                            const std::vector<Placement> &placements )
{
    for (const Placement &p : placements)
    {
        if (log)
        {
            *log << player_name(player) << " place_armies "
                 << world.map.countries[p.dst].id
                 << ' ' << p.armies << '\n';
        }
        assert(world.occupations[p.dst].owner == player);
        world.occupations[p.dst].armies += p.armies;
        armies -= p.armies;
        log_map();
    }
    assert(armies >= 0);
}

std::vector<Movement> Arbiter::get_attack_transfer_moves(int p)
{
    assert(p != 0);
    Player &player = (p > 0) ? player1 : player2;
    return player.attack_transfer(World(world, p), timeout_ms);
}

void Arbiter::play_movement_phase()
{
    std::vector<Movement> movements1 = get_attack_transfer_moves(+1),
                          movements2 = get_attack_transfer_moves(-1);
    std::vector<int> reinforcements(world.map.countries.size());
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
    for (size_t n = 0; n < world.map.countries.size(); ++n)
    {
        world.occupations[n].armies += reinforcements[n];
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
    Occupation &src_occ = world.occupations[movement.src];
    Occupation &dst_occ = world.occupations[movement.dst];

    // Verify country still belongs to moving player, otherwise skip this move:
    if (src_occ.owner != player) return;

    // Calculate how many armies will be used in the movement:
    int armies = std::min(movement.armies, src_occ.armies - 1);
    if (armies <= 0) return;  // no armies to move left!

    if (log)
    {
        *log << player_name(player) << " attack/transfer "
             << world.map.countries[movement.src].id << ' '
             << world.map.countries[movement.dst].id << ' '
             << armies << '\n';
    }

    // Remove armies from source country.
    src_occ.armies -= armies;

    if (dst_occ.owner == player)
    {
        // Transfer to destination country.
        reinforcements[movement.dst] += armies;
    }
    else  // attack!
    {
        const int defenders = dst_occ.armies + reinforcements[movement.dst];
        int defenders_killed = count_kills(6, 10, armies);
        int attackers_killed = count_kills(7, 10, defenders);

        if (defenders_killed >= defenders)
        {
            // Attack succeeded.
            if (attackers_killed < armies)
            {
                dst_occ.armies = 0;
                reinforcements[movement.dst] = armies - attackers_killed;
                change_owner(movement.dst, player);
            }
            else  // special case: everyone is dead!
            {
                dst_occ.armies = 1;  // resurrect a lone defender.
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
                dst_occ.armies -= defenders_killed - reinforcements[movement.dst];
                reinforcements[movement.dst] = 0;
            }
        }
    }
    log_map(&reinforcements);
}

void Arbiter::change_owner(int country, int new_owner)
{
    int old_owner = world.occupations[country].owner;
    world.occupations[country].owner = new_owner;

    // Update total country count.
    if (old_owner != 0) --total_countries[old_owner < 0];
    if (new_owner != 0) ++total_countries[new_owner < 0];

    int continent = world.map.countries[country].continent;
    if (continent >= 0)
    {
        // Update continent country counts (and possibly bonus) for each player:
        int (&cc)[2] = continent_countries[continent];
        int size     = world.map.continents[continent].countries.size();
        int extra    = world.map.continents[continent].bonus;
        if (old_owner != 0 && cc[old_owner < 0]-- == size) bonus[old_owner < 0] -= extra;
        if (new_owner != 0 && ++cc[new_owner < 0] == size) bonus[new_owner < 0] += extra;
    }
}

void Arbiter::log_map(std::vector<int> *reinforcements)
{
    if (log)
    {
        *log << "map";
        for (size_t i = 0; i < world.map.countries.size(); ++i)
        {
            *log << ' ' << world.map.countries[i].id
                 << ';' << player_name(world.occupations[i].owner)
                 << ';' << ( world.occupations[i].armies +
                             (reinforcements ? (*reinforcements)[i] : 0) );
        }
        *log << std::endl;
    }
}
