#include "LineReader.h"

void LineReader::readLines()
{
    QString line;
    while (!(line = input.readLine()).isNull())
    {
        emit lineRead(line);
    }
}
