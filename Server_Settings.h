#ifndef SERVER_SETTINGS_H
#define SERVER_SETTINGS_H
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QLayout>
#include <QMessageBox>
#include <QtNetwork/QTcpServer>
#include <QMainWindow>


class Server_Settings : public QDialog {
    Q_OBJECT

public:
    Server_Settings(QWidget *parent=nullptr);
    ~Server_Settings();
    QPushButton *accept;
    QLineEdit *ip;
    QLineEdit *port;


private:

    QLabel *ipLabel;
    QLabel *portLabel;
    QHBoxLayout *ipHorizontalLayout;
    QHBoxLayout *portHorizontalLayout;
    QVBoxLayout *verticalLayout;

};
#endif // SERVER_SETTINGS_H
