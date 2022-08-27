#include "mainwindow.h"
#include "QMenuBar"


MainWindow::MainWindow() {
    QPalette darkPalette;

        // Настраиваем палитру для цветовых ролей элементов интерфейса
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::black);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);

        // Устанавливаем данную палитру
        qApp->setPalette(darkPalette);
    serverPos = new QLabel;
    logs=new QTextEdit();
    logs->setReadOnly(true);
    logs->acceptRichText();
    createMenu();

    doc=new QDomDocument("logs");
    domElement=doc->createElement("logs");
    doc->appendChild(domElement);

    sDialog=new Server_Settings(this);



    QHBoxLayout *mainLayout=new QHBoxLayout;
    QVBoxLayout *tabLayout=new QVBoxLayout;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(serverPos);
    layout->addWidget(logs);
   //layout->addWidget(quitButton);
    mainLayout->addLayout(layout);
    mainLayout->addLayout(tabLayout);
    this->setLayout(mainLayout);
    QString temp="127.0.0.1";
    setWindowTitle("Server "+temp+" "+QString::number(port)+" 0");

//    testButton=new QPushButton("Тест");
//    testButton->setIcon(QIcon(QPixmap::fromImage(QImage("/home/user/Desktop/photo.jpg"))));
//    mainLayout->addWidget(testButton);
    messageLength = 0;

}

void MainWindow::on_saveLogs() {
    saveXMLFile();
}
void MainWindow::createMenu(){
//    MenuFile = menuBar()->addMenu("Файл");
//    MenuEdit = menuBar()->addMenu("Правка");
//    MenuFormat=menuBar()->addMenu("Формат");
//    MenuView=menuBar()->addMenu("Вид");
//    MenuHelp=menuBar()->addMenu("Справка");

//    New=MenuFile->addAction("Новый файл");
//    Open=MenuFile->addAction("Открыть");
//    Save=MenuFile->addAction("Сохранить");
//    Save_as=MenuFile->addAction("Сохранить как");
//    Exit=MenuFile->addAction("Выход");
    QMenuBar *t=new QMenuBar(this);
    fileMenu=t->addMenu("&File");
    settingsMenu=t->addMenu("&Settings");
    networkAction=new QAction(tr("Сеть"));
    startAction=new QAction(tr("On"));
    stopAction=new QAction(tr("Off"));
    exitAction=new QAction(tr("Quit"));
    saveXMLAction=new QAction(tr("Save to XML file"));
    fileMenu->addAction(startAction);
    fileMenu->addAction(stopAction);
    fileMenu->addAction(saveXMLAction);
    fileMenu->addAction(exitAction);
    settingsMenu->addAction(networkAction);
    QMenuBar *menu=new QMenuBar(this);
    menu->addMenu(fileMenu);
    menu->addMenu(settingsMenu);
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(start()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));
    connect(saveXMLAction, SIGNAL(triggered()), this, SLOT(on_saveLogs()));
    connect(networkAction, SIGNAL(triggered()), this, SLOT(on_network()));
}

void MainWindow::start() {
    server = new QTcpServer(this);
    if(!server->listen(ip, port)) {
        serverPos->setText(tr("The server could not start: ") + server->errorString());
        logs->append(tr("The server could not start: ")+server->errorString());
    } else {
        setWindowTitle("IP: "+ip.toString()+" PORT: "+QString::number(port)+" 0");
//        serverState->setText("\n ");
//        serverState->setText(" ");
//        serverState->setText(" ");
//        serverState->setText(tr("\nServer is exist on port ") + QString::number(server->serverPort()));
        logs->append(tr("Server started! "));
        connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }
}

void MainWindow::stop() {
    logs->append(tr("Server has been stopped!"));
    for(auto user : users) {
        user->disconnectFromHost();
        usersInfo.remove(user);
        users.removeOne(user);
    }

    server->disconnect();
    server->close();
    setWindowTitle("Server "+QString::number(port)+" "+ip.toString());
}

void MainWindow::on_network() {
    sDialog->ip->setText(ip.toString());
    sDialog->port->setText(QString::number(port));
    sDialog->open();
}

void MainWindow::accept() {
    if (sDialog->ip->text().isEmpty()||sDialog->port->text().isEmpty()) {
        QMessageBox::information(sDialog, tr("Empty line"), tr("Repeat action"));
        return;
    }
    ip=QHostAddress(sDialog->ip->text());
    port=sDialog->port->text().toUInt();
    setWindowTitle("Server "+ip.toString()+" "+QString::number(port)+" 0");
    //QMessageBox::information(sDialog, tr("Успех!"), tr("Вы успешно поменяли настройки сервера!"));
    sDialog->hide();
}

