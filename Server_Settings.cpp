#include "Server_Settings.h"
#include <QHBoxLayout>

Server_Settings::Server_Settings(QWidget *parent) : QDialog(parent) {
    ipLabel=new QLabel(tr("IP: "));
    portLabel=new QLabel(tr("Port: "));
    ip=new QLineEdit();
    //    ipLineEdit->setText(server->getIp());
    port=new QLineEdit();
    //    portLineEdit->setText(server->getPort());
    accept=new QPushButton(tr("&Accept"));
    //    MainWindow Server(parent);
    //    server=&Server;

    // чтобы редактор оставался активен, пока открыто окно
    setFocusProxy(ip);

    // устанавливаем все виджеты и слои
    ipHorizontalLayout=new QHBoxLayout();
    portHorizontalLayout=new QHBoxLayout();
    verticalLayout=new QVBoxLayout();
    verticalLayout->addLayout(ipHorizontalLayout);
    verticalLayout->addLayout(portHorizontalLayout);
    verticalLayout->addWidget(accept);
    ipHorizontalLayout->addWidget(ipLabel);
    ipHorizontalLayout->addWidget(ip);
    portHorizontalLayout->addWidget(portLabel);
    portHorizontalLayout->addWidget(port);
    setLayout(verticalLayout);
    setWindowTitle(tr("Server settings"));
    connect(accept, SIGNAL(clicked()), parent, SLOT(accept()));

}

Server_Settings::~Server_Settings() {
    delete ipLabel;
    delete portLabel;
    delete ip;
    delete port;
    delete accept;
    delete ipHorizontalLayout;
    delete portHorizontalLayout;
    delete verticalLayout;
}



//void Server_Settings::on_acceptButton_clicked() {
//    QString ips=ip->text();
//    QString ports=port->text();

//    if (ips.isEmpty()||ports.isEmpty()) {
//        QMessageBox::information(this, tr("Empty line"), tr("Repeat your action"));
//        return;
//    }
//    //    server->setIp(ip);
//    //    server->setPort(port);
//    //    server->setTitleIpPort();

//}
