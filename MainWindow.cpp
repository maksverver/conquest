#include "MainWindow.h"
#include <QtDebug>
#include <QFile>

static const QRegExp re_space("\\s+");
static Map the_map = Map::getDefault();

MainWindow::MainWindow()
    : splitter(new QSplitter(Qt::Horizontal, this)),
      map_view(new MapView(splitter)),
      moves_widget(new QTreeWidget(splitter)),
      current_round(NULL)
{
    connect(moves_widget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
        SLOT(updateMap(QTreeWidgetItem*)) );

    setCentralWidget(splitter);
}

void MainWindow::addToTranscript(QString line)
{
    QStringList tokens = line.split(re_space);
    if (tokens.size() > 0)
    {
        if (tokens.at(0) == "map")
        {
            current_map_line = line;
        }
        else
        {
            QStringList strings;
            strings.push_back(line);
            QTreeWidgetItem *new_item = new QTreeWidgetItem(strings);
            new_item->setData(0, 1, current_map_line);
            if (tokens[0] != "round" && current_round != NULL)
            {
                current_round->addChild(new_item);
            }
            else
            {
                moves_widget->addTopLevelItem(new_item);
                if (tokens[0] == "round") current_round = new_item;
            }
        }
    }
}

void MainWindow::updateMap(QTreeWidgetItem *current_move)
{
    QStringList tokens = current_move->data(0, 1).toString().split(re_space);
    if (tokens.at(0) == "map")
    {
        World world(the_map);  // use default map
        for (int i = 1; i < tokens.size(); ++i)
        {
            QStringList parts = tokens.at(i).split(';');
            if (parts.size() == 3)
            {
                int country_id = parts[0].toInt();
                int owner      = (parts[1] == "player1")
                               - (parts[1] == "player2");
                int armies     = parts[2].toInt();
                Occupation occ = { owner, armies };
                world.occupations[world.map.country_index(country_id)] = occ;
            }
        }

        // Also parse move itself:
        tokens = current_move->data(0, 0).toString().split(re_space);

        int src, dst;
        if ( tokens.size() == 4 && tokens[1] == "place_armies" &&
             (dst = world.map.country_index(tokens[2].toInt())) >= 0 )
        {
            Placement placement = { dst, tokens[3].toInt() };
            map_view->updateWorld(world, placement);
        }
        else
        if ( tokens.size() == 5 && tokens[1] == "attack/transfer" &&
             (src = world.map.country_index(tokens[2].toInt())) >= 0 &&
             (dst = world.map.country_index(tokens[3].toInt())) >= 0 )
        {
            Movement movement = { src, dst, tokens[4].toInt() };
            map_view->updateWorld(world, movement);
        }
        else
        {
            map_view->updateWorld(world);
        }
    }
}
