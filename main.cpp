#include <QApplication>
#include "terminal.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Terminal w;
    w.show();
    return app.exec();
}