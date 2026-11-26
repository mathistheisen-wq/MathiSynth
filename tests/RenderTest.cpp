// Offline render test for MathiSynth.
//
//   MathiSynthTests                 -> renders every factory preset, checks for NaN/silence/clipping
//   MathiSynthTests --wav <dir>     -> also writes one WAV per preset so you can audition them
//   MathiSynthTests --ui <file.png> -> snapshots the editor (needs a display; use xvfb-run on Linux)

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;

    struct Result { float peak = 0.0f, rms = 0.0f, tailPeak = 0.0f; bool finite = true; };

    // Plays a phrase suited to the preset's role, then lets the tail ring out.
    Result renderPreset (MathiSynthProcessor& proc, const juce::String& category, juce::AudioBuffer<float>& out)
    {
        const bool isBass = category == "Bass";
        const int base = isBass ? 36 : (category == "Lead" ? 72 : 60);
        const double seconds = 4.0;
        const int total = (int) (seconds * sampleRate);
        out.setSize (2, total);
        out.clear();

        struct Ev { double t; int note; bool on; };
        std::vector<Ev> events;
        if (isBass || category == "Lead")
        {
            // Overlapping notes -> exercises legato glide.
            events = { { 0.0, base, true }, { 0.6, base + 7, true }, { 0.7, base, false },
                       { 1.2, base + 5, true }, { 1.3, base + 7, false }, { 2.0, base + 5, false } };
        }
        else
        {
            // A minor-7 chord, then an arpeggio line.
            for (int n : { 0, 3, 7, 10 }) events.push_back ({ 0.0, base + n, true });
            for (int n : { 0, 3, 7, 10 }) events.push_back ({ 1.4, base + n, false });
            double t = 1.6;
            for (int n : { 12, 15, 19, 22 }) { events.push_back ({ t, base + n, true }); events.push_back ({ t + 0.15, base + n, false }); t += 0.18; }
        }

        proc.prepareToPlay (sampleRate, blockSize);
        juce::AudioBuffer<float> block (2, blockSize);
        juce::MidiBuffer midi;
        Result r;
        double sumSq = 0.0;
        const int tailStart = (int) (3.6 * sampleRate);

        for (int pos = 0; pos < total; pos += blockSize)
        {
            const int n = std::min (blockSize, total - pos);
            midi.clear();
            for (auto& e : events)
            {
                const int s = (int) (e.t * sampleRate) - pos;
                if (s >= 0 && s < n)
                    midi.addEvent (e.on ? juce::MidiMessage::noteOn (1, e.note, (juce::uint8) 100)
                                        : juce::MidiMessage::noteOff (1, e.note), s);
            }
            block.setSize (2, n, false, false, true);
            proc.processBlock (block, midi);

            for (int ch = 0; ch < 2; ++ch)
            {
                const float* d = block.getReadPointer (ch);
                for (int i = 0; i < n; ++i)
                {
                    if (! std::isfinite (d[i])) r.finite = false;
                    const float a = std::abs (d[i]);
                    r.peak = std::max (r.peak, a);
                    if (pos + i >= tailStart) r.tailPeak = std::max (r.tailPeak, a);
                    sumSq += (double) d[i] * d[i];
                }
                out.copyFrom (ch, pos, block, ch, 0, n);
            }
        }
        r.rms = (float) std::sqrt (sumSq / (2.0 * total));
        return r;
    }

    void writeWav (const juce::File& f, const juce::AudioBuffer<float>& buf)
    {
        f.deleteFile();
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> os (f.createOutputStream().release());
        if (os == nullptr) return;
        if (auto* w = wav.createWriterFor (os.get(), sampleRate, 2, 24, {}, 0))
        {
            os.release();
            std::unique_ptr<juce::AudioFormatWriter> writer (w);
            writer->writeFromAudioSampleBuffer (buf, 0, buf.getNumSamples());
        }
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    juce::StringArray args;
    for (int i = 1; i < argc; ++i) args.add (argv[i]);

    MathiSynthProcessor proc;
    proc.setPlayConfigDetails (0, 2, sampleRate, blockSize);

    if (args.contains ("--ui"))
    {
        const auto file = juce::File::getCurrentWorkingDirectory().getChildFile (args[args.indexOf ("--ui") + 1]);
        proc.presets.load (2);
        std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
        ed->setSize (MathiSynthEditor::designWidth, MathiSynthEditor::designHeight);
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f);
        juce::FileOutputStream fos (file);
        if (fos.openedOk()) { fos.setPosition (0); fos.truncate(); juce::PNGImageFormat().writeImageToStream (img, fos); }
        std::cout << "UI snapshot written to " << file.getFullPathName() << "\n";
        return 0;
    }

    juce::File wavDir;
    if (args.contains ("--wav"))
    {
        wavDir = juce::File::getCurrentWorkingDirectory().getChildFile (args[args.indexOf ("--wav") + 1]);
        wavDir.createDirectory();
    }

    int failures = 0;
    const auto& entries = proc.presets.getEntries();
    std::printf ("%-24s %-8s %8s %8s %9s  %s\n", "Preset", "Role", "Peak", "RMS dB", "Tail", "Status");

    for (int i = 0; i < (int) entries.size(); ++i)
    {
        if (entries[(size_t) i].factoryIndex < 0) continue;
        proc.setCurrentProgram (i);
        juce::AudioBuffer<float> out;
        const auto r = renderPreset (proc, entries[(size_t) i].category, out);

        juce::String status = "ok";
        if (! r.finite)              status = "FAIL: non-finite samples";
        else if (r.peak < 0.02f)     status = "FAIL: too quiet";
        else if (r.peak > 1.0001f)   status = "FAIL: exceeds 0 dBFS";
        if (status != "ok") ++failures;

        std::printf ("%-24s %-8s %8.3f %8.1f %9.4f  %s\n", entries[(size_t) i].name.toRawUTF8(),
                     entries[(size_t) i].category.toRawUTF8(), r.peak,
                     juce::Decibels::gainToDecibels (r.rms), r.tailPeak, status.toRawUTF8());

        if (wavDir.isDirectory())
            writeWav (wavDir.getChildFile (entries[(size_t) i].category + " - " + entries[(size_t) i].name + ".wav"), out);
    }

    // State round-trip: save, change, restore.
    {
        proc.setCurrentProgram (3);
        juce::MemoryBlock mb;
        proc.getStateInformation (mb);
        const float before = proc.apvts.getRawParameterValue (ParamID::cutoff)->load();
        proc.setCurrentProgram (10);
        proc.setStateInformation (mb.getData(), (int) mb.getSize());
        const float after = proc.apvts.getRawParameterValue (ParamID::cutoff)->load();
        const bool ok = std::abs (before - after) < 0.01f && proc.getCurrentProgram() == 3;
        std::printf ("State round-trip: %s\n", ok ? "ok" : "FAIL");
        if (! ok) ++failures;
    }

    std::printf ("\n%d presets, %d failures\n", (int) entries.size(), failures);
    return failures == 0 ? 0 : 1;
}
