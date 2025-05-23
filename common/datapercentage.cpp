/*****************************************************************************
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2025 Codreanu Alexandru <alovada.med@gmail.com>
 *
 * This file is part of the USG project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "datapercentage.h"

DataPercentage::DataPercentage(QObject *parent) : QObject(parent)
{
    resetBloodFlow();
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

QMap<int, PercentileValues> DataPercentage::getPI_CMA_FMF()
{
    CMA_Percentiles[20] = {1.14, 1.21, 1.32, 1.46, 1.61, 1.76, 1.86};
    CMA_Percentiles[21] = {1.18, 1.25, 1.37, 1.51, 1.67, 1.82, 1.92};
    CMA_Percentiles[22] = {1.22, 1.30, 1.42, 1.57, 1.73, 1.89, 1.99};
    CMA_Percentiles[23] = {1.27, 1.34, 1.47, 1.63, 1.79, 1.95, 2.06};
    CMA_Percentiles[24] = {1.31, 1.39, 1.52, 1.68, 1.85, 2.02, 2.12};
    CMA_Percentiles[25] = {1.35, 1.43, 1.57, 1.73, 1.91, 2.08, 2.20};
    CMA_Percentiles[26] = {1.39, 1.47, 1.61, 1.78, 1.97, 2.14, 2.27};
    CMA_Percentiles[27] = {1.42, 1.51, 1.65, 1.83, 2.02, 2.19, 2.32};
    CMA_Percentiles[28] = {1.46, 1.54, 1.69, 1.86, 2.06, 2.24, 2.37};
    CMA_Percentiles[29] = {1.47, 1.57, 1.72, 1.90, 2.09, 2.28, 2.41};
    CMA_Percentiles[30] = {1.49, 1.59, 1.74, 1.92, 2.11, 2.31, 2.44};
    CMA_Percentiles[31] = {1.51, 1.59, 1.74, 1.92, 2.12, 2.32, 2.45};
    CMA_Percentiles[32] = {1.51, 1.59, 1.74, 1.92, 2.12, 2.31, 2.44};
    CMA_Percentiles[33] = {1.49, 1.57, 1.72, 1.90, 2.09, 2.29, 2.41};
    CMA_Percentiles[34] = {1.46, 1.54, 1.69, 1.86, 2.06, 2.25, 2.37};
    CMA_Percentiles[35] = {1.42, 1.50, 1.64, 1.82, 2.00, 2.19, 2.31};
    CMA_Percentiles[36] = {1.37, 1.44, 1.58, 1.75, 1.93, 2.10, 2.22};
    CMA_Percentiles[37] = {1.30, 1.38, 1.51, 1.67, 1.84, 2.00, 2.12};
    CMA_Percentiles[38] = {1.23, 1.30, 1.43, 1.58, 1.74, 1.90, 2.00};
    CMA_Percentiles[39] = {1.15, 1.21, 1.33, 1.46, 1.62, 1.77, 1.86};
    CMA_Percentiles[40] = {1.06, 1.12, 1.22, 1.35, 1.49, 1.62, 1.70};
    CMA_Percentiles[41] = {0.96, 1.02, 1.12, 1.23, 1.36, 1.48, 1.56};
    CMA_Percentiles[42] = {0.86, 0.91, 1.00, 1.10, 1.21, 1.33, 1.40};

    return CMA_Percentiles;
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

        // determinam flux
        if (measuredPI <= values.p5 || measuredPI >= values.p95)
            m_flow = Enums::BLOOD_FLOW::FLOW_ANORMAL;
        else
            m_flow = Enums::BLOOD_FLOW::FLOW_NORMAL;

        // descifrarea
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

        // determinam flux
        if (measuredPI <= values.p5 || measuredPI >= values.p95)
            m_flow = Enums::BLOOD_FLOW::FLOW_ANORMAL;
        else
            m_flow = Enums::BLOOD_FLOW::FLOW_NORMAL;

        // descifrarea
        if (measuredPI < values.p5)
            return "sub percentila 5 (Flux sanguin scăzut - posibil risc de insuficiență placentară)";
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

    } else if (m_type_percentiles == TYPE_PERCENTILES::P_CMA) {

        dataset = getPI_CMA_FMF();
        PercentileValues values = dataset[gestationalWeek];

        // determinam flux
        if (measuredPI <= values.p5 || measuredPI >= values.p95)
            m_flow = Enums::BLOOD_FLOW::FLOW_ANORMAL;
        else
            m_flow = Enums::BLOOD_FLOW::FLOW_NORMAL;

        // descifrarea
        if (measuredPI < values.p5)
            return "sub percentila 5 (Vasodilatație cerebrală severă → risc crescut de restricție de creștere intrauterină sever)";
        else if (measuredPI < values.p10)
            return "între percentila 5 și 10 (risc de hipoxie fetală și stres fetal, sugestie de restricție de creștere intrauterină, se recomandă monitorizare)";
        else if (measuredPI < values.p25)
            return "între percentila 10 și 25 (Limita inferioară a normalului)";
        else if (measuredPI < values.p50)
            return "între percentila 25 și 50 (Normal)";
        else if (measuredPI < values.p75)
            return "între percentila 50 și 75 (Normal superior)";
        else if (measuredPI < values.p90)
            return "între percentila 75 și 90 (flux cerebral ușor rezistent, se recomandă monitorizare)";
        else if (measuredPI < values.p95)
            return "între percentila 90 și 95 (risc de hipoxie fetală și stres fetal)";
        else
            return "peste percentila 95 (rezistența vasculară cerebrală, anemia fetală severă sau hipertensiune fetală)";

    }

    return nullptr;

}

void DataPercentage::resetBloodFlow()
{
    m_flow = Enums::BLOOD_FLOW::FLOW_UNKNOW;
}

Enums::BLOOD_FLOW DataPercentage::getBloodFlow()
{
    return m_flow;
}
