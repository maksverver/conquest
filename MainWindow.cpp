#include "MainWindow.h"
#include <QtDebug>
#include <QFile>

static const QRegExp re_space("\\s+");

MainWindow::MainWindow()
    : splitter(new QSplitter(Qt::Horizontal, this)),
      map_view(new MapView(splitter)),
      moves_widget(new QTreeWidget(splitter))
{
    connect(moves_widget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
        SLOT(updateMap(QTreeWidgetItem*)) );

    // Read transcript:
    QTextStream in(stdin);
    QString line, map_line;
    QTreeWidgetItem *current_round = NULL;
    while (!(line = in.readLine()).isNull())
    {
        QStringList tokens = line.split(re_space);
        if (tokens.size() > 0)
        {
            if (tokens.at(0) == "map")
            {
                map_line = line;
            }
            else
            {
                QStringList strings;
                strings.push_back(line);
                QTreeWidgetItem *new_item = new QTreeWidgetItem(strings);
                new_item->setData(0, 1, map_line);
                if (tokens[0] != "round")
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

    setCentralWidget(splitter);
}

void MainWindow::updateMap(QTreeWidgetItem *current_move)
{
    QStringList tokens = current_move->data(0, 1).toString().split(re_space);
    if (tokens.at(0) == "map")
    {
        World world;
        for (int i = 1; i < tokens.size(); ++i)
        {
            QStringList parts = tokens.at(i).split(';');
            if (parts.size() == 3)
            {
                int id        = parts[0].toInt();
                int continent = -1;  // unknown!
                int owner     = (parts[1] == "player1") - (parts[1] == "player2");
                int armies    = parts[2].toInt();
                std::vector<int> neighbours;  // unknown!

                Country country = { id, continent, owner, armies, neighbours };
                world.countries.push_back(country);
            }
        }

        // Also parse move itself:
        tokens = current_move->data(0, 0).toString().split(re_space);

        int src, dst;
        if ( tokens.size() == 4 && tokens[1] == "place_armies" &&
             (dst = world.country_index(tokens[2].toInt())) >= 0 )
        {
            Placement placement = { dst, tokens[3].toInt() };
            map_view->updateWorld(world, placement);
        }
        else
        if ( tokens.size() == 5 && tokens[1] == "attack/transfer" &&
             (src = world.country_index(tokens[2].toInt())) >= 0 &&
             (dst = world.country_index(tokens[3].toInt())) >= 0 )
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
