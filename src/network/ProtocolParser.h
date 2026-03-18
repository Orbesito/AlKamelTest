#pragma once

#include <QString>

#include "network/ProtocolMessage.h"

namespace network
{

struct ProtocolParseResult
{
    bool ok = false;
    ProtocolMessage message;
    QString error;
};

class ProtocolParser
{
public:
    static ProtocolParseResult parseLine(const QString &line);
};

} // namespace network
