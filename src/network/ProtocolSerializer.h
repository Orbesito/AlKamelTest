#pragma once

#include <QString>

#include "network/ProtocolMessage.h"

namespace network
{

class ProtocolSerializer
{
public:
    static QString serialize(const ProtocolMessage &message);
};

} // namespace network
