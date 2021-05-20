#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace self {

MainWindow::MainWindow(::QWidget* parent) : ::QMainWindow(parent), _ui(new ::Ui::MainWindow()) {
    _ui->setupUi(this);
}

MainWindow::~MainWindow() {
    assert(_ui != nullptr);
    delete _ui;
}

} // namespace self
