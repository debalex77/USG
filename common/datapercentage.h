#ifndef DATAPERCENTAGE_H
#define DATAPERCENTAGE_H

#include <QObject>
#include <QMap>

struct PercentileValues {
    double p5;
    double p10;
    double p25;
    double p50;
    double p75;
    double p90;
    double p95;
};

class DataPercentage : public QObject
{
    Q_OBJECT
public:
    explicit DataPercentage(QObject* parent = nullptr);
    ~DataPercentage();

    // doppler a.umbilical
    QMap<int, PercentileValues> getUmbilicalArteryPIDataset_FMF();
    // doppler a.uterine
    QMap<int, PercentileValues> getUterineArteryPI_FMF();

    QString determinePIPercentile_FMF(double measuredPI, int gestationalWeek);

private:
    QMap<int, PercentileValues> umbelicalArteryPercentiles;
    QMap<int, PercentileValues> uterineArteryPercentiles;
};

#endif // DATAPERCENTAGE_H
