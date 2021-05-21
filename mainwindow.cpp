#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_trade_dialog.h"
#include "ui_account_dialog.h"

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

    connect(_ui->actionNewTrade, &::QAction::triggered, this, &MainWindow::on_add_trade);
    connect(_ui->actionAddAccount, &::QAction::triggered, this, &MainWindow::on_add_account);
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

void MainWindow::on_add_trade() {
    ::QDialog dialog(this);
    ::Ui::TradeDialog trade_dialog;
    trade_dialog.setupUi(&dialog);
    trade_dialog.time_input->setDateTime(::QDateTime::currentDateTime());

    { // set up accounts
        const ::QString sql = "SELECT name FROM accounts";
        auto query = _db.exec(sql);
        if (!query.isActive()) {
            qDebug() << "Fail to exec sql:<" << sql << ">, error:" << query.lastError();
            return;
        }
        while (query.next()) {
            trade_dialog.account_input->addItem(query.value(0).toString());
        }
    }

    if (::QDialog::Rejected == dialog.exec()) {
        return;
    }

    const ::QString account = trade_dialog.account_input->currentText();
    const ::QString amount = trade_dialog.amount_input->text();
    const ::QString time = trade_dialog.time_input->text();
    const ::QString brief = trade_dialog.brief_input->text();
    const ::QString description = trade_dialog.description_input->toPlainText();

    qDebug() << "User finished trade_dialog, account:" << account
        << ", amount:" << amount << ", time:" << time << ", brief:" << brief
        << ", description:" << description;

    { // amount must be a number
        bool amount_ok = false;
        (void)amount.toDouble(&amount_ok);
        if (!amount_ok) {
            qDebug() << "Invalid amount:" << amount << ", must be a number";
            return;
        }
    }

    { // insert into db
        const ::QString sql = ::QString("INSERT INTO trades (account, amount, time, brief, description) values ('%1', %2, '%3', '%4', '%5')").arg(account, amount, time, brief, description);
        auto query = _db.exec(sql);
        if (!query.isActive()) {
            qDebug() << "Fail to exec sql:<" << sql << ">, error:" << query.lastError();
            return;
        }
    }

    this->refresh_data();
}

void MainWindow::on_add_account() {
    ::QDialog dialog(this);
    ::Ui::AccountDialog account_dialog;
    account_dialog.setupUi(&dialog);

    if (::QDialog::Rejected == dialog.exec()) {
        return;
    }

    const ::QString account = account_dialog.account_input->text();
    const ::QString balance = account_dialog.balance_input->text();
    const ::QString description = account_dialog.description_input->toPlainText();

    qDebug() << "User finished accmount_dialog, account:" << account
        << ", balance:" << balance << ", description:" << description;

    { // balance must be a number
        bool balance_ok = false;
        (void)balance.toDouble(&balance_ok);
        if (!balance_ok) {
            qDebug() << "Invalid balance:" << balance << ", must be a number";
            return;
        }
    }

    { // insert into db
        const ::QString sql = ::QString("INSERT INTO accounts (name, balance, description) values ('%1', %2, '%3')").arg(account, balance, description);
        auto query = _db.exec(sql);
        if (!query.isActive()) {
            qDebug() << "Fail to exec sql:<" << sql << ">, error:" << query.lastError();
            return;
        }
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
