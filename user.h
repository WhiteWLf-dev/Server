#ifndef USER_H
#define USER_H

#include <QByteArray>
#include <QVector>
#include <QString>
#include <QtNetwork/QTcpSocket>
#include <QIcon>
#include <QDateTime>

class User {
public:
    User();
    QString username="User";
    QTcpSocket user_socket;
    QString user_status="Avaible";
    QImage user_icon;
    QString ip="127.0.0.1";
    QByteArray user_icon_64;
    bool autoFileGet=0;
    QDateTime connected_time;

private:
    //    QString username="Test";
    QByteArray user_id;
    QVector<QByteArray> current_users;

public:
    void add_user(QByteArray);
    void remove_user(QByteArray);
    bool find_user(QByteArray);
    void clear_users();
    void set_user_id(QByteArray);
    void set_username(QString);
    void set_user_icon(QImage);
    QByteArray get_user_id();
    QString get_user_name();
};
#endif // USER_H
