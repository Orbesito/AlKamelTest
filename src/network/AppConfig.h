#pragma once

#include <QString>
#include <QStringList>
#include <QtGlobal>

namespace network
{

struct AppConfig
{
    QString host;
    quint16 port = 11001;

    QString user;
    QString password;

    QString appName;
    QString appVersion;
    QString protocolName;
    QString protocolVersion;
    QStringList joinChannels;

    static AppConfig fromAssignmentDefaults();
};

} // namespace network
