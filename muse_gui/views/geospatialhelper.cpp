#include "geospatialhelper.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <QPixmapCache>

GeospatialHelper::GeospatialHelper(QDir dir, QObject* parent) : QObject(parent), dir{ dir } {
}

void
GeospatialHelper::work() {
    using namespace std::literals::chrono_literals;

    int zoom = dir.dirName().split("zoom")[1].toInt();

    std::vector<QImage> images;

    dir.setNameFilters(QStringList{ "*.png" });
    dir.setFilter(QDir::Files);

    QStringList files = dir.entryList();

    std::sort(std::begin(files), std::end(files), [] (const QString& str1, const QString& str2) {
        QString newStr1 = str1.split(".")[0];
        QString newStr2 = str2.split(".")[0];

        int x1 = newStr1.split("_")[0].toInt();
        int y1 = newStr1.split("_")[1].toInt();
        int x2 = newStr2.split("_")[0].toInt();
        int y2 = newStr2.split("_")[1].toInt();

        if (x1 == x2) {
            if (y1 <= y2) {
                return true;
            } else {
                return false;
            }
        }

        if (x1 < x2) {
            return true;
        }

        return false;
    });

    for (const auto& str : files) {
        std::cout << str.toStdString() << std::endl;
    }

    std::cout << "loading zoom level: " << zoom << std::endl;
    std::cout << "file count for zoom level: " << zoom << " is " << files.size() << std::endl;
    std::cout << "dir location: " << dir.absolutePath().toStdString() << std::endl;

    //int count = 0;

    for (const auto& file : files) {
        //if (count % 10 == 0)
        //    std::cout << "file: " << count << "/" << files.size() << std::endl;

        //QPixmap t{ dir.absoluteFilePath(file) };
        //QPixmap t{ "/home/kyle/.local/share/MUSE/maps/zoom4/" + QString::number(x) + "_" + QString::number(y) + ".png" };

        images.push_back(QImage{ dir.absoluteFilePath(file) });
        //count++;

        std::this_thread::sleep_for(100ns);
    }

    std::cout << "done loading zoom level: " << zoom << std::endl;

    emit finished(zoom, images);
}
