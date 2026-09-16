#ifndef FETALREFERENCERANGES_H
#define FETALREFERENCERANGES_H

#include <QObject>
#include <common/table_sections.h>
#include <data/database.h>

struct PercentileValues
{
    double p5  = 0.0;
    double p10 = 0.0;
    double p25 = 0.0;
    double p50 = 0.0;
    double p75 = 0.0;
    double p90 = 0.0;
    double p95 = 0.0;
};

struct ReferenceInterpretationResult
{
    bool ok = false;
    double measuredValue = 0.0;
    int gestationalWeek = 0;
    PercentileValues ref;
    QString interpretation;
    QString errorString;
    Doppler::bloodFlow flow = Doppler::bloodFlow::Unknow;
};

struct NtPercentileResult
{
    bool ok = false;

    double crl = 0.0;
    double nt = 0.0;

    double p5 = 0.0;
    double p50 = 0.0;
    double p95 = 0.0;

    double mom = 0.0;
    double zScore = 0.0;
    double percentile = 0.0;

    QString interpretation;
    QString errorString;
};

class FetalReferenceRanges : public QObject
{
    Q_OBJECT
public:
    explicit FetalReferenceRanges(DataBase &db, QObject *parent = nullptr);
    ~FetalReferenceRanges() override = default;

    enum class Type
    {
        FetalWeight,
        UmbilicalArteryPI,
        UterineArteryPI,
        MiddleCerebralArteryPI
    };

    ReferenceInterpretationResult determineFmfPercentile(Type type,
                                                         double measuredValue,
                                                         int gestationalWeek) const;

    NtPercentileResult determineNtPercentile(double ntMeasured,
                                             double crl) const;

private:
    DataBase &m_db;
    struct NtReferenceRow
    {
        double crl = 0.0;
        double p5  = 0.0;
        double p50 = 0.0;
        double p95 = 0.0;
    };

    static const QMap<int, PercentileValues> &fetalWeightFmf();
    static const QMap<int, PercentileValues> &umbilicalArteryPiFmf();
    static const QMap<int, PercentileValues> &uterineArteryPiFmf();
    static const QMap<int, PercentileValues> &middleCerebralArteryPiFmf();

    static const QMap<int, PercentileValues> &datasetForType(Type type);

    static QString buildFetalWeightInterpretation(double value, const PercentileValues &ref);
    static QString buildUmbilicalInterpretation(double value, const PercentileValues &ref);
    static QString buildUterineInterpretation(double value, const PercentileValues &ref);
    static QString buildMcaInterpretation(double value, const PercentileValues &ref);

    static Doppler::bloodFlow determineBloodFlow(Type type, double value, const PercentileValues &ref);

    static double lerp(double x, double x0, double y0, double x1, double y1);
    static double normalCdf(double z);
    static QString buildNtInterpretation(double nt, double p5, double p50, double p95, double percentile);

    bool queryNtReferenceExact(double crl,
                               NtReferenceRow &row) const;

    bool queryNtReferenceBounds(double crl,
                                NtReferenceRow &lower,
                                NtReferenceRow &upper,
                                QString &errorString) const;

};

#endif // FETALREFERENCERANGES_H
