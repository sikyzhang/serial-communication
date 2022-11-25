#include "CommunicatorDialog.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    auto *dialog = new CommunicatorDialog();
    dialog->show();

    return QApplication::exec();
}
