#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

#include <vector>
#include <iostream>
#include <fstream>

void hits_to_text(const char* outfilename)
{
    TFile* f=TFile::Open("/dune/data2/users/plasorak/SNana_real_flux_20200417_0.root");
  
    TTreeReader reader("snanatrigprim2000/SNSimTree", f);
  
    //Read a vector of Muon objects from the tree entries:
    TTreeReaderValue<std::vector<int>> chans(reader, "Hit_Chan");
    // gentypes:
    // kUnknown=0, kMarl=1, kAPA=2, kCPA=3, kAr39=4, kNeut=5, kKryp=6, kPlon=7, kRdon=8 , kAr42=9
    TTreeReaderValue<std::vector<int>> gentypes(reader, "Hit_True_GenType");
    TTreeReaderValue<std::vector<float>> times(reader, "Hit_Time");
    TTreeReaderValue<std::vector<float>> sadcs(reader, "Hit_SADC");

    std::ofstream fout(outfilename);
    
    size_t counter=0;
    size_t readout_window=4500; // TPC ticks
    
    while(reader.Next() && counter<1000){
        assert(chans->size() == times->size());
        for(size_t i=0; i<chans->size(); ++i){
            fout << chans->at(i) << " " << (times->at(i)+readout_window*counter) << " " << sadcs->at(i) << " " << gentypes->at(i) << std::endl;
        }
        ++counter;
    }
}

// Local Variables:
// c-basic-offset: 4
// End:
