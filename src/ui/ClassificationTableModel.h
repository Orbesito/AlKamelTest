#pragma once

#include <QAbstractTableModel>

#include "domain/RaceState.h"

namespace ui
{

class ClassificationTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ClassificationTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setRows(domain::ClassificationRows rows);
    [[nodiscard]] const domain::ClassificationRows &rows() const;

private:
    enum Column
    {
        Position = 0,
        CarNumber,
        DriverName,
        Team,
        Vehicle,
        ClassName,
        Gap,
        Interval,
        LapNumber,
        Status,
        Count
    };

    domain::ClassificationRows m_rows;
};

} // namespace ui
