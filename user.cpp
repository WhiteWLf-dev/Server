#include "user.h"

User::User()=default;

void User::add_user(QByteArray contact) {
    current_users.push_back(contact);
}

void User::remove_user(QByteArray contact) {
    for(std::size_t i=0; i<current_users.size(); ++i) {
        if(current_users[i]==contact) {
            current_users.remove(i);
        }
    }
}

bool User::find_user(QByteArray contact) {
    for(std::size_t i=0; i<current_users.size(); ++i) {
        if(current_users[i]==contact) {
            return true;
        }
    }
    return false;
}

void User::clear_users() {
    current_users.clear();
}

void User::set_user_id(QByteArray user_id) {
    this->user_id=user_id;
}

void User::set_username(QString name) {
    this->username=name;
}

void User::set_user_icon(QImage image) {
    this->user_icon=image;
}

QByteArray User::get_user_id() {
    return user_id;
}

QString User::get_user_name() {
    return username;
}
