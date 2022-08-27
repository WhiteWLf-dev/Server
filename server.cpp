#include "server.h"

Server::Server() {
    serverState = new QLabel;
    logs=new QTextEdit();
    logs->setReadOnly(true);
    logs->acceptRichText();
    fileMenu=new QMenu(tr("Файл"));
    settingsMenu= new QMenu(tr("Настройки"));
    networkAction=new QAction(tr("Сеть"));
    startAction=new QAction(tr("Включить"));
    stopAction=new QAction(tr("Выключить"));
    exitAction=new QAction(tr("Выйти"));
    saveLogsAction=new QAction(tr("Сохранить журнал"));
    quitButton = new QPushButton(tr("Quit"));

    doc=new QDomDocument("logs");
    domElement=doc->createElement("logs");
    doc->appendChild(domElement);

    fileMenu->addAction(startAction);
    fileMenu->addAction(stopAction);
    fileMenu->addAction(saveLogsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    settingsMenu->addAction(networkAction);
    QMenuBar *menu=new QMenuBar(this);
    menu->addMenu(fileMenu);
    menu->addMenu(settingsMenu);
    sDialog=new ServerDialog(this);

    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(on_starting()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(on_stopping()));
    connect(saveLogsAction, SIGNAL(triggered()), this, SLOT(on_saveLogs()));
    connect(networkAction, SIGNAL(triggered()), this, SLOT(on_network()));
    connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    QHBoxLayout *mainLayout=new QHBoxLayout;
    QVBoxLayout *tabLayout=new QVBoxLayout;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(serverState);
    layout->addWidget(logs);
    layout->addWidget(quitButton);
    mainLayout->addLayout(layout);
    mainLayout->addLayout(tabLayout);
    this->setLayout(mainLayout);

    setWindowTitle("Сервер");

//    testButton=new QPushButton("Тест");
//    testButton->setIcon(QIcon(QPixmap::fromImage(QImage("/home/user/Desktop/photo.jpg"))));
//    mainLayout->addWidget(testButton);
    messageLength = 0;
}

void Server::on_saveLogs() {
    saveXMLLogs();
}

void Server::on_starting() {
    server = new QTcpServer(this);
    if(!server->listen(ip, port)) {
        serverState->setText(tr("Сервер не смог запуститься: ") + server->errorString());
        logs->append(tr("Сервер не смог запуститься: ")+server->errorString());
    } else {
        setWindowTitle(ip.toString()+":"+QString::number(port)+" 0");
        serverState->setText(tr("Сервер запущен на порту ") + QString::number(server->serverPort()));
        logs->append(tr("Сервер запущен!"));
        connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }
}

void Server::on_stopping() {
    logs->append(tr("Сервер был остановлен!"));
    for(auto user : users) {
        user->disconnectFromHost();
        usersInfo.remove(user);
        users.removeOne(user);
    }
//    users.clear();
    server->disconnect();
    server->close();
    qDebug()<<users;
    qDebug()<<usersInfo;
    setWindowTitle("Сервер");
}

void Server::on_network() {
    sDialog->ipLineEdit->setText(ip.toString());
    sDialog->portLineEdit->setText(QString::number(port));
    sDialog->open();
}

void Server::on_accept() {
    if (sDialog->ipLineEdit->text().isEmpty()||sDialog->portLineEdit->text().isEmpty()) {
        QMessageBox::information(sDialog, tr("Пустые поля"), tr("Введите запрос."));
        return;
    }
    ip=QHostAddress(sDialog->ipLineEdit->text());
    port=sDialog->portLineEdit->text().toUInt();
    QMessageBox::information(sDialog, tr("Успех!"), tr("Вы успешно поменяли настройки сервера!"));
    sDialog->hide();
}

void Server::userListUpdated(){
    QString names;
    QByteArrayList images_64;
    QStringList ips;
    QStringList times;
    QStringList statuses;
    for(std::size_t i=0; i<usersInfo.count(); ++i) {
        names.append(usersInfo.value(users[i])->username);
        names.append(",");
        images_64<<usersInfo.value(users[i])->icon_64;
        ips<<usersInfo.value(users[i])->ip;
        statuses<<usersInfo.value(users[i])->status;
        times<<usersInfo.value(users[i])->connected_time.toString();
    }
//    qDebug()<<"Список пользователей онлайн: "<<names;
//    qDebug()<<"Список их картинок: "<<images_64;
    quint16 amount=images_64.count();
    for(std::size_t i=0; i<usersInfo.count(); ++i) {
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_1);
            out<<(quint16)0<<(QString)"USERLIST_UPDATED"<<names<<images_64<<ips<<times<<statuses;
//            qDebug()<<"IMAGES"<<images_64;
            users[i]->write(block);
            users[i]->flush();
    }
}

void Server::newConnection() {

    QTcpSocket* newUser = server->nextPendingConnection();
    users << newUser;
    User *user=new User();
    usersInfo.insert(newUser, user);

    connect(newUser, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(newUser, SIGNAL(disconnected()), this, SLOT(logoutUser()));

    QString message="Пользователь подключился!";
    setWindowTitle(ip.toString()+":"+QString::number(port)+" "+QString::number(users.count()));
    QDomElement newLog=log(doc, QDateTime::currentDateTime().toString(), newUser->peerAddress().toString(), "SERVER", "serverMessage", message);
    domElement.appendChild(newLog);

    sendToAll("<em>Пользователь подключился!</em>");
}

void Server::dataReceived() {
    int mainData=0;
    //packet received
    //Whose packet is it?

    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(socket == 0) { //can't find sender
        return;
    }
    //otherwise lets continue
    //Getting message
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_1);
//    in.setVersion(QDataStream::Qt_5_5);
    if(messageLength == 0) {
        if(socket->bytesAvailable() < (int)sizeof(quint16)) {
            qDebug()<<"<intsizeofquint16";
            return;
        }
        in >> messageLength;
    }
    //did we get all the bytes?
    if(socket->bytesAvailable() < messageLength) {
        qDebug()<<"do not get all the bytes";
        return;  //no
    }
    //yes
    qDebug()<<"control F: "<<messageLength;
    if(messageLength==0) {
        //TODO Картинку сохранять и добавлять в логи.
        //Файлу генерировать стремное название и сохранять в Downloads
        //-------SEND PHOTO
        QString test;
        QString username;
        QByteArray icon;
        QString name;
        QString status;
        QByteArray icon_64;
        in>>test;
        if(test=="ICON") {
            in>>username;
            in>>icon;
        } else if(test=="USERLIST_UPDATE") {
            in>>name;
            in>>icon_64;
            in>>status;
            qDebug()<<"Получены данные пользователя: "<<name;
            qDebug()<<icon_64;
            qDebug()<<"Статус пользователя "<<status;
        }
        qDebug()<<test;
        qDebug()<<icon;
        if(test=="ICON") {
            QImage image=QImage::fromData(QByteArray::fromBase64(icon),"PNG");
            //QLabel *label=new QLabel(this);
            //label->setPixmap(QPixmap::fromImage(image));
            //label->show();
            testButton->setIcon(QIcon(QPixmap::fromImage(image)));
            usersInfo.value(socket)->username=username;
            usersInfo.value(socket)->set_icon(image);
            usersInfo.value(socket)->icon_64=icon;
            messageLength=0;

            QBuffer buffer;
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_1);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");
            QString encoded=buffer.data().toBase64();
            out<<(quint16)0<<(QString)"UPDATED_USER_ICON"<<username<<icon;//encoded;

            for(int i = 0; i < users.size(); ++i) {
                if(users[i]!=socket) {
                    users[i]->write(block);
                    users[i]->flush();
                }
            }

            qDebug()<<"icon!";
            mainData=1;
            messageLength=0;
        } else if(test=="FILESENDEDS") {
            QByteArray block;
            QString filename;
            QString username;
            in>>username;
            in>>filename;
            in>>block;
            qDebug()<<username<<filename;
            qDebug()<<block;
            for(std::size_t i=0; i<users.count(); ++i) {
                if(usersInfo.value(users[i])->username==username) {
                    QByteArray packet;
                    QDataStream out(&packet, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_1);
                    out<<(quint16)0<<(QString)"FILEFORYOU"<<filename<<block<<usersInfo.value(socket)->username;
                    users[i]->write(packet);
                    qDebug()<<"FLUSH!";
                    users[i]->flush();
                }
            }
            QByteArray packetForFirstClient;
            QDataStream out(&packetForFirstClient, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_1);
            out<<(quint16)0<<(QString)"FILESENDEDC";
            socket->write(packetForFirstClient);
            socket->flush();
            messageLength=0;
            mainData=1;
//            if(QFileInfo("/home/user/DownloadsFromChat/"+filename+".txt").exists()) {
//                int i=1;
//                bool numberFound=false;
//                while(numberFound==false) {
//                    QString filename_copy=filename;
//                    filename_copy.append(QString::number(i)+".txt");
//                    if(!QFileInfo("/home/user/DownloadsFromChat/"+filename_copy).exists()) {
//                        numberFound=true;
//                        filename=filename_copy;
//                        break;
//                    }
//                    ++i;
//                }
//                QFile file("/home/user/DownloadsFromChat/"+filename);
//                if(file.open(QIODevice::ReadWrite)) {
//                    QTextStream stream(&file);
//                    stream<<block;
//                    file.close();

//                    QByteArray blockForUser;
//                    QDataStream out(&blockForUser, QIODevice::WriteOnly);
//                    out.setVersion(QDataStream::Qt_4_1);
//                    out<<(quint16)0<<(QString)"FILESENDEDC";
//                    socket->write(blockForUser);
//                    messageLength=0;
//                    mainData=1;
//                }
//            } else {
//                filename.append(".txt");
//                QFile file("/home/user/DownloadsFromChat/"+filename);
//                if(file.open(QIODevice::ReadWrite)) {
//                    QTextStream stream(&file);
//                    stream<<block;
//                    file.close();

//                    QByteArray blockForUser;
//                    QDataStream out(&blockForUser, QIODevice::WriteOnly);
//                    out.setVersion(QDataStream::Qt_4_1);
//                    out<<(quint16)0<<(QString)"FILESENDEDC";
//                    socket->write(blockForUser);
//                    messageLength=0;
//                    mainData=1;
//                }
//            }
        } else if(test=="FILEGETACCEPT") {
            QString usernameGet=usersInfo.value(socket)->username;
            QString usernameSend=filesList.value(usernameGet)[0];
            QString filename=filesList.value(usernameGet)[1];
            quint16 size=filesList.value(usernameGet)[2].toUInt();
            QByteArray fileBlock=QByteArray::fromBase64(filesList.value(usernameGet)[3].toUtf8());
            qDebug()<<"FILEBLOCK"<<fileBlock;

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_1);
            out<<(quint16)0<<(QString)"FILEFORCLIENT"<<usernameSend<<filename<<size<<fileBlock;
            qDebug()<<block;
            socket->write(block);
//            socket->flush();

            QByteArray blockForFirstClient;
            QDataStream out1(&blockForFirstClient, QIODevice::WriteOnly);
            out1.setVersion(QDataStream::Qt_4_1);
            out1<<(quint16)0<<(QString)"CLIENTACCEPTFILE";
            for(std::size_t i=0; i<usersInfo.count(); ++i) {
                if(usersInfo.value(users[i])->username==usernameSend) {
                    users[i]->write(blockForFirstClient);
                    qDebug()<<"SENDER BLOCK"<<blockForFirstClient;
                    users[i]->flush();
                }
            }
            filesList.remove(usernameGet);
            messageLength=0;
            mainData=1;
            return;
        } else if(test=="FILEGETDECLINE") {
            QString usernameGet=usersInfo.value(socket)->username;
            QString usernameSend=filesList.value(usernameGet)[0];

            QByteArray blockForFirstClient;
            QDataStream out1(&blockForFirstClient, QIODevice::WriteOnly);
            out1.setVersion(QDataStream::Qt_4_1);
            out1<<(quint16)0<<(QString)"CLIENTDECLINEFILE";
            for(std::size_t i=0; i<usersInfo.count(); ++i) {
                if(usersInfo.value(users[i])->username==usernameSend) {
                    users[i]->write(blockForFirstClient);
                    users[i]->flush();
                }
            }
            filesList.remove(usernameGet);
            messageLength=0;
            mainData=1;
//            socket->flush();
        } else if(test=="FILEFORSERVER") {
            QString username;
            QString filename;
            quint16 size;
            QByteArray block;
            in>>username;
            in>>filename;
            in>>size;
            in>>block;
            qDebug()<<"FIRSTBLOCK"<<block;

            QByteArray packetForFirstClient;
            QDataStream out(&packetForFirstClient, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_1);
            out<<(quint16)0<<(QString)"SERVERGOTFILE";
//            socket->write(packetForFirstClient);
//            socket->flush();

            QList<QString> list;
            list.append(usersInfo.value(socket)->username);
            list.append(filename);
            list.append(QString::number(size));
            list.append(QString::fromUtf8(block.toBase64()));
            filesList.insert(username, list);
            qDebug()<<"LIST"<<list;
            for(std::size_t i=0; i<users.count(); ++i) {
                if(usersInfo.value(users[i])->username==username) {
//                    QByteArray secondPacketForFirstClient;
//                    QDataStream out2(&secondPacketForFirstClient, QIODevice::WriteOnly);
//                    out2.setVersion(QDataStream::Qt_4_1);
//                    out2<<(quint16)0<<(QString)"CLIENTASKEDFORFILE";
//                    socket->write(secondPacketForFirstClient);
//                    socket->flush();

                    QByteArray packet;
                    QDataStream out(&packet, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_1);
                    //DELETED "<<block<<
                    out<<(quint16)0<<(QString)"ASKFORFILEGET"<<filename<<usersInfo.value(socket)->username;
                    users[i]->write(packet);
                    users[i]->flush();
                }
            }

            messageLength=0;
            mainData=1;
            return;

        } else if(test=="USERLIST_UPDATE") {
            User *newUser=new User();
            newUser->username=name;
            newUser->icon_64=icon_64;
            newUser->status=status;
            newUser->ip=socket->peerAddress().toString();
            newUser->connected_time=QDateTime::currentDateTime();
            usersInfo.insert(socket, newUser);
            userListUpdated();
            qDebug()<<"NAME"<<name;
            messageLength=0;
            mainData=1;
        } else if(test=="USER_UPDATED") {
            User *newUser=new User();
            QString username;
            QByteArray icon_64;
            QString status;
            in>>username;
            in>>icon_64;
            in>>status;
            QDateTime oldTime;
            if(usersInfo.contains(socket)) {
                oldTime=usersInfo.take(socket)->connected_time;
            }
            newUser->username=username;
            newUser->icon_64=icon_64;
            newUser->status=status;
            newUser->connected_time=oldTime;
            newUser->ip=socket->peerAddress().toString();
            usersInfo.insert(socket, newUser);
            userListUpdated();
            messageLength=0;
            mainData=1;
        } else if(test=="PHOTO") {
            QByteArray image_64;
            QString author;
            in>>author;
            in>>image_64;

            /*QImage image=QImage::fromData(QByteArray::fromBase64(image_64), "PNG");
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");
            QString encoded=buffer.data().toBase64();
            QDomElement newLog=log(doc, QDateTime::currentDateTime().toString(), socket->peerAddress().toString(), author, "imageMessage", encoded);
            domElement.appendChild(newLog);*/

            QString ipString=usersInfo.value(socket)->ip;
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_1);
            out<<(quint16)0<<(QString)"PHOTO"<<author<<ipString<<image_64;
            for(std::size_t i=0; i<users.count(); ++i) {
                users[i]->write(block);
                users[i]->flush();
            }
            messageLength=0;
            mainData=1;
        } else if(test=="FORMATTED_MESSAGE") {
            QString fontName;
            int fontWeight;
            QString colorStr;
            QString username;
            QString message;
            in>>username;
            in>>colorStr;
            in>>fontName;
            in>>fontWeight;
            in>>message;
            QByteArray packet;
            QDataStream out(&packet, QIODevice::WriteOnly);

            QString messageNew="("+colorStr+","+fontName+","+fontWeight+") "+message;
            QDomElement newLog=log(doc, QDateTime::currentDateTime().toString(), socket->peerAddress().toString(), username, "formattedMessage", messageNew);
            domElement.appendChild(newLog);

            out<<(quint16)0<<(QString)"FORMATTED_MESSAGE"<<username<<colorStr<<fontName<<fontWeight<<(QString)socket->peerAddress().toString()<<message;
            for(std::size_t i=0; i<users.count(); ++i) {
                users[i]->write(packet);
                users[i]->flush();
            }
            messageLength=0;
            mainData=1;
        } else if(test=="FILE") {
            //Сохраняем файл с прикольным названием
        } else if(test=="USERNAME_CHANGE") {
            QString oldUsername;
            QString newUsername;
            in>>oldUsername;
            in>>newUsername;
            for(auto user : usersInfo) {
                if(user->username==oldUsername) {
                    QTcpSocket *savedSocket=usersInfo.key(user);
                    QByteArray savedImage=usersInfo.value(savedSocket)->icon_64;
                    User *savedUser=new User();
                    savedUser->username=newUsername;
                    savedUser->icon_64=savedImage;
                    usersInfo.remove(usersInfo.key(user));
                    usersInfo.insert(savedSocket, savedUser);
                }
            }
            userListUpdated();
            mainData=1;
            messageLength=0;
        } else if(test=="DISCONNECT") {
            QString username;
            in>>username;
//            usersInfo.remove(socket);
//            users.removeOne(socket);
//            userListUpdated();
        } else {
            qDebug()<<"Clown";
        }
    }
    QString message;
    in >> message;

    logs->append(message);



    if(message.contains("[USERNAME]")) {
        int needPos=message.indexOf("]");
        int needPos2=message.indexOf(",");
        int needPos21=message.indexOf("]", needPos2);
        int needPos3=message.indexOf(",", needPos2+1);
        int needPos4=message.indexOf("]", needPos3);
        int needPos41=message.indexOf(",", needPos4);
        int needPos5=message.indexOf("]", needPos41);
//        int needPos51=message.indexOf()
        QString name=message.mid(needPos+2, needPos2-needPos-2);
        QString status=message.mid(needPos21+1, needPos3-needPos21-1/*message.count()-1*/);
        QString autoFile=message.mid(needPos4+1, needPos41-needPos4-1);
//        QString icon_bits=message.mid(needPos5+1, message.count()-needPos5-1);
        QByteArray icon_bits=message.mid(needPos5+1, message.count()-needPos5-1).toUtf8().toBase64();
        QPixmap image;
        image.loadFromData(QByteArray::fromBase64(icon_bits));
        QIcon icon;
        if(usersInfo.contains(socket)) {
            for(auto user : users) {
                if(socket==user) {
                    User *newUser=new User();
                    newUser->set_username(name);
                    newUser->status=status;
                    if(autoFile=="yes") {
                        newUser->autoFileGet=true;
                    } else if(autoFile=="no") {
                        newUser->autoFileGet=true;
                    }
                    logs->append("Новый пользователь: "+newUser->get_username()+" со статусом "+newUser->status+" с AFG "+autoFile);
                    QImage img=QImage::fromData(icon_bits);
                    testButton->setIcon(QIcon(QPixmap::fromImage(img)));
                }
            }
            messageLength = 0;
        } else {
            logs->append("Пользователь "+usersInfo.value(socket)->get_username()+" сменил имя на "+name+" А статус c "+usersInfo.value(socket)->status+" на "+status+" А AFG на "+autoFile);
            usersInfo.value(socket)->set_username(name);
            //TODO ТУТ ПРИХОДИТ ТОЛЬКО ТЕГ [USERNAME]: либо отправляем остальные теги, либо обрабатываем здесь их по отдельности
            //            usersInfo.value(socket)->status=status;
//            if(autoFile=="yes") {
//                usersInfo.value(socket)->autoFileGet=true;
//            } else if(autoFile=="no") {
//                usersInfo.value(socket)->autoFileGet=false;
//            }
            messageLength=0;
            return;
        }
    } /*else if(message.contains("[ICON]")) {
        int needPos=message.indexOf("]");
        //ICON

        //аналог sendToAll() но со списком пользователей, чтобы выгрузить оттуда статус, иконку и имя
    }*/ /*else if(message.contains("[STATUS]")) {
        int needPos=message.indexOf("]");
        QString status=message.mid(needPos+2, message.count()-1);
        User *user=usersInfo.value(socket);
        user->status=status;
        logs->append("Статус "+user->get_username()+" изменен на: "+user->status);
        messageLength=0;
//        return;
    }*/ else {
//        QString name=message.left(message.indexOf(":"));
        QString name=usersInfo.value(socket)->username;
        //TODO при чтении xml подставлять вместо непонятных символов <
        QDomElement newLog=log(doc, QDateTime::currentDateTime().toString(), socket->peerAddress().toString(), name, "simpleMessage", message.mid(message.indexOf(">", 8)+1, message.count()));
        domElement.appendChild(newLog);

        message.prepend("["+socket->peerAddress().toString()+"] ");
        if(mainData!=1)
            sendToAll(message);
        messageLength = 0;
     }
}

