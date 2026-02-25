#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>
#include <vector>

#include "ExternalSequence.h"

namespace py = pybind11;

// ---------------------------------------------------------------------------
// Version reading (no ExternalSequence instance needed)
// ---------------------------------------------------------------------------

static std::pair<int, int> read_version_from_file(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + filename);

    std::string line;
    bool in_version = false;
    int major = -1, minor = -1;

    while (std::getline(file, line))
    {
        if (line.find("[VERSION]") != std::string::npos)
        {
            in_version = true;
            continue;
        }
        if (in_version)
        {
            // Stop at next section
            if (!line.empty() && line[0] == '[')
                break;
            std::istringstream iss(line);
            std::string key;
            int val;
            if (iss >> key >> val)
            {
                if (key == "major")
                    major = val;
                else if (key == "minor")
                    minor = val;
            }
            if (major != -1 && minor != -1)
                break;
        }
    }

    if (major == -1 || minor == -1)
        throw std::runtime_error("Could not parse version from: " + filename);

    return {major, minor};
}

// ---------------------------------------------------------------------------
// PySequence wrapper
// ---------------------------------------------------------------------------

class PySequence
{
public:
    explicit PySequence(const std::string& filepath)
    {
        auto ver = read_version_from_file(filepath);
        m_version_major = ver.first;
        m_version_minor = ver.second;

        m_seq = CreateLoaderForVersion(m_version_major, m_version_minor);
        if (!m_seq)
            throw std::runtime_error("Unsupported Pulseq version " +
                                     std::to_string(m_version_major) + "." +
                                     std::to_string(m_version_minor));

        if (!m_seq->load(filepath))
            throw std::runtime_error("Failed to load sequence: " + filepath);
    }

    int num_blocks() const { return m_seq->GetNumberOfBlocks(); }
    int version_major() const { return m_version_major; }
    int version_minor() const { return m_version_minor; }

    py::dict definitions() const
    {
        py::dict d;
        auto keys = m_seq->GetAllDefinitions();
        for (const auto& k : keys)
        {
            auto vals = m_seq->GetDefinition(k);
            if (vals.size() == 1)
                d[py::str(k)] = vals[0];
            else
            {
                py::list lst;
                for (double v : vals) lst.append(v);
                d[py::str(k)] = lst;
            }
        }
        return d;
    }

    py::dict get_block(int idx) const
    {
        if (idx < 0 || idx >= m_seq->GetNumberOfBlocks())
            throw std::out_of_range("Block index out of range: " + std::to_string(idx));

        SeqBlock* blk = m_seq->GetBlock(idx);
        if (!blk)
            throw std::runtime_error("GetBlock returned null for index " + std::to_string(idx));

        m_seq->decodeBlock(blk);

        py::dict d;

        // --- duration (us) ---
        d["duration_us"] = blk->GetDuration();

        // --- RF ---
        if (blk->isRF())
        {
            RFEvent& rf = blk->GetRFEvent();
            int n = blk->GetRFLength();
            float* amp_ptr = blk->GetRFAmplitudePtr();
            float* ph_ptr  = blk->GetRFPhasePtr();
            float dwell    = blk->GetRFDwellTime();

            py::array_t<float> rf_amp(n);
            py::array_t<float> rf_phase(n);
            py::array_t<float> rf_time(n);
            auto ra = rf_amp.mutable_unchecked<1>();
            auto rp = rf_phase.mutable_unchecked<1>();
            auto rt = rf_time.mutable_unchecked<1>();
            for (int i = 0; i < n; ++i)
            {
                ra(i) = amp_ptr[i] * rf.amplitude;
                rp(i) = ph_ptr[i];
                rt(i) = rf.delay + i * dwell;
            }
            d["rf_amp"]   = rf_amp;
            d["rf_phase"] = rf_phase;
            d["rf_time"]  = rf_time;
        }
        else
        {
            d["rf_amp"]   = py::none();
            d["rf_phase"] = py::none();
            d["rf_time"]  = py::none();
        }

        // --- Gradients (GX=0, GY=1, GZ=2) ---
        const char* gnames[] = {"gx", "gy", "gz"};
        for (int ch = 0; ch < 3; ++ch)
        {
            std::string tk = std::string(gnames[ch]) + "_time";
            std::string wk = std::string(gnames[ch]) + "_wave";
            if (blk->isTrapGradient(ch))
            {
                GradEvent& g = blk->GetGradEvent(ch);
                // Build trapezoidal waveform: 4 time/value points
                // times: delay, delay+rampUp, delay+rampUp+flat, delay+rampUp+flat+rampDown
                long t0 = g.delay;
                long t1 = t0 + g.rampUpTime;
                long t2 = t1 + g.flatTime;
                long t3 = t2 + g.rampDownTime;
                py::array_t<float> g_time({4});
                py::array_t<float> g_wave({4});
                auto gt = g_time.mutable_unchecked<1>();
                auto gw = g_wave.mutable_unchecked<1>();
                gt(0) = (float)t0; gw(0) = 0.0f;
                gt(1) = (float)t1; gw(1) = g.amplitude;
                gt(2) = (float)t2; gw(2) = g.amplitude;
                gt(3) = (float)t3; gw(3) = 0.0f;
                d[tk.c_str()] = g_time;
                d[wk.c_str()] = g_wave;
            }
            else if (blk->isArbitraryGradient(ch))
            {
                GradEvent& g = blk->GetGradEvent(ch);
                int n = blk->GetArbGradNumSamples(ch);
                float* shape = blk->GetArbGradShapePtr(ch);
                py::array_t<float> g_time(n);
                py::array_t<float> g_wave(n);
                auto gt = g_time.mutable_unchecked<1>();
                auto gw = g_wave.mutable_unchecked<1>();
                // For arbitrary gradients with uniform sampling, use gradient raster time
                // We use the delay + index * default_raster approach
                for (int i = 0; i < n; ++i)
                {
                    gt(i) = (float)(g.delay + i * 10); // default 10us gradient raster
                    gw(i) = shape[i] * g.amplitude;
                }
                d[tk.c_str()] = g_time;
                d[wk.c_str()] = g_wave;
            }
            else
            {
                d[tk.c_str()] = py::none();
                d[wk.c_str()] = py::none();
            }
        }

        // --- ADC ---
        if (blk->isADC())
        {
            ADCEvent& adc = blk->GetADCEvent();
            d["adc_num_samples"] = adc.numSamples;
            d["adc_dwell"]       = adc.dwellTime;   // ns
            d["adc_delay"]       = adc.delay;        // us
        }
        else
        {
            d["adc_num_samples"] = py::none();
            d["adc_dwell"]       = py::none();
            d["adc_delay"]       = py::none();
        }

        blk->free();
        return d;
    }

private:
    std::unique_ptr<ExternalSequence> m_seq;
    int m_version_major {-1};
    int m_version_minor {-1};
};

// ---------------------------------------------------------------------------
// Module definition
// ---------------------------------------------------------------------------

PYBIND11_MODULE(_seqeyes_core, m)
{
    m.doc() = "SeqEyes Pulseq sequence reader (C++ core)";

    m.def("read_version", &read_version_from_file,
          py::arg("filename"),
          "Read (major, minor) version tuple from a .seq file.");

    py::class_<PySequence>(m, "PySequence")
        .def(py::init<const std::string&>(), py::arg("filepath"))
        .def("num_blocks",     &PySequence::num_blocks)
        .def("version_major",  &PySequence::version_major)
        .def("version_minor",  &PySequence::version_minor)
        .def("definitions",    &PySequence::definitions)
        .def("get_block",      &PySequence::get_block, py::arg("idx"));
}
