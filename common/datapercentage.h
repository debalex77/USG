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

    enum TYPE_PERCENTILES
    {
        P_FETAL_WEIGHT,
        P_UMBILICAL_ARTERY,
        P_UTERINE_ARTERY
    };

    void setTypePercentile(TYPE_PERCENTILES typePercentile);

    // masa fatului
    QMap<int, PercentileValues> getFetalWeight_FMF();
    // PI a.umbilical
    QMap<int, PercentileValues> getUmbilicalArteryPIDataset_FMF();
    // PI a.uterine
    QMap<int, PercentileValues> getUterineArteryPI_FMF();

    QString determinePIPercentile_FMF(double measuredPI, int gestationalWeek);

private:
    QMap<int, PercentileValues> umbelicalArteryPercentiles;
    QMap<int, PercentileValues> uterineArteryPercentiles;
    QMap<int, PercentileValues> fetalWeightPercentiles;
    TYPE_PERCENTILES m_type_percentiles;
};

#endif // DATAPERCENTAGE_H
