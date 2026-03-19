#include "ui/ClassificationTableModel.h"

#include <QBrush>
#include <utility>

namespace
{

QVariant valueOrDash(const QString &value)
{
    if (value.isEmpty()) {
        return QStringLiteral("-");
    }
    return value;
}

} // namespace

namespace ui
{

ClassificationTableModel::ClassificationTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ClassificationTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(m_rows.size());
}

int ClassificationTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return Count;
}

QVariant ClassificationTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() < 0 || index.row() >= m_rows.size()) {
        return QVariant();
    }

    const domain::ClassificationRow &row = m_rows.at(index.row());

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case Position:
        case CarNumber:
        case LapNumber:
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role == Qt::ForegroundRole) {
        if (row.status == domain::RaceStatus::Retired || row.status == domain::RaceStatus::Disqualified) {
            return QBrush(Qt::darkRed);
        }
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (index.column()) {
    case Position:
        return row.position >= 0 ? QVariant(row.position) : QVariant(QStringLiteral("-"));
    case CarNumber:
        return valueOrDash(row.carNumber);
    case DriverName:
        return valueOrDash(row.driverName);
    case Team:
        return valueOrDash(row.team);
    case Vehicle:
        return valueOrDash(row.vehicle);
    case ClassName:
        return valueOrDash(row.className);
    case Gap:
        return valueOrDash(row.gap);
    case Interval:
        return valueOrDash(row.interval);
    case LapNumber:
        return row.lapNumber >= 0 ? QVariant(row.lapNumber) : QVariant(QStringLiteral("-"));
    case Status:
        return domain::raceStatusToString(row.status);
    default:
        return QVariant();
    }
}

QVariant ClassificationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    switch (section) {
    case Position:
        return QStringLiteral("Pos");
    case CarNumber:
        return QStringLiteral("Car");
    case DriverName:
        return QStringLiteral("Driver");
    case Team:
        return QStringLiteral("Team");
    case Vehicle:
        return QStringLiteral("Vehicle");
    case ClassName:
        return QStringLiteral("Class");
    case Gap:
        return QStringLiteral("Gap");
    case Interval:
        return QStringLiteral("Interval");
    case LapNumber:
        return QStringLiteral("Lap");
    case Status:
        return QStringLiteral("Status");
    default:
        return QVariant();
    }
}

void ClassificationTableModel::setRows(domain::ClassificationRows rows)
{
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
}

const domain::ClassificationRows &ClassificationTableModel::rows() const
{
    return m_rows;
}

} // namespace ui
