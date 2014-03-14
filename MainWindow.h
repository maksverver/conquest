#ifndef MAIN_WINDOW_H_INCLUDED
#define MAIN_WINDOW_H_INCLUDED

#include <QMainWindow>
#include <QTreeWidget>
#include <QSplitter>
#include "MapView.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected slots:
    void addToTranscript(QString line);
    void updateMap(QTreeWidgetItem *current_move);

protected:
    QSplitter *splitter;
    MapView *map_view;
    QTreeWidget *moves_widget;
    QTreeWidgetItem *current_round;
    QString current_map_line;
};

#endif /* ndef  MAIN_WINDOW_H_INCLUDED */
