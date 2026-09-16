#include "fetalreferenceranges.h"

FetalReferenceRanges::FetalReferenceRanges(DataBase &db,
                                           QObject *parent)
    : QObject{parent}
    , m_db(db)
{}

ReferenceInterpretationResult FetalReferenceRanges::determineFmfPercentile(Type type,
                                                                           double measuredValue,
                                                                           int gestationalWeek) const
{
    ReferenceInterpretationResult result;
    result.measuredValue = measuredValue;
    result.gestationalWeek = gestationalWeek;

    const auto &dataset = datasetForType(type);

    if (!dataset.contains(gestationalWeek)) {
        result.errorString = tr("Vârsta gestațională nu este în intervalul valid.");
        return result;
    }

    result.ref = dataset.value(gestationalWeek);
    result.flow = determineBloodFlow(type, measuredValue, result.ref);

    switch (type) {
    case Type::FetalWeight:
        result.interpretation = buildFetalWeightInterpretation(measuredValue, result.ref);
        break;
    case Type::UmbilicalArteryPI:
        result.interpretation = buildUmbilicalInterpretation(measuredValue, result.ref);
        break;
    case Type::UterineArteryPI:
        result.interpretation = buildUterineInterpretation(measuredValue, result.ref);
        break;
    case Type::MiddleCerebralArteryPI:
        result.interpretation = buildMcaInterpretation(measuredValue, result.ref);
        break;
    }

    result.ok = true;
    return result;
}

NtPercentileResult FetalReferenceRanges::determineNtPercentile(double ntMeasured,
                                                               double crl) const
{
    NtPercentileResult result;
    result.nt = ntMeasured;
    result.crl = crl;

    if (ntMeasured <= 0.0) {
        result.errorString = tr("Valoarea NT trebuie să fie > 0.");
        return result;
    }

    if (crl <= 0.0) {
        result.errorString = tr("Valoarea CRL trebuie să fie > 0.");
        return result;
    }

    NtReferenceRow exactRow;
    if (queryNtReferenceExact(crl, exactRow)) {
        result.p5 = exactRow.p5;
        result.p50 = exactRow.p50;
        result.p95 = exactRow.p95;
    } else {
        NtReferenceRow lower;
        NtReferenceRow upper;
        QString errorString;

        if (!queryNtReferenceBounds(crl, lower, upper, errorString)) {
            result.errorString = errorString;
            return result;
        }

        if (qFuzzyCompare(lower.crl + 1.0, upper.crl + 1.0)) {
            result.p5 = lower.p5;
            result.p50 = lower.p50;
            result.p95 = lower.p95;
        } else {
            result.p5 = lerp(crl, lower.crl, lower.p5, upper.crl, upper.p5);
            result.p50 = lerp(crl, lower.crl, lower.p50, upper.crl, upper.p50);
            result.p95 = lerp(crl, lower.crl, lower.p95, upper.crl, upper.p95);
        }
    }

    if (result.p5 <= 0.0 || result.p50 <= 0.0 || result.p95 <= 0.0) {
        result.errorString = tr("Valorile de referință NT sunt invalide.");
        return result;
    }

    result.mom = ntMeasured / result.p50;

    const double sigmaLn = (std::log(result.p95) - std::log(result.p5)) / (2.0 * 1.645);
    if (sigmaLn <= 0.0) {
        result.errorString = tr("Nu s-a putut calcula deviația standard logaritmică pentru NT.");
        return result;
    }

    result.zScore = (std::log(ntMeasured) - std::log(result.p50)) / sigmaLn;
    result.percentile = normalCdf(result.zScore) * 100.0;

    if (result.percentile < 0.0)
        result.percentile = 0.0;
    else if (result.percentile > 100.0)
        result.percentile = 100.0;

    result.interpretation = buildNtInterpretation(result.nt,
                                                  result.p5,
                                                  result.p50,
                                                  result.p95,
                                                  result.percentile);

    result.ok = true;
    return result;
}

