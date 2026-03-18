#include "network/AppConfig.h"

namespace network
{

AppConfig AppConfig::fromAssignmentDefaults()
{
    AppConfig config;
    config.host = QStringLiteral("eu-sim.datapublisher.alkamelcloud.com");
    config.port = 11001;
    config.user = QStringLiteral("FE_SIM_Eric");
    config.password = QStringLiteral("2m8vzbV87Fu2MvsZ");
    config.appName = QStringLiteral("Alkamel Qt Client");
    config.appVersion = QStringLiteral("0.1.0");
    config.protocolName = QStringLiteral("AKS V2 Protocol");
    config.protocolVersion = QStringLiteral("1.0.0");
    config.joinChannels = {
        QStringLiteral("timing.session.entry"),
        QStringLiteral("timing.session.standings.overall.active")
    };
    return config;
}

} // namespace network
