#ifndef LINE_READER_H_INCLUDED
#define LINE_READER_H_INCLUDED

#include <QObject>
#include <QTextStream>

class LineReader : public QObject
{
    Q_OBJECT

public:
    LineReader(QTextStream &in) : input(in) { }

public slots:
    void readLines();

signals:
    void lineRead(const QString &line);

protected:
    QTextStream &input;
};

#endif /* ndef LINE_READER_H_INCLUDED */