void Server::logoutUser() {
//    sendToAll(tr("<em>Пользователь отключился.</em>"));
    //which client wants to leave
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(socket == 0) {
        return; //i dunno m8, let's stop there
    }

    QString message="Пользователь отключился.";
    QDomElement newLog=log(doc, QDateTime::currentDateTime().toString(), socket->peerAddress().toString(), "SERVER", "serverMessage", message);
    domElement.appendChild(newLog);

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_1);
    out<<(quint16)0<<(QString)"DISCONNECTEDUSER"<<usersInfo.value(socket)->username;
    qDebug()<<"USERS"<<users;
    qDebug()<<"USERSINFO"<<usersInfo;
    for(std::size_t i=0; i<users.count(); ++i) {
        if(users[i]!=socket) {
//            users[i]->write(packet);
        }
    }
    users.removeOne(socket);
    usersInfo.remove(socket);
    setWindowTitle(ip.toString()+":"+QString::number(port)+" "+QString::number(users.count()));
    sendToAll(tr("<em>Пользователь отключился.</em>"));

    socket->deleteLater(); //makes qt not lag; once the slot is done executing, will remove socket
}

void Server::sendToAll(const QString &message) {
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);

    out<<(quint16)0<<(QString)"SIMPLE_MESSAGE"<<message;
    logs->append(message);
    for(std::size_t i=0; i<users.size(); ++i) {
        users[i]->write(packet);
        users[i]->flush();
    }
