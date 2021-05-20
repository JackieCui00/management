#include <QMainWindow>
#include <QSqlDatabase>

namespace Ui {
class MainWindow;
};

namespace self {

class MainWindow final : public ::QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(::QWidget* parent = nullptr);
    ~MainWindow();

    bool start();
    void stop();

private:
    void on_add_trade();

private:
    bool setup_db();
    bool refresh_data();

private:
    ::Ui::MainWindow* _ui = nullptr;

    QSqlDatabase _db = ::QSqlDatabase::database();
};

} // namespace self
