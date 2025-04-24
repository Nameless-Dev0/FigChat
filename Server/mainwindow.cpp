#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto path = getIconPath();
    qDebug() << path;

    QIcon windowIcon(path);
    setWindowIcon(windowIcon);

    applyCustomPalette();

    setupServer();

    QMessageBox server_ip_pop_up(this);
    server_ip_pop_up.setWindowTitle("Server IP Address");
    server_ip_pop_up.setText(ServerManager::get_ip().toString());
    server_ip_pop_up.resize(200, 100);
    server_ip_pop_up.exec();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newClientConnected(QTcpSocket *client)
{
    auto id = client->property("id").toInt();
    ui->lstClients->addItem(QString("New Client added: %1").arg(id));
    auto chatWidget= new ClientChatWidget(client, ui->tbClientsChat);
    ui->tbClientsChat->addTab(chatWidget, QString("Client (%1)").arg(id));

    connect(chatWidget, &ClientChatWidget::clientNameChanged, this, &MainWindow::setClientName);
    connect(chatWidget, &ClientChatWidget::statusChanged, this, &MainWindow::setClientStatus);
    connect(chatWidget, &ClientChatWidget::isTyping, [this](QString name){
        this->statusBar()->showMessage(name, 750);
    });

        connect(chatWidget, &ClientChatWidget::textForOtherClients, _server, &ServerManager::onTextForOtherClients);
}

void MainWindow::clientDisconnected(QTcpSocket *client)
{
    auto id = client->property("id").toInt();
    ui->lstClients->addItem(QString("Client disconnected: %1").arg(id));
}

void MainWindow::setClientName(QString prevName, QString name)
{
    auto widget = qobject_cast<QWidget *>(sender());
    auto index = ui->tbClientsChat->indexOf(widget);
    ui->tbClientsChat->setTabText(index, name);

    _server->notifyOtherClients(prevName, name);
}

void MainWindow::setClientStatus(ChatProtocol::Status status)
{
    auto widget = qobject_cast<QWidget *>(sender());
    auto index = ui->tbClientsChat->indexOf(widget);
    QString iconName = ":/icons/";
    switch (status) {
    case ChatProtocol::Available:
        iconName.append("available.png");
        break;
    case ChatProtocol::Away:
        iconName.append("away.png");
        break;
    case ChatProtocol::Busy:
        iconName.append("busy.png");
        break;
    default:
        iconName = "";
        break;

    }

    auto icon = QIcon(iconName);
    ui->tbClientsChat->setTabIcon(index, icon);
}

void MainWindow::setupServer()
{
    _server = new ServerManager();
    connect(_server, &ServerManager::newClientConnected, this, &MainWindow::newClientConnected);
    connect(_server, &ServerManager::clientDisconnected, this, &MainWindow::clientDisconnected);

}

QString MainWindow::getIconPath()
{
    QString path = QCoreApplication::applicationDirPath();
    QDir dir(path);

    // Go back 2 directories
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
    dir.cd("icons");

    QString newPath = dir.absolutePath();
    QString filePath = dir.filePath("fig.png");
    return filePath;
}


void MainWindow::on_tbClientsChat_tabCloseRequested(int index)
{
    auto chatWidget = qobject_cast<ClientChatWidget *>(ui->tbClientsChat->widget(index));
    chatWidget->disconnect();
    ui->tbClientsChat->removeTab(index);
}

void MainWindow::applyCustomPalette()
{
    QPalette palette;

    // Main background - soft light beige
    QColor bgColor(183, 183, 164);
    QColor accentColor(221, 190, 169);
    QColor darkTextColor(107, 112, 92);
    QColor inputColor(107, 112, 92);
    QColor highlightColor(130, 90, 60);

    palette.setColor(QPalette::Window, bgColor);
    palette.setColor(QPalette::Base, inputColor);
    palette.setColor(QPalette::Text, bgColor);
    palette.setColor(QPalette::Button, accentColor);
    palette.setColor(QPalette::ButtonText, darkTextColor);
    palette.setColor(QPalette::Highlight, highlightColor);
    palette.setColor(QPalette::WindowText, darkTextColor);

    this->setPalette(palette);
    this->setFont(QFont("Segoe UI", 9));

    ui->pushButton->setStyleSheet(R"(
        QPushButton {
            background-color: #E8D8C4;
            color: #3C281E;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #D6C4B0;
        }
        QPushButton:pressed {
            background-color: #CBB8A3;
        }
    )");

    ui->tbClientsChat->setStyleSheet(R"(
        QTabWidget {
            background-color: #B7B7A4;
            color: #3C281E;
            border-radius: 6px;
            padding: 6px 12px;
        }

        QTabBar::tab {
            background-color: #E8D8C4;
            color: #3C281E;
            padding: 6px 12px;
            border-radius: 6px;
        }

        QTabBar::tab:selected {
            background-color: #E8D8C4;
            color: #3C281E;
        }

        QTabBar::tab:hover {
            background-color: #D6C4B0;
        }
    )");


    this->update();
}

