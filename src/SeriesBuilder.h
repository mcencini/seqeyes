#ifndef SERIESBUILDER_H
#define SERIESBUILDER_H

#include <QVector>
#include <vector>
#include "external/pulseq/ExternalSequence.h"

// Build merged time/value series per axis from decoded Pulseq blocks.
// Rules:
// - Do NOT fabricate zeros; preserve true continuity across blocks.
// - Insert a NaN separator ONLY when there is a real temporal gap between
//   the previous point time and the next segment start time.
// - All times are expected in internal units (already multiplied by tFactor).

namespace SeriesBuilder {

// RF: build two independent series for amplitude and phase.
void buildRFSeries(
    const std::vector<SeqBlock*>& blocks,
    const QVector<double>& edges,
    double tFactor,
    QVector<double>& rfTimeAmp,
    QVector<double>& rfAmp,
    QVector<double>& rfTimePh,
    QVector<double>& rfPh
);

// Gradients: build merged series for GX, GY, GZ channels.
void buildGradientSeries(
    const std::vector<SeqBlock*>& blocks,
    const QVector<double>& edges,
    double tFactor,
    int channel, // 0=GX, 1=GY, 2=GZ
    QVector<double>& gradTime,
    QVector<double>& gradValues,
    double gradientRasterUs = -1.0
);

// ADC: build merged series for ADC events.
void buildADCSeries(
    const std::vector<SeqBlock*>& blocks,
    const QVector<double>& edges,
    double tFactor,
    QVector<double>& adcTime,
    QVector<double>& adcValues
);

}

#endif // SERIESBUILDER_H