void MainWindow::userListUpdated(){
    QString names;
    QByteArrayList images_64;
    QStringList ips;
    QStringList times;
    QStringList statuses;
    for(std::size_t i=0; i<usersInfo.count(); ++i) {
        names.append(usersInfo.value(users[i])->username);
        names.append(",");
        images_64<<usersInfo.value(users[i])->user_icon_64;
        ips<<usersInfo.value(users[i])->ip;
        statuses<<usersInfo.value(users[i])->user_status;
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

void MainWindow::newConnection() {

    QTcpSocket* newUser = server->nextPendingConnection();
    users << newUser;
    User *user=new User();
    usersInfo.insert(newUser, user);

    connect(newUser, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(newUser, SIGNAL(disconnected()), this, SLOT(logoutUser()));

    QString message="User is connected!";
    setWindowTitle(ip.toString()+":"+QString::number(port)+" "+QString::number(users.count()));
    QDomElement newLog=log(doc, QDateTime::currentDateTime().toString(), newUser->peerAddress().toString(), "SERVER", "serverMessage", message);
    domElement.appendChild(newLog);

    sendMessage("<em>User is connected!</em>");
}

void MainWindow::dataReceived() {
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

            return;
        }
        in >> messageLength;
    }
    //did we get all the bytes?
    if(socket->bytesAvailable() < messageLength) {

        return;  //no
    }
    //yes

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

        }

        if(test=="ICON") {
            QImage image=QImage::fromData(QByteArray::fromBase64(icon),"PNG");
            //QLabel *label=new QLabel(this);
            //label->setPixmap(QPixmap::fromImage(image));
            //label->show();
            testButton->setIcon(QIcon(QPixmap::fromImage(image)));
            usersInfo.value(socket)->username=username;
            usersInfo.value(socket)->set_user_icon(image);
            usersInfo.value(socket)->user_icon_64=icon;
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


            mainData=1;
            messageLength=0;
        } else if(test=="FILESENDEDS") {
            QByteArray block;
            QString filename;
            QString username;
            in>>username;
            in>>filename;
            in>>block;

            for(std::size_t i=0; i<users.count(); ++i) {
                if(usersInfo.value(users[i])->username==username) {
                    QByteArray packet;
                    QDataStream out(&packet, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_1);
                    out<<(quint16)0<<(QString)"FILEFORYOU"<<filename<<block<<usersInfo.value(socket)->username;
                    users[i]->write(packet);

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


            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_1);
            out<<(quint16)0<<(QString)"FILEFORCLIENT"<<usernameSend<<filename<<size<<fileBlock;

            socket->write(block);
//            socket->flush();

            QByteArray blockForFirstClient;
            QDataStream out1(&blockForFirstClient, QIODevice::WriteOnly);
            out1.setVersion(QDataStream::Qt_4_1);
            out1<<(quint16)0<<(QString)"CLIENTACCEPTFILE";
            for(std::size_t i=0; i<usersInfo.count(); ++i) {
                if(usersInfo.value(users[i])->username==usernameSend) {
                    users[i]->write(blockForFirstClient);

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
            newUser->user_icon_64=icon_64;
            newUser->user_status=status;
            newUser->ip=socket->peerAddress().toString();
            newUser->connected_time=QDateTime::currentDateTime();
            usersInfo.insert(socket, newUser);
            userListUpdated();
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
            newUser->user_icon_64=icon_64;
            newUser->user_status=status;
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

            QString messageNew="("+colorStr+","+fontName+","+QString::number(fontWeight)+") "+message;
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
                    QByteArray savedImage=usersInfo.value(savedSocket)->user_icon_64;
                    User *savedUser=new User();
                    savedUser->username=newUsername;
                    savedUser->user_icon_64=savedImage;
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
                    newUser->user_status=status;
                    if(autoFile=="yes") {
                        newUser->autoFileGet=true;
                    } else if(autoFile=="no") {
                        newUser->autoFileGet=true;
                    }
                    logs->append("Новый пользователь: "+newUser->get_user_name()+" со статусом "+newUser->user_status+" с AFG "+autoFile);
                    QImage img=QImage::fromData(icon_bits);
                    testButton->setIcon(QIcon(QPixmap::fromImage(img)));
                }
            }
            messageLength = 0;
        } else {
            logs->append("Пользователь "+usersInfo.value(socket)->get_user_name()+" сменил имя на "+name+" А статус c "+usersInfo.value(socket)->user_status+" на "+status+" А AFG на "+autoFile);
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
            sendMessage(message);
        messageLength = 0;
     }
}

void MainWindow::logoutUser() {
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
    sendMessage(tr("<em>Пользователь отключился.</em>"));

    socket->deleteLater(); //makes qt not lag; once the slot is done executing, will remove socket
}

void MainWindow::sendMessage(const QString &message) {
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

QDomElement MainWindow::makeElement(QDomDocument *domDoc, const QString &strName, const QString &strAttr=nullptr, const QString &Text=nullptr) {
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


QDomElement MainWindow::log(QDomDocument *domDoc, const QString &dateTime, const QString &ip, const QString &name, const QString &type, const QString &message) {
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

void MainWindow::saveXMLFile() {
    QFile file("logs.xml");
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream(&file)<<doc->toString();
        file.close();
    }
}