const QMap<int, PercentileValues> &FetalReferenceRanges::fetalWeightFmf()
{
    static const QMap<int, PercentileValues> data = {
        {22, {399, 412, 433, 456, 480, 502, 516}},
        {23, {472, 488, 512, 540, 568, 596, 640}},
        {24, {556, 574, 603, 636, 670, 702, 724}},
        {25, {650, 671, 706, 745, 786, 824, 850}},
        {26, {756, 781, 822, 868, 916, 960, 986}},
        {27, {874, 904, 952, 1006, 1062, 1114, 1150}},
        {28, {1006, 1040, 1096, 1158, 1224, 1286, 1318}},
        {29, {1150, 1190, 1250, 1324, 1400, 1468, 1508}},
        {30, {1300, 1350, 1420, 1504, 1590, 1670, 1715}},
        {31, {1470, 1515, 1600, 1696, 1796, 1890, 1950}},
        {32, {1635, 1700, 1792, 1896, 2010, 2120, 2180}},
        {33, {1820, 1880, 1985, 2106, 2236, 2356, 2416}},
        {34, {2010, 2068, 2186, 2320, 2460, 2600, 2684}},
        {35, {2194, 2260, 2390, 2538, 2690, 2840, 2916}},
        {36, {2370, 2450, 2590, 2750, 2920, 3074, 3180}},
        {37, {2540, 2630, 2776, 2950, 3140, 3310, 3410}},
        {38, {2700, 2790, 2950, 3140, 3340, 3520, 3630}},
        {39, {2840, 2940, 3110, 3310, 3520, 3720, 3840}},
        {40, {2960, 3060, 3240, 3456, 3680, 3880, 4000}},
        {41, {3050, 3160, 3350, 3570, 3800, 4030, 4170}},
        {42, {3120, 3230, 3420, 3650, 3890, 4120, 4270}}
    };
    return data;
}

const QMap<int, PercentileValues> &FetalReferenceRanges::umbilicalArteryPiFmf()
{
    static const QMap<int, PercentileValues> data = {
        {20, {0.955, 1.010, 1.115, 1.218, 1.320, 1.445, 1.553}},
        {21, {0.939, 0.993, 1.096, 1.197, 1.298, 1.420, 1.526}},
        {22, {0.922, 0.976, 1.078, 1.176, 1.275, 1.395, 1.499}},
        {23, {0.906, 0.959, 1.059, 1.155, 1.253, 1.370, 1.472}},
        {24, {0.889, 0.942, 1.041, 1.134, 1.230, 1.345, 1.446}},
        {25, {0.871, 0.924, 1.022, 1.113, 1.208, 1.320, 1.420}},
        {26, {0.854, 0.907, 1.003, 1.092, 1.185, 1.295, 1.395}},
        {27, {0.836, 0.890, 0.985, 1.070, 1.163, 1.270, 1.371}},
        {28, {0.818, 0.872, 0.966, 1.049, 1.140, 1.245, 1.346}},
        {29, {0.800, 0.855, 0.947, 1.028, 1.118, 1.220, 1.322}},
        {30, {0.782, 0.838, 0.929, 1.007, 1.095, 1.195, 1.299}},
        {31, {0.763, 0.820, 0.910, 0.986, 1.073, 1.170, 1.275}},
        {32, {0.744, 0.803, 0.891, 0.965, 1.050, 1.145, 1.252}},
        {33, {0.725, 0.786, 0.873, 0.944, 1.028, 1.120, 1.229}},
        {34, {0.706, 0.768, 0.854, 0.923, 1.005, 1.095, 1.207}},
        {35, {0.687, 0.751, 0.835, 0.902, 0.983, 1.070, 1.184}},
        {36, {0.668, 0.734, 0.817, 0.881, 0.960, 1.045, 1.162}},
        {37, {0.649, 0.716, 0.798, 0.860, 0.938, 1.020, 1.140}},
        {38, {0.630, 0.699, 0.779, 0.839, 0.915, 0.995, 1.118}},
        {39, {0.610, 0.682, 0.761, 0.818, 0.893, 0.970, 1.097}},
        {40, {0.591, 0.664, 0.742, 0.797, 0.870, 0.945, 1.075}},
        {41, {0.572, 0.647, 0.723, 0.776, 0.848, 0.920, 1.053}}
    };
    return data;
}

const QMap<int, PercentileValues> &FetalReferenceRanges::uterineArteryPiFmf()
{
    static const QMap<int, PercentileValues> data = {
        {20, {0.70, 0.78, 0.92, 1.11, 1.33, 1.56, 1.74}},
        {21, {0.67, 0.75, 0.88, 1.06, 1.27, 1.50, 1.64}},
        {22, {0.64, 0.72, 0.81, 1.01, 1.22, 1.42, 1.55}},
        {23, {0.62, 0.69, 0.81, 0.98, 1.16, 1.37, 1.50}},
        {24, {0.60, 0.66, 0.78, 0.94, 1.12, 1.31, 1.45}},
        {25, {0.58, 0.64, 0.75, 0.90, 1.08, 1.26, 1.39}},
        {26, {0.56, 0.62, 0.73, 0.87, 1.04, 1.22, 1.34}},
        {27, {0.54, 0.60, 0.71, 0.84, 1.00, 1.17, 1.29}},
        {28, {0.53, 0.58, 0.68, 0.82, 0.97, 1.13, 1.25}},
        {29, {0.51, 0.57, 0.67, 0.79, 0.94, 1.09, 1.21}},
        {30, {0.50, 0.55, 0.65, 0.77, 0.91, 1.06, 1.17}},
        {31, {0.49, 0.54, 0.63, 0.75, 0.89, 1.04, 1.14}},
        {32, {0.48, 0.53, 0.59, 0.73, 0.87, 1.01, 1.11}},
        {33, {0.47, 0.52, 0.60, 0.72, 0.85, 0.98, 1.08}},
        {34, {0.46, 0.51, 0.59, 0.71, 0.83, 0.96, 1.06}},
        {35, {0.46, 0.50, 0.58, 0.69, 0.81, 0.94, 1.03}},
        {36, {0.45, 0.49, 0.57, 0.68, 0.80, 0.93, 1.00}},
        {37, {0.45, 0.49, 0.57, 0.67, 0.79, 0.91, 0.99}},
        {38, {0.44, 0.48, 0.56, 0.66, 0.78, 0.90, 0.97}},
        {39, {0.44, 0.48, 0.56, 0.66, 0.77, 0.89, 0.96}},
        {40, {0.44, 0.48, 0.55, 0.65, 0.76, 0.88, 0.95}},
        {41, {0.44, 0.48, 0.55, 0.65, 0.76, 0.87, 0.94}},
        {42, {0.43, 0.47, 0.55, 0.65, 0.76, 0.87, 0.94}}
    };
    return data;
}

const QMap<int, PercentileValues> &FetalReferenceRanges::middleCerebralArteryPiFmf()
{
    static const QMap<int, PercentileValues> data = {
        {20, {1.14, 1.21, 1.32, 1.46, 1.61, 1.76, 1.86}},
        {21, {1.18, 1.25, 1.37, 1.51, 1.67, 1.82, 1.92}},
        {22, {1.22, 1.30, 1.42, 1.57, 1.73, 1.89, 1.99}},
        {23, {1.27, 1.34, 1.47, 1.63, 1.79, 1.95, 2.06}},
        {24, {1.31, 1.39, 1.52, 1.68, 1.85, 2.02, 2.12}},
        {25, {1.35, 1.43, 1.57, 1.73, 1.91, 2.08, 2.20}},
        {26, {1.39, 1.47, 1.61, 1.78, 1.97, 2.14, 2.27}},
        {27, {1.42, 1.51, 1.65, 1.83, 2.02, 2.19, 2.32}},
        {28, {1.46, 1.54, 1.69, 1.86, 2.06, 2.24, 2.37}},
        {29, {1.47, 1.57, 1.72, 1.90, 2.09, 2.28, 2.41}},
        {30, {1.49, 1.59, 1.74, 1.92, 2.11, 2.31, 2.44}},
        {31, {1.51, 1.59, 1.74, 1.92, 2.12, 2.32, 2.45}},
        {32, {1.51, 1.59, 1.74, 1.92, 2.12, 2.31, 2.44}},
        {33, {1.49, 1.57, 1.72, 1.90, 2.09, 2.29, 2.41}},
        {34, {1.46, 1.54, 1.69, 1.86, 2.06, 2.25, 2.37}},
        {35, {1.42, 1.50, 1.64, 1.82, 2.00, 2.19, 2.31}},
        {36, {1.37, 1.44, 1.58, 1.75, 1.93, 2.10, 2.22}},
        {37, {1.30, 1.38, 1.51, 1.67, 1.84, 2.00, 2.12}},
        {38, {1.23, 1.30, 1.43, 1.58, 1.74, 1.90, 2.00}},
        {39, {1.15, 1.21, 1.33, 1.46, 1.62, 1.77, 1.86}},
        {40, {1.06, 1.12, 1.22, 1.35, 1.49, 1.62, 1.70}},
        {41, {0.96, 1.02, 1.12, 1.23, 1.36, 1.48, 1.56}},
        {42, {0.86, 0.91, 1.00, 1.10, 1.21, 1.33, 1.40}}
    };
    return data;
}

const QMap<int, PercentileValues> &FetalReferenceRanges::datasetForType(Type type)
{
    switch (type) {
    case Type::FetalWeight:
        return fetalWeightFmf();
    case Type::UmbilicalArteryPI:
        return umbilicalArteryPiFmf();
    case Type::UterineArteryPI:
        return uterineArteryPiFmf();
    case Type::MiddleCerebralArteryPI:
        return middleCerebralArteryPiFmf();
    }

    return fetalWeightFmf();
}

QString FetalReferenceRanges::buildFetalWeightInterpretation(double value, const PercentileValues &ref)
{
    if (value < ref.p5)
        return tr("sub percentila 5 (risc de restricție de creștere intrauterină)");
    if (value < ref.p10)
        return tr("între percentila 5 și 10 (limita inferioară a normalului - sugestie la o potențială restricție de creștere)");
    if (value < ref.p25)
        return tr("între percentila 10 și 25 (greutate fetală sub medie, dar normală)");
    if (value < ref.p50)
        return tr("între percentila 25 și 50 (normal)");
    if (value < ref.p75)
        return tr("între percentila 50 și 75 (normal superior)");
    if (value < ref.p90)
        return tr("între percentila 75 și 90 (făt cu tendință spre macrosomie)");
    if (value < ref.p95)
        return tr("între percentila 90 și 95 (valori crescute, posibil risc de macrosomie)");

    return tr("peste percentila 95 (macrosomie fetală)");
}

QString FetalReferenceRanges::buildUmbilicalInterpretation(double value, const PercentileValues &ref)
{
    if (value < ref.p5)
        return tr("sub percentila 5 (risc crescut, sunt necesare investigații suplimentare)");
    if (value < ref.p10)
        return tr("între percentila 5 și 10 (ușor scăzut)");
    if (value < ref.p25)
        return tr("între percentila 10 și 25 (limita inferioară a normalului)");
    if (value < ref.p50)
        return tr("între percentila 25 și 50 (normal)");
    if (value < ref.p75)
        return tr("între percentila 50 și 75 (normal superior)");
    if (value < ref.p90)
        return tr("între percentila 75 și 90 (monitorizare recomandată)");
    if (value < ref.p95)
        return tr("între percentila 90 și 95 (valori crescute, se recomandă supraveghere atentă)");

    return tr("peste percentila 95 (risc crescut, sunt necesare investigații suplimentare)");
}

QString FetalReferenceRanges::buildUterineInterpretation(double value, const PercentileValues &ref)
{
    if (value < ref.p5)
        return tr("sub percentila 5 (flux sanguin scăzut - posibil risc de insuficiență placentară)");
    if (value < ref.p10)
        return tr("între percentila 5 și 10 (ușor scăzut - sugestie la o insuficiență placentară)");
    if (value < ref.p25)
        return tr("între percentila 10 și 25 (limita inferioară a normalului)");
    if (value < ref.p50)
        return tr("între percentila 25 și 50 (normal)");
    if (value < ref.p75)
        return tr("între percentila 50 și 75 (normal superior)");
    if (value < ref.p90)
        return tr("între percentila 75 și 90 (monitorizare recomandată)");
    if (value < ref.p95)
        return tr("între percentila 90 și 95 (valori crescute, posibil risc de preeclampsie sau restricție de creștere intrauterină)");

    return tr("peste percentila 95 (risc crescut de preeclampsie, insuficiență placentară)");
}

QString FetalReferenceRanges::buildMcaInterpretation(double value, const PercentileValues &ref)
{
    if (value < ref.p5)
        return tr("sub percentila 5 (vasodilatație cerebrală severă -> risc crescut de restricție de creștere intrauterină sever)");
    if (value < ref.p10)
        return tr("între percentila 5 și 10 (risc de hipoxie fetală și stres fetal, sugestie de restricție de creștere intrauterină, se recomandă monitorizare)");
    if (value < ref.p25)
        return tr("între percentila 10 și 25 (limita inferioară a normalului)");
    if (value < ref.p50)
        return tr("între percentila 25 și 50 (normal)");
    if (value < ref.p75)
        return tr("între percentila 50 și 75 (normal superior)");
    if (value < ref.p90)
        return tr("între percentila 75 și 90 (flux cerebral ușor rezistent, se recomandă monitorizare)");
    if (value < ref.p95)
        return tr("între percentila 90 și 95 (risc de hipoxie fetală și stres fetal)");

    return tr("peste percentila 95 (rezistență vasculară cerebrală crescută, posibilă anemie fetală severă sau hipertensiune fetală)");
}

Doppler::bloodFlow FetalReferenceRanges::determineBloodFlow(Type type,
                                                            double value,
                                                            const PercentileValues &ref)
{
    if (type == Type::FetalWeight)
        return Doppler::bloodFlow::Unknow;

    if (value <= ref.p5 || value >= ref.p95)
        return Doppler::bloodFlow::Anormal;

    return Doppler::bloodFlow::Normal;
}

double FetalReferenceRanges::lerp(double x, double x0, double y0, double x1, double y1)
{
    if (qFuzzyCompare(x0, x1))
        return y0;

    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

double FetalReferenceRanges::normalCdf(double z)
{
    return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
}

QString FetalReferenceRanges::buildNtInterpretation(double nt, double p5, double p50, double p95, double percentile)
{
    if (nt < p5)
        return QObject::tr("sub percentila 5 (NT sub limita inferioară a intervalului de referință)");

    if (nt > p95)
        return QObject::tr("peste percentila 95 (NT crescut peste intervalul de referință)");

    if (std::abs(nt - p50) < 0.01)
        return QObject::tr("aproape de percentila 50 (valoare mediană)");

    if (nt < p50)
        return QObject::tr("în interval normal, sub mediană (~percentila %1)")
            .arg(QString::number(percentile, 'f', 1));

    return QObject::tr("în interval normal, peste mediană (~percentila %1)")
        .arg(QString::number(percentile, 'f', 1));
}

bool FetalReferenceRanges::queryNtReferenceExact(double crl,
                                                 NtReferenceRow &row) const
{
    if (!m_db.getDatabase().isValid() || !m_db.getDatabase().isOpen())
        return false;

    QSqlQuery q(m_db.getDatabase());
    q.prepare(R"(
        SELECT CAST(crl AS REAL),
               `5_centile`,
               `50_centile`,
               `95_centile`
        FROM normograms
        WHERE name = 'normograma_nt'
          AND CAST(crl AS REAL) = :crl
        LIMIT 1
    )");
    q.bindValue(":crl", crl);

    if (!q.exec())
        return false;

    if (!q.next())
        return false;

    row.crl = q.value(0).toDouble();
    row.p5  = q.value(1).toDouble();
    row.p50 = q.value(2).toDouble();
    row.p95 = q.value(3).toDouble();
    return true;
}

bool FetalReferenceRanges::queryNtReferenceBounds(double crl,
                                                  NtReferenceRow &lower,
                                                  NtReferenceRow &upper,
                                                  QString &errorString) const
{
    if (!m_db.getDatabase().isValid()) {
        errorString = tr("Conexiunea la baza de date este invalidă.");
        return false;
    }

    if (!m_db.getDatabase().isOpen()) {
        errorString = tr("Conexiunea la baza de date nu este deschisă.");
        return false;
    }

    {
        QSqlQuery q(m_db.getDatabase());
        q.prepare(R"(
            SELECT CAST(crl AS REAL),
                   `5_centile`,
                   `50_centile`,
                   `95_centile`
            FROM normograms
            WHERE name = 'normograma_nt'
              AND CAST(crl AS REAL) <= :crl
            ORDER BY CAST(crl AS REAL) DESC
            LIMIT 1
        )");
        q.bindValue(":crl", crl);

        if (!q.exec()) {
            errorString = tr("Eroare la citirea limitei inferioare NT: %1")
            .arg(q.lastError().text());
            return false;
        }

        if (!q.next()) {
            errorString = tr("Nu există valori NT inferioare pentru CRL=%1.")
                              .arg(QString::number(crl, 'f', 2));
            return false;
        }

        lower.crl = q.value(0).toDouble();
        lower.p5  = q.value(1).toDouble();
        lower.p50 = q.value(2).toDouble();
        lower.p95 = q.value(3).toDouble();
    }

    {
        QSqlQuery q(m_db.getDatabase());
        q.prepare(R"(
            SELECT CAST(crl AS REAL),
                   `5_centile`,
                   `50_centile`,
                   `95_centile`
            FROM normograms
            WHERE name = 'normograma_nt'
              AND CAST(crl AS REAL) >= :crl
            ORDER BY CAST(crl AS REAL) ASC
            LIMIT 1
        )");
        q.bindValue(":crl", crl);

        if (!q.exec()) {
            errorString = tr("Eroare la citirea limitei superioare NT: %1")
            .arg(q.lastError().text());
            return false;
        }

        if (!q.next()) {
            errorString = tr("Nu există valori NT superioare pentru CRL=%1.")
                              .arg(QString::number(crl, 'f', 2));
            return false;
        }

        upper.crl = q.value(0).toDouble();
        upper.p5  = q.value(1).toDouble();
        upper.p50 = q.value(2).toDouble();
        upper.p95 = q.value(3).toDouble();
    }

    return true;
}
