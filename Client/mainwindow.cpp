#include "ChatItemWidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <qDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    applyCustomPalette();

    auto path = getIconPath();
    qDebug() << path;

    QIcon windowIcon(path);
    setWindowIcon(windowIcon);

    setupClient();
    setup_db();
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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup_db()
{
    auto db_path  = QCoreApplication::applicationDirPath() + "/local_chat_history_DB.db";
    local_chat_history_DB = QSqlDatabase::addDatabase("QSQLITE");
    local_chat_history_DB.setDatabaseName(db_path);

    if(local_chat_history_DB.open())
        qDebug() << "Connection Established.";
    else
        qDebug() << "Connection to Database failed. Error: " << local_chat_history_DB.lastError() << "path: " << QCoreApplication::applicationDirPath();

    QSqlQuery query;
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            receiver TEXT NOT NULL,
            message TEXT NOT NULL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (query.exec(createTableQuery)) {
        qDebug() << "Table 'messages' created or already exists.";
    } else {
        qDebug() << "Failed to create table: " << query.lastError().text();
    }
}

void MainWindow::setupClient()
{
    QString ip = ui->l_server_ip->text().trimmed();
    _client = new ClientManager(QHostAddress(ip),4500);

    ui->centralwidget->setEnabled(true);

    connect(_client, &ClientManager::connected, [this]() {ui->centralwidget->setEnabled(true);});
    connect(_client, &ClientManager::disconnected, [this]() {ui->centralwidget->setEnabled(false);});


    // Only enable the chat panels when connected
    connect(_client, &ClientManager::connected, [this]() {ui->centralwidget->setEnabled(true);});
    connect(_client, &ClientManager::disconnected, [this]() {ui->centralwidget->setEnabled(false);});

    connect(_client, &ClientManager::textMessageReceived, this, &MainWindow::dataReceived);
    connect(_client, &ClientManager::isTyping, this, &MainWindow::onTyping);
    connect(_client, &ClientManager::initReceivingFile, this, &MainWindow::onInitReceivingFile);
    connect(_client, &ClientManager::rejectReceivingFile, this, &MainWindow::onRejectReceivingFile);

    connect(ui->lnMessage, &QLineEdit::textChanged, _client, &ClientManager::sendIsTyping);
    connect(_client, &ClientManager::connectionACK, this, &MainWindow::onConnectionACK);
    connect(_client, &ClientManager::newClientConnectedToServer, this, &MainWindow::onNewClientConnectedToServer);
    connect(_client, &ClientManager::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(_client, &ClientManager::clientNameChanged, this, &MainWindow::onClientNameChanged);

}

void MainWindow::on_actionConnect_triggered()
{

    QString ip = ui->l_server_ip->text().trimmed();
    if (ip.isEmpty()) {
        QMessageBox::warning(this, "Connect", "Please enter a valid Server IP");
        return;
    }

    if (_client) {
        delete _client;
        _client = nullptr;
    }
    _client = new ClientManager(QHostAddress(ip), 4500, this);

    connect(_client, &ClientManager::connected, [this]() {ui->centralwidget->setEnabled(true);});
    connect(_client, &ClientManager::disconnected, [this]() {ui->centralwidget->setEnabled(false);});


    // Only enable the chat panels when connected
    connect(_client, &ClientManager::connected, [this]() {ui->centralwidget->setEnabled(true);});
    connect(_client, &ClientManager::disconnected, [this]() {ui->centralwidget->setEnabled(false);});

    connect(_client, &ClientManager::textMessageReceived, this, &MainWindow::dataReceived);
    connect(_client, &ClientManager::isTyping, this, &MainWindow::onTyping);
    connect(_client, &ClientManager::initReceivingFile, this, &MainWindow::onInitReceivingFile);
    connect(_client, &ClientManager::rejectReceivingFile, this, &MainWindow::onRejectReceivingFile);

    connect(ui->lnMessage, &QLineEdit::textChanged, _client, &ClientManager::sendIsTyping);
    connect(_client, &ClientManager::connectionACK, this, &MainWindow::onConnectionACK);
    connect(_client, &ClientManager::newClientConnectedToServer, this, &MainWindow::onNewClientConnectedToServer);
    connect(_client, &ClientManager::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(_client, &ClientManager::clientNameChanged, this, &MainWindow::onClientNameChanged);

    _client->connectToServer();
}

void MainWindow::insertMessage(const QString& sender, const QString& receiver, const QString& message)
{
    QSqlQuery query;
    QString insertQuery = R"(
        INSERT INTO messages (sender, receiver, message)
        VALUES (:sender, :receiver, :message)
    )";

    query.prepare(insertQuery);

    query.bindValue(":sender", sender);
    query.bindValue(":receiver", receiver);
    query.bindValue(":message", message);

    if (query.exec()) {
        qDebug() << "Message inserted successfully.";
    } else {
        qDebug() << "Failed to insert message: " << query.lastError().text();
    }
}

void MainWindow::on_btnSend_clicked()
{
    auto message = ui->lnMessage->text().trimmed();
    _client->sendMessage(message, ui->cmbDestination->currentText());
//    ui->lstMessages->addItem(message);
    ui->lnMessage->setText("");
    ui->lnMessage->setFocus();

    auto chatWidget = new ChatItemWidget();
    chatWidget->setMessage(message, true);
    auto listWidgetItem = new QListWidgetItem();
    listWidgetItem->setSizeHint(QSize(0, 65));
    ui->lstMessages->addItem(listWidgetItem);
    ui->lstMessages->setItemWidget(listWidgetItem, chatWidget);

    //database stuff
    QString sender = ui->lnClientName->text();
    if(sender == "")
        sender = "client";

    QString receiver = ui->cmbDestination->currentText();
    insertMessage(sender, receiver, message);
}

void MainWindow::dataReceived(QString message)
{
//    ui->lstMessages->addItem(data);
    auto chatWidget = new ChatItemWidget();
    chatWidget->setMessage(message);
    auto listWidgetItem = new QListWidgetItem();
    listWidgetItem->setSizeHint(QSize(0, 65));
    ui->lstMessages->addItem(listWidgetItem);
    listWidgetItem->setBackground(QColor(167, 255, 237));
    ui->lstMessages->setItemWidget(listWidgetItem, chatWidget);

    QString receiver = ui->lnClientName->text();
    QString sender = ui->cmbDestination->currentText();
    insertMessage(sender, receiver, message);
}


void MainWindow::on_lnClientName_editingFinished()
{
    auto name = ui->lnClientName->text().trimmed();
    _client->sendName(name);
}


void MainWindow::on_cmbStatus_currentIndexChanged(int index)
{
    auto status = (ChatProtocol::Status)index;
    _client->sendStatus(status);
}

void MainWindow::onTyping()
{
    statusBar()->showMessage("Server is typing...", 750);
}


void MainWindow::on_btnSendFile_clicked()
{
    auto fileName = QFileDialog::getOpenFileName(this, "Select a file", "/home");
    _client->sendInitSendingFile(fileName);
}

void MainWindow::onRejectReceivingFile()
{
    QMessageBox::critical(this, "Sending File", "Operation rejected...");
}

void MainWindow::onInitReceivingFile(QString clientName, QString fileName, qint64 fileSize)
{
    auto message = QString("Client (%1) wants to send a file. Do you want to accept it?\nFile Name:%2\nFile Size: %3 bytes")
            .arg(clientName, fileName)
            .arg(fileSize);
    auto result = QMessageBox::question(this, "Receiving File", message);
    if (result == QMessageBox::Yes) {
        _client->sendAcceptFile();
    } else {
        _client->sendRejectFile();
    }

}

void MainWindow::onConnectionACK(QString myName, QStringList clientsName)
{
    ui->cmbDestination->clear();
    clientsName.prepend("All");
    clientsName.prepend("Server");
    foreach (auto client, clientsName) {
        ui->cmbDestination->addItem(client);
    }
    setWindowTitle(myName);
}

void MainWindow::onNewClientConnectedToServer(QString clienName)
{
    ui->cmbDestination->addItem(clienName);
}

void MainWindow::onClientNameChanged(QString prevName, QString clientName)
{
    for (int i = 0; i < ui->cmbDestination->count(); ++i) {
        if (ui->cmbDestination->itemText(i) == prevName) {
            ui->cmbDestination->setItemText(i, clientName);
            return;
        }
    }
}

void MainWindow::onClientDisconnected(QString clientName)
{
    for (int i = 0; i < ui->cmbDestination->count(); ++i) {
        if (ui->cmbDestination->itemText(i) == clientName) {
            ui->cmbDestination->removeItem(i);
            return;
        }
    }
}

void MainWindow::applyCustomPalette()
{
    QPalette palette;

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

    QString labelStyle = "QLabel { color: #3C281E; background-color: transparent; font-size: 13px; }";
    ui->label->setStyleSheet(labelStyle);
    ui->label_2->setStyleSheet(labelStyle);
    ui->label_3->setStyleSheet(labelStyle);

    ui->btnSend->setStyleSheet(R"(
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

    ui->lnMessage->setStyleSheet(R"(
        QLineEdit {
            background-color: #E8D8C4;
            color: #3C281E;
            border: 1px solid #6B705C;
            border-radius: 6px;
            padding: 6px;
        }
    )");

    this->update();
}



