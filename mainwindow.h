#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtWidgets>
#include <QMainWindow>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QPlainTextEdit>
#include <QHostAddress>
#include "Server_Settings.h"
#include <QSql>
#include <QDomDocument>
#include "user.h"

class MainWindow : public QWidget {

    Q_OBJECT

    public:
        MainWindow();
        void sendMessage(const QString &message);

    public slots:
        void accept();
    private slots:
        void newConnection();
        void dataReceived();
        void logoutUser();
        void start();
        void stop();
        void on_saveLogs();
        void on_network();

    private:
        void createMenu();
        QPushButton *testButton;
        QMenu *fileMenu;
        QMenu *settingsMenu;
        QAction *networkAction,*startAction,*stopAction,*exitAction,*saveXMLAction;
        QTextEdit *logs;
        QLabel *serverPos;

        quint16 port=45678;
        QHostAddress ip=QHostAddress("127.0.0.1");
        QTcpServer *server;
        QList<QTcpSocket *> users;
        QMap<QTcpSocket *, User *> usersInfo;
        quint16 messageLength;
        Server_Settings *sDialog;
        QByteArray currentByte;
        QDomDocument *doc;
        QDomElement domElement;
        void userListUpdated();
        void saveXMLFile();
        QDomElement log(QDomDocument *domDoc, const QString &dateTime, const QString &ip, const QString &name, const QString &type, const QString &message);
        QDomElement makeElement(QDomDocument *domDoc, const QString &strName, const QString &stdAttr, const QString &Text);
        QMap<QString, QList<QString>> filesList;

};
#endif // MAINWINDOW_H
