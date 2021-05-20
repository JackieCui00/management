#include <QMainWindow>

namespace Ui {
class MainWindow;
};

namespace self {

class MainWindow final : public ::QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(::QWidget* parent = nullptr);
    ~MainWindow();

private:
    ::Ui::MainWindow* _ui = nullptr;
};

} // namespace self