//    out << (quint16) 0;
//    out << message;
//    qDebug()<<"Sent to all "<<message;
//    out.device()->seek(0);
//    out << (quint16) (packet.size() - sizeof(quint16));
//    logs->append(message);
//    for(int i = 0; i < users.size(); ++i) {
//        users[i]->write(packet);
//    }
}

QDomElement Server::makeElement(QDomDocument *domDoc, const QString &strName, const QString &strAttr=QString::null, const QString &Text=QString::null) {
    QDomElement domElement=domDoc->createElement(strName);
    if(!strAttr.isEmpty()) {
        QDomAttr domAttr=domDoc->createAttribute("number");
        domAttr.setValue(strAttr);
        domElement.setAttributeNode(domAttr);
    }
    if(!Text.isEmpty()) {
        QDomText domText=domDoc->createTextNode(Text);
        domElement.appendChild(domText);
    }
    return domElement;
}


QDomElement Server::log(QDomDocument *domDoc, const QString &dateTime, const QString &ip, const QString &name, const QString &type, const QString &message) {
    static int n=1;
    QDomElement domElement=makeElement(domDoc, "log", QString().setNum(n));
    domElement.appendChild(makeElement(domDoc, "name", "", name));
    domElement.appendChild(makeElement(domDoc, "ip", "", ip));
    domElement.appendChild(makeElement(domDoc, "dateAndTime", "",dateTime));
    if(type=="simpleMessage") {
        domElement.appendChild(makeElement(domDoc, "message", "", message));
    } else if(type=="formattedMessage") {
        domElement.appendChild(makeElement(domDoc, "formattedMessage", "", message));
    } else if(type=="imageMessage") {
        domElement.appendChild(makeElement(domDoc, "imageMessage", "", message));
    } else if(type=="fileMessage") {
        domElement.appendChild(makeElement(domDoc, "fileMessage", "", message));
    } else if(type=="serverMessage") {
        domElement.appendChild(makeElement(domDoc, "serverMessage", "", message));
    }
    ++n;
    return domElement;
}

void Server::saveXMLLogs() {
    QFile file("logs.xml");
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream(&file)<<doc->toString();
        file.close();
    }
}
