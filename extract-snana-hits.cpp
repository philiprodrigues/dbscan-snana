#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <TChain.h>
#include <vector>
#include <iostream>
#include <fstream>

#include "cnpy/cnpy.h"
#include "CLI11.hpp"

void hits_to_file(std::vector<std::string> infilenames, std::string outfilename, std::string format, int N=-1)
{
    TChain ch("snanatrigprim2000/SNSimTree");

    for(auto const& f : infilenames){
        ch.Add(f.c_str());
    }
    TTreeReader reader(&ch);
      
    //Read a vector of Muon objects from the tree entries:
    TTreeReaderValue<std::vector<int>> chans(reader, "Hit_Chan");
    // gentypes:
    // kUnknown=0, kMarl=1, kAPA=2, kCPA=3, kAr39=4, kNeut=5, kKryp=6, kPlon=7, kRdon=8 , kAr42=9
    TTreeReaderValue<std::vector<int>> gentypes(reader, "Hit_True_GenType");
    TTreeReaderValue<std::vector<float>> times(reader, "Hit_Time");
    TTreeReaderValue<std::vector<float>> sadcs(reader, "Hit_SADC");
    TTreeReaderValue<std::vector<float>> true_enu(reader, "True_ENu");
    TTreeReaderValue<std::vector<float>> true_elep(reader, "True_ENu_Lep");

    std::ofstream fout;

    if(format=="text"){
        fout.open(outfilename);
    }
    
    size_t counter=0;
    size_t readout_window=4500; // TPC ticks

    struct Hit
    {
        int chan, time, sadc, gentype;
    };
    
    while(reader.Next()){
        if(N!=-1 && counter>N) break;

        if(counter%10000==0) std::cout << counter << " " << std::flush;
        assert(chans->size() == times->size());
        std::vector<Hit> hits;

        // Make a dummy hit for each event to hold the true neutrino
        // and lepton energies. Since all of the data members have to
        // be integers, we convert the energies from GeV to keV to
        // minimize rounding error
        Hit dummyhit;
        dummyhit.chan=99999999;
        dummyhit.time=static_cast<int>(chans->size());
        dummyhit.sadc=static_cast<int>(1e6*true_enu->at(0));
        dummyhit.gentype=static_cast<int>(1e6*true_elep->at(0));

        hits.push_back(dummyhit);
        
        for(size_t i=0; i<chans->size(); ++i){
            Hit hit;
            hit.chan=chans->at(i);
            hit.time=static_cast<int>(times->at(i))+readout_window*counter;
            hit.sadc=static_cast<int>(sadcs->at(i));
            hit.gentype=gentypes->at(i);
            hits.push_back(hit);
        }

        auto second=hits.begin();
        ++second;
        std::sort(second, hits.end(), [](const Hit& a, const Hit& b) { return a.time < b.time; } );
        
        if(format=="text"){
            for(auto& hit: hits) {
                fout << hit.chan << " " << hit.time << " " << hit.sadc << " " << hit.gentype << std::endl;
            }
        }
        else if(format=="npy"){
            std::vector<unsigned int> hits_arr;
            for(auto& hit: hits){
                hits_arr.push_back(hit.chan);
                hits_arr.push_back(hit.time);
                hits_arr.push_back(hit.sadc);
                hits_arr.push_back(hit.gentype);
            }
            cnpy::npy_save(outfilename, hits_arr.data(), {hits_arr.size()/4, 4}, counter==0 ? "w" : "a");
        }
        ++counter;
    }
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    CLI::App app{"Extract SNAna hits to numpy or test file"};

    std::vector<std::string> in_filenames;
    app.add_option("-i,--input", in_filenames, "Input filenames");
    std::string out_filename;
    app.add_option("-o,--output", out_filename, "Output filename");
    std::string format;
    app.add_option("-f,--format", format, "Output format")->check(CLI::IsMember({"text", "npy"}));
    int N = -1;
    app.add_option("-n", N, "Number of events to read");
    
    CLI11_PARSE(app, argc, argv);
    
    hits_to_file(in_filenames, out_filename, format, N);
}

// Local Variables:
// c-basic-offset: 4
// End:
