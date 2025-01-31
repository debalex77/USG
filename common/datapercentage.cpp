#include "datapercentage.h"

DataPercentage::DataPercentage(QObject *parent) : QObject(parent)
{

}

DataPercentage::~DataPercentage()
{

}

void DataPercentage::setTypePercentile(TYPE_PERCENTILES typePercentile)
{
    m_type_percentiles = typePercentile;
}

QMap<int, PercentileValues> DataPercentage::getFetalWeight_FMF()
{
    fetalWeightPercentiles[22] = {399, 412, 433, 456, 480, 502, 516};
    fetalWeightPercentiles[23] = {472, 488, 512, 540, 568, 596, 640};
    fetalWeightPercentiles[24] = {556, 574, 603, 636, 670, 702, 724};
    fetalWeightPercentiles[25] = {650, 671, 706, 745, 786, 824, 850};
    fetalWeightPercentiles[26] = {756, 781, 822, 868, 916, 960, 986};
    fetalWeightPercentiles[27] = {874, 904, 952, 1006, 1062, 1114, 1150};
    fetalWeightPercentiles[28] = {1006, 1040, 1096, 1158, 1224, 1286, 1318};
    fetalWeightPercentiles[29] = {1150, 1190, 1250, 1324, 1400, 1468, 1508};
    fetalWeightPercentiles[30] = {1300, 1350, 1420, 1504, 1590, 1670, 1715};
    fetalWeightPercentiles[31] = {1470, 1515, 1600, 1696, 1796, 1890, 1950};
    fetalWeightPercentiles[32] = {1635, 1700, 1792, 1896, 2010, 2120, 2180};
    fetalWeightPercentiles[33] = {1820, 1880, 1985, 2106, 2236, 2356, 2416};
    fetalWeightPercentiles[34] = {2010, 2068, 2186, 2320, 2460, 2600, 2684};
    fetalWeightPercentiles[35] = {2194, 2260, 2390, 2538, 2690, 2840, 2916};
    fetalWeightPercentiles[36] = {2370, 2450, 2590, 2750, 2920, 3074, 3180};
    fetalWeightPercentiles[37] = {2540, 2630, 2776, 2950, 3140, 3310, 3410};
    fetalWeightPercentiles[38] = {2700, 2790, 2950, 3140, 3340, 3520, 3630};
    fetalWeightPercentiles[39] = {2840, 2940, 3110, 3310, 3520, 3720, 3840};
    fetalWeightPercentiles[40] = {2960, 3060, 3240, 3456, 3680, 3880, 4000};
    fetalWeightPercentiles[41] = {3050, 3160, 3350, 3570, 3800, 4030, 4170};
    fetalWeightPercentiles[42] = {3120, 3230, 3420, 3650, 3890, 4120, 4270};

    return fetalWeightPercentiles;
}

QMap<int, PercentileValues> DataPercentage::getUmbilicalArteryPIDataset_FMF()
{
    umbelicalArteryPercentiles[20] = {0.955, 1.010, 1.115, 1.218, 1.320, 1.445, 1.553};
    umbelicalArteryPercentiles[21] = {0.939, 0.993, 1.096, 1.197, 1.298, 1.420, 1.526};
    umbelicalArteryPercentiles[22] = {0.922, 0.976, 1.078, 1.176, 1.275, 1.395, 1.499};
    umbelicalArteryPercentiles[23] = {0.906, 0.959, 1.059, 1.155, 1.253, 1.370, 1.472};
    umbelicalArteryPercentiles[24] = {0.889, 0.942, 1.041, 1.134, 1.230, 1.345, 1.446};
    umbelicalArteryPercentiles[25] = {0.871, 0.924, 1.022, 1.113, 1.208, 1.320, 1.420};
    umbelicalArteryPercentiles[26] = {0.854, 0.907, 1.003, 1.092, 1.185, 1.295, 1.395};
    umbelicalArteryPercentiles[27] = {0.836, 0.890, 0.985, 1.070, 1.163, 1.270, 1.371};
    umbelicalArteryPercentiles[28] = {0.818, 0.872, 0.966, 1.049, 1.140, 1.245, 1.346};
    umbelicalArteryPercentiles[29] = {0.800, 0.855, 0.947, 1.028, 1.118, 1.220, 1.322};
    umbelicalArteryPercentiles[30] = {0.782, 0.838, 0.929, 1.007, 1.095, 1.195, 1.299};
    umbelicalArteryPercentiles[31] = {0.763, 0.820, 0.910, 0.986, 1.073, 1.170, 1.275};
    umbelicalArteryPercentiles[32] = {0.744, 0.803, 0.891, 0.965, 1.050, 1.145, 1.252};
    umbelicalArteryPercentiles[33] = {0.725, 0.786, 0.873, 0.944, 1.028, 1.120, 1.229};
    umbelicalArteryPercentiles[34] = {0.706, 0.768, 0.854, 0.923, 1.005, 1.095, 1.207};
    umbelicalArteryPercentiles[35] = {0.687, 0.751, 0.835, 0.902, 0.983, 1.070, 1.184};
    umbelicalArteryPercentiles[36] = {0.668, 0.734, 0.817, 0.881, 0.960, 1.045, 1.162};
    umbelicalArteryPercentiles[37] = {0.649, 0.716, 0.798, 0.860, 0.938, 1.020, 1.140};
    umbelicalArteryPercentiles[38] = {0.630, 0.699, 0.779, 0.839, 0.915, 0.995, 1.118};
    umbelicalArteryPercentiles[39] = {0.610, 0.682, 0.761, 0.818, 0.893, 0.970, 1.097};
    umbelicalArteryPercentiles[40] = {0.591, 0.664, 0.742, 0.797, 0.870, 0.945, 1.075};
    umbelicalArteryPercentiles[41] = {0.572, 0.647, 0.723, 0.776, 0.848, 0.920, 1.053};

    return umbelicalArteryPercentiles;
}

QMap<int, PercentileValues> DataPercentage::getUterineArteryPI_FMF()
{
    uterineArteryPercentiles[20] = {0.70, 0.78, 0.92, 1.11, 1.33, 1.56, 1.74};
    uterineArteryPercentiles[21] = {0.67, 0.75, 0.88, 1.06, 1.27, 1.50, 1.64};
    uterineArteryPercentiles[22] = {0.64, 0.72, 0.81, 1.01, 1.22, 1.42, 1.55};
    uterineArteryPercentiles[23] = {0.62, 0.69, 0.81, 0.98, 1.16, 1.37, 1.50};
    uterineArteryPercentiles[24] = {0.60, 0.66, 0.78, 0.94, 1.12, 1.31, 1.45};
    uterineArteryPercentiles[25] = {0.58, 0.64, 0.75, 0.90, 1.08, 1.26, 1.39};
    uterineArteryPercentiles[26] = {0.56, 0.62, 0.73, 0.87, 1.04, 1.22, 1.34};
    uterineArteryPercentiles[27] = {0.54, 0.60, 0.71, 0.84, 1.00, 1.17, 1.29};
    uterineArteryPercentiles[28] = {0.53, 0.58, 0.68, 0.82, 0.97, 1.13, 1.25};
    uterineArteryPercentiles[29] = {0.51, 0.57, 0.67, 0.79, 0.94, 1.09, 1.21};
    uterineArteryPercentiles[30] = {0.50, 0.55, 0.65, 0.77, 0.91, 1.06, 1.17};
    uterineArteryPercentiles[31] = {0.49, 0.54, 0.63, 0.75, 0.89, 1.04, 1.14};
    uterineArteryPercentiles[32] = {0.48, 0.53, 0.59, 0.73, 0.87, 1.01, 1.11};
    uterineArteryPercentiles[33] = {0.47, 0.52, 0.60, 0.72, 0.85, 0.98, 1.08};
    uterineArteryPercentiles[34] = {0.46, 0.51, 0.59, 0.71, 0.83, 0.96, 1.06};
    uterineArteryPercentiles[35] = {0.46, 0.50, 0.58, 0.69, 0.81, 0.94, 1.03};
    uterineArteryPercentiles[36] = {0.45, 0.49, 0.57, 0.68, 0.80, 0.93, 1.00};
    uterineArteryPercentiles[37] = {0.45, 0.49, 0.57, 0.67, 0.79, 0.91, 0.99};
    uterineArteryPercentiles[38] = {0.44, 0.48, 0.56, 0.66, 0.78, 0.90, 0.97};
    uterineArteryPercentiles[39] = {0.44, 0.48, 0.56, 0.66, 0.77, 0.89, 0.96};
    uterineArteryPercentiles[40] = {0.44, 0.48, 0.55, 0.65, 0.76, 0.88, 0.95};
    uterineArteryPercentiles[41] = {0.44, 0.48, 0.55, 0.65, 0.76, 0.87, 0.94};
    uterineArteryPercentiles[42] = {0.43, 0.47, 0.55, 0.65, 0.76, 0.87, 0.94};

    return uterineArteryPercentiles;
}

QString DataPercentage::determinePIPercentile_FMF(double measuredPI, int gestationalWeek)
{
    QMap<int, PercentileValues> dataset ;
    if (m_type_percentiles == TYPE_PERCENTILES::P_FETAL_WEIGHT) {

        dataset = getFetalWeight_FMF();
        if (! dataset.contains(gestationalWeek)) {
            return "Vârsta gestațională nu este în intervalul valid (20-42 săptămâni).";
        }

        PercentileValues values = dataset[gestationalWeek];

        if (measuredPI < values.p5)
            return "sub percentila 5 (risc de restricție de creștere intrauterină)";
        else if (measuredPI < values.p10)
            return "între percentila 5 și 10 (Limita inferioară a normalului - sugestie la o potențială restricție de creștere)";
        else if (measuredPI < values.p25)
            return "între percentila 10 și 25 (Greutate fetală sub medie, dar normală)";
        else if (measuredPI < values.p50)
            return "între percentila 25 și 50 (Normal)";
        else if (measuredPI < values.p75)
            return "între percentila 50 și 75 (Normal superior)";
        else if (measuredPI < values.p90)
            return "între percentila 75 și 90 (Fetuși cu tendință spre macrosomie)";
        else if (measuredPI < values.p95)
            return "între percentila 90 și 95 (Valori crescute, posibil risc de macrosomie)";
        else
            return "peste percentila 95 (Macrosomie fetală)";

    } else if (m_type_percentiles == TYPE_PERCENTILES::P_UMBILICAL_ARTERY) {

        dataset = getUmbilicalArteryPIDataset_FMF();
        if (! dataset.contains(gestationalWeek)) {
            return "Vârsta gestațională nu este în intervalul valid (20-42 săptămâni).";
        }

        PercentileValues values = dataset[gestationalWeek];

        if (measuredPI < values.p5)
            return "sub percentila 5 (Risc crescut, sunt necesare investigații suplimentare)";
        else if (measuredPI < values.p10)
            return "între percentila 5 și 10 (Ușor scăzut)";
        else if (measuredPI < values.p25)
            return "între percentila 10 și 25 (Limita inferioară a normalului)";
        else if (measuredPI < values.p50)
            return "între percentila 25 și 50 (Normal)";
        else if (measuredPI < values.p75)
            return "între percentila 50 și 75 (Normal superior)";
        else if (measuredPI < values.p90)
            return "între percentila 75 și 90 (Monitorizare recomandată)";
        else if (measuredPI < values.p95)
            return "între percentila 90 și 95 (Valori crescute, se recomandă supraveghere atentă)";
        else
            return "peste percentila 95 (Risc crescut, sunt necesare investigații suplimentare)";

    } else if (m_type_percentiles == TYPE_PERCENTILES::P_UTERINE_ARTERY) {

        dataset = getUterineArteryPI_FMF();
        PercentileValues values = dataset[gestationalWeek];

        if (measuredPI < values.p5)
            return "sub percentila 5 (Flux sanguin scăzut - posibil risc de insuficiență placentară";
        else if (measuredPI < values.p10)
            return "între percentila 5 și 10 (Ușor scăzut - sugestie la o insuficiență placentară)";
        else if (measuredPI < values.p25)
            return "între percentila 10 și 25 (Limita inferioară a normalului)";
        else if (measuredPI < values.p50)
            return "între percentila 25 și 50 (Normal)";
        else if (measuredPI < values.p75)
            return "între percentila 50 și 75 (Normal superior)";
        else if (measuredPI < values.p90)
            return "între percentila 75 și 90 (Monitorizare recomandată)";
        else if (measuredPI < values.p95)
            return "între percentila 90 și 95 (Valori crescute, posibil risc de preeclampsie sau restricție de creștere intrauterină)";
        else
            return "peste percentila 95 (Risc crescut de preeclampsie, insuficiență placentară )";

    }

    return nullptr;

}
