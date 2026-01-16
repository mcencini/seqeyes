#ifndef KSPACE_TRAJECTORY_H
#define KSPACE_TRAJECTORY_H

#include <QVector>
#include <QString>
#include <vector>

class SeqBlock;

namespace KSpaceTrajectory
{

struct Input
{
    const std::vector<SeqBlock*>& blocks;
    const QVector<double>& blockEdges;
    double tFactor = 1.0;
    bool supportsRfUseMetadata = false;
    double rfRasterUs = 1.0;       // microseconds
    double gradientRasterUs = -1.0; // microseconds
    QVector<double> adcEventTimesInternal;
    // Optional system parameters for RF-use guessing (v1.4.x fallback)
    double b0Tesla = 0.0;          // If 0, ppm fallback from freqOffset is disabled
};

struct Result
{
    QVector<double> kx;
    QVector<double> ky;
    QVector<double> kz;
    QVector<double> t;       // seconds
    QVector<double> t_adc;   // seconds
    QVector<double> kx_adc;
    QVector<double> ky_adc;
    QVector<double> kz_adc;
    QVector<double> excitationTimesInternal;
    QVector<double> refocusingTimesInternal;
    QVector<char>   rfUsePerBlock;   // per block rf.use ('e','r','s','i','p','u' or 0)
    bool rfUseGuessed = false;
    QString warning;
};

Result compute(const Input& input);

} // namespace KSpaceTrajectory

#endif // KSPACE_TRAJECTORY_H
