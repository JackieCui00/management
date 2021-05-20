#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

namespace self {

MainWindow::MainWindow(::QWidget* parent) : ::QMainWindow(parent), _ui(new ::Ui::MainWindow()) {
    _ui->setupUi(this);

    { // fix tab names
        auto tab = _ui->tabWidget;
        for (int i = 0; i < tab->count(); ++i) {
            if (_ui->trade_tab == tab->widget(i)) {
                tab->setTabText(i, "Trade");
            }
        }

        _ui->trade_table->verticalHeader()->setVisible(false);
    }
}

MainWindow::~MainWindow() {
    assert(_ui != nullptr);
    delete _ui;
}

bool MainWindow::start() {
    if (!this->setup_db()) {
        return false;
    }

    if (!this->refresh_data()) {
        return false;
    }

    return true;
}

void MainWindow::stop() {
    if (_db.isOpen()) {
        _db.close();
    }
}

bool MainWindow::setup_db() {
    _db = ::QSqlDatabase::addDatabase("QSQLITE");
    const ::QString db_path = ::QDir::currentPath() + "/db";
    _db.setDatabaseName(db_path);

    if (!_db.open()) {
        qDebug() << "Fail to open db:" << db_path << ", error:" << _db.lastError();
        return false;
    }

    { // prepare tables
        auto exec_sql = [this] (const ::QString& sql, const char* const explain) {
            auto query = _db.exec(sql);
            if (query.lastError().type() != ::QSqlError::NoError) {
                qDebug() << "Fail to " << explain << ", sql:<" << sql << ">, error:" << query.lastError();
                return false;
            }
            qDebug() << "Succ to " << explain;
            return true;
        };

        // accounts table
        const ::QString accounts_sql = "CREATE TABLE IF NOT EXISTS accounts "
            "(name VARCHAR(100) PRIMARY KEY NOT NULL, balance DOUBLE NOT NULL, "
            "description TEXT)";
        if (!exec_sql(accounts_sql, "create accounts table")) {
            return false;
        }

        // trades table
        const ::QString trades_sql = "CREATE TABLE IF NOT EXISTS trades "
            "(id INTEGER PRIMARY KEY AUTOINCREMENT, account VARCHAR(100) NOT NULL, "
            "amount DOUBLE NOT NULL, time DATETIME NOT NULL, brief TEXT, description TEXT, "
            "insert_time DATETIME DEFAULT (datetime('now', 'localtime')), "
            "FOREIGN KEY (account) REFERENCES accounts (name) )";
        if (!exec_sql(trades_sql, "create trades table")) {
            return false;
        }

        // enable foreign key
        const ::QString foreign_sql = "PRAGMA foreign_keys = ON";
        if (!exec_sql(foreign_sql, "enable foreign key")) {
            return false;
        }
    }

    return true;
}

bool MainWindow::refresh_data() {
    assert(_db.isValid());

    const ::QString sql = "SELECT id, account, amount, time, brief FROM trades";
    auto query = _db.exec(sql);
    if (!query.isActive()) {
        qDebug() << "Fail to exec sql:<" << sql << ">, error:" << query.lastError();
        return false;
    }

    std::vector<std::tuple<::QString, ::QString, ::QString, ::QString, ::QString>> results;
    while (query.next()) {
        results.emplace_back(query.value(0).toString(),
                            query.value(1).toString(),
                            query.value(2).toString(),
                            query.value(3).toString(),
                            query.value(4).toString());
    }

    auto trade_table = _ui->trade_table;
    trade_table->clear();
    trade_table->setColumnCount(5);
    {
        trade_table->setHorizontalHeaderItem(0, new ::QTableWidgetItem("ID"));
        trade_table->setHorizontalHeaderItem(1, new ::QTableWidgetItem("Account"));
        trade_table->setHorizontalHeaderItem(2, new ::QTableWidgetItem("Amount"));
        trade_table->setHorizontalHeaderItem(3, new ::QTableWidgetItem("Time"));
        trade_table->setHorizontalHeaderItem(4, new ::QTableWidgetItem("Brief"));
    }
    trade_table->setRowCount(results.size());

    for (std::size_t i = 0; i < results.size(); ++i) {
        trade_table->setItem(i, 0, new ::QTableWidgetItem(std::move(std::get<0>(results[i]))));
        trade_table->setItem(i, 1, new ::QTableWidgetItem(std::move(std::get<1>(results[i]))));
        trade_table->setItem(i, 2, new ::QTableWidgetItem(std::move(std::get<2>(results[i]))));
        trade_table->setItem(i, 3, new ::QTableWidgetItem(std::move(std::get<3>(results[i]))));
        trade_table->setItem(i, 4, new ::QTableWidgetItem(std::move(std::get<4>(results[i]))));
    }

    return true;
}

} // namespace self
