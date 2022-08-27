#include "serverDialog.h"
#include <QHBoxLayout>

ServerDialog::ServerDialog(QWidget *parent) : QDialog(parent) {
    initializeWidgets();
    //    MainWindow Server(parent);
    //    server=&Server;

    // чтобы редактор оставался активен, пока открыто окно
    setFocusProxy(ipLineEdit);

    // устанавливаем все виджеты и слои
    initializeLayout();
    setWindowTitle(tr("Настройки сервера"));
    connect(acceptButton, SIGNAL(clicked()), parent, SLOT(on_accept()));
}

ServerDialog::~ServerDialog() {
    delete ipLabel;
    delete portLabel;
    delete ipLineEdit;
    delete portLineEdit;
    delete acceptButton;
    delete ipHorizontalLayout;
    delete portHorizontalLayout;
    delete verticalLayout;
}

void ServerDialog::initializeWidgets() {
    ipLabel=new QLabel(tr("IP: "));
    portLabel=new QLabel(tr("Порт: "));
    ipLineEdit=new QLineEdit();
    //    ipLineEdit->setText(server->getIp());
    portLineEdit=new QLineEdit();
    //    portLineEdit->setText(server->getPort());
    acceptButton=new QPushButton(tr("&Применить"));
}

void ServerDialog::initializeLayout() {
    ipHorizontalLayout=new QHBoxLayout();
    portHorizontalLayout=new QHBoxLayout();
    verticalLayout=new QVBoxLayout();
    verticalLayout->addLayout(ipHorizontalLayout);
    verticalLayout->addLayout(portHorizontalLayout);
    verticalLayout->addWidget(acceptButton);
    ipHorizontalLayout->addWidget(ipLabel);
    ipHorizontalLayout->addWidget(ipLineEdit);
    portHorizontalLayout->addWidget(portLabel);
    portHorizontalLayout->addWidget(portLineEdit);
    setLayout(verticalLayout);
}

void ServerDialog::on_acceptButton_clicked() {
    QString ip=ipLineEdit->text();
    QString port=portLineEdit->text();

    if (ip.isEmpty()||port.isEmpty()) {
        QMessageBox::information(this, tr("Пустые поля"), tr("Введите запрос."));
        return;
    }
    //    server->setIp(ip);
    //    server->setPort(port);
    //    server->setTitleIpPort();

}
