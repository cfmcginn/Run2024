//c and cpp
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//root
#include "TEnv.h"
#include "TFile.h"
#include "TH1D.h"
#include "TSystem.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

//Temp choice, nasty pathing
#include "/afs/cern.ch/work/c/cmcginn/private/Projects/UPCopenHFanalysis/include/etree.h"

int dTurnOn(std::string inFileName, std::string saveTag = "NoTag")
{
  gSystem->mkdir("output");
  //Create histograms and output file
  std::string outFileName = "output/dTurnOn_" + saveTag + ".root";
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");    
  TEnv* config_p = new TEnv();  
  
  const Int_t nPtBins = 10;  
  const Double_t ptBins[nPtBins+1] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.5, 8.0, 10.0, 12.0, 20.0};

  const Int_t nJtPtThresh = 4;
  const Double_t jtPtThresh[nJtPtThresh] = {4.0, 8.0, 12.0, 16.0};

  TH1D* dPtDenom_h = new TH1D("dPtDenom_h", ";D p_{T} (GeV);Counts", nPtBins, ptBins);
  TH1D* dPtNum_h[nJtPtThresh];

  const Int_t nTrkBins = 60;
  const Int_t nTrkMin = 0;
  const Int_t nTrkMax = 240;

  const Double_t trkPtMin = 0.5;
  const Double_t trkAbsEtaMax = 2.4;
  
  TH1D* nTrkWithDDenom_h = new TH1D("nTrkWithDDenom_h", ";N_{trk};Counts", nTrkBins, nTrkMin, nTrkMax);
  TH1D* nTrkWithDNum_h[nJtPtThresh];

  config_p->SetValue("NJTPTTHRESH", nJtPtThresh);
  
  for(Int_t jI = 0; jI < nJtPtThresh; ++jI){
    std::string histName = Form("dPtNum_Jt%d_h", (Int_t)jtPtThresh[jI]);
    std::string title = Form(";D p_{T} (GeV);Counts (L1 Jet E_{T} > %d GeV)", (Int_t)jtPtThresh[jI]);

    dPtNum_h[jI] = new TH1D(histName.c_str(), title.c_str(), nPtBins, ptBins);
    config_p->SetValue(("JTPTTHRESH." + std::to_string(jI)).c_str(), jtPtThresh[jI]);

    histName = Form("nTrkWithDNum_Jt%d_h", (Int_t)jtPtThresh[jI]);
    title = Form(";N_{trk};Counts (L1 Jet E_{T} > %d GeV)", (Int_t)jtPtThresh[jI]);

    nTrkWithDNum_h[jI] = new TH1D(histName.c_str(), title.c_str(), nTrkBins, nTrkMin, nTrkMax);
  }

  config_p->SetValue("SAVETAG", saveTag.c_str());
  config_p->SetValue("TRKPTMIN", trkPtMin);
  config_p->SetValue("TRKABSETAMAX", trkAbsEtaMax);
  
  //Create a file list from the input, root or txt allowed
  std::vector<std::string> fileList;
  if(inFileName.find(".root") != std::string::npos) fileList.push_back(inFileName);
  else if(inFileName.find(".txt") != std::string::npos){
    std::ifstream inFile(inFileName.c_str());
    std::string tempStr;
    //Parse txt with fstream and getline for root files
    while(std::getline(inFile, tempStr)){
      if(tempStr.find(".root") == std::string::npos) continue;
      fileList.push_back(tempStr);
    }
    inFile.close();
  }
  else{//return if not root or txt
    std::cout << "Given input '" << inFileName << "' is not .root or .txt, return 1" << std::endl;
    return 1;
  }

  //Create the etree for gap testing
  hfupc::etree* etr = new hfupc::etree();
  const float gap_gapEMinMinus = 8.6;
  const float gap_gapEMinPlus = 9.2;
  etr->setRapidityGapEMinMinus(gap_gapEMinMinus);
  etr->setRapidityGapEMinPlus(gap_gapEMinPlus);
  
  //pre-parse files for totalnEntries, convenient counting
  ULong64_t totalEntries = 0;
  std::cout << "Processing " << fileList.size() << " files..." << std::endl;
  for(unsigned fI = 0; fI < fileList.size(); ++fI){
    //    std::cout << " " << fI << "/" << fileList.size() << ": '" << fileList[fI] << "'" << std::endl;

    TFile* inFile_p = new TFile(fileList[fI].c_str(), "READ");
    TTree* l1Tree_p = (TTree*)inFile_p->Get("l1UpgradeEmuTree/L1UpgradeTree");

    totalEntries += l1Tree_p->GetEntries();
    
    inFile_p->Close();
    delete inFile_p;
  }

  std::cout << " Corresponds to " << totalEntries << " events..." << std::endl;
  //Setup some cout params
  const ULong64_t nPrint = 10000;
  ULong64_t currEntry = 0;
  
  for(unsigned fI = 0; fI < fileList.size(); ++fI){
    //grab file, l1 and dtrees
    TFile* inFile_p = new TFile(fileList[fI].c_str(), "READ");
    TTree* pfTree_p = (TTree*)inFile_p->Get("particleFlowAnalyser/pftree");
    TTree* hltTree_p = (TTree*)inFile_p->Get("hltanalysis/HltTree");
    TTree* l1Tree_p = (TTree*)inFile_p->Get("l1UpgradeEmuTree/L1UpgradeTree");
    TTree* dTree_p = (TTree*)inFile_p->Get("Dfinder/ntDkpi");
    TTree* trkTree_p = (TTree*)inFile_p->Get("ppTracks/trackTree");    

    //Handle pf tree branches
    std::vector<float>* pfE_p=nullptr;
    std::vector<float>* pfPt_p=nullptr;
    std::vector<float>* pfEta_p=nullptr;
    std::vector<int>* pfId_p=nullptr;

    pfTree_p->SetBranchStatus("*", 0);
    pfTree_p->SetBranchStatus("pfE", 1);
    pfTree_p->SetBranchStatus("pfPt", 1);
    pfTree_p->SetBranchStatus("pfEta", 1);
    pfTree_p->SetBranchStatus("pfId", 1);

    pfTree_p->SetBranchAddress("pfE", &pfE_p);
    pfTree_p->SetBranchAddress("pfPt", &pfPt_p);
    pfTree_p->SetBranchAddress("pfEta", &pfEta_p);
    pfTree_p->SetBranchAddress("pfId", &pfId_p);
    
    //Handle HLT tree branches
    Int_t HLT_HIUPC_ZDC1nOR_MinPixelCluster400_MaxPixelCluster10000_v8;
    Int_t HLT_HIUPC_ZDC1nOR_SinglePixelTrackLowPt_MaxPixelCluster400_v8;
    Int_t HLT_HIUPC_ZDC1nOR_SinglePixelTrack_MaxPixelTrack_v8;

    hltTree_p->SetBranchStatus("*", 0);
    hltTree_p->SetBranchStatus("HLT_HIUPC_ZDC1nOR_MinPixelCluster400_MaxPixelCluster10000_v8", 1);
    hltTree_p->SetBranchStatus("HLT_HIUPC_ZDC1nOR_SinglePixelTrackLowPt_MaxPixelCluster400_v8", 1);
    hltTree_p->SetBranchStatus("HLT_HIUPC_ZDC1nOR_SinglePixelTrack_MaxPixelTrack_v8", 1);

    hltTree_p->SetBranchAddress("HLT_HIUPC_ZDC1nOR_MinPixelCluster400_MaxPixelCluster10000_v8", &HLT_HIUPC_ZDC1nOR_MinPixelCluster400_MaxPixelCluster10000_v8);
    hltTree_p->SetBranchAddress("HLT_HIUPC_ZDC1nOR_SinglePixelTrackLowPt_MaxPixelCluster400_v8", &HLT_HIUPC_ZDC1nOR_SinglePixelTrackLowPt_MaxPixelCluster400_v8);
    hltTree_p->SetBranchAddress("HLT_HIUPC_ZDC1nOR_SinglePixelTrack_MaxPixelTrack_v8", &HLT_HIUPC_ZDC1nOR_SinglePixelTrack_MaxPixelTrack_v8);
    
    //Handle l1 tree jet branches
    TTreeReader l1Reader(l1Tree_p);
    TTreeReaderValue<std::vector<float> > jetEt(l1Reader, "jetEt");
    TTreeReaderValue<std::vector<short> > jetBx(l1Reader, "jetBx");
    
    l1Tree_p->SetBranchStatus("*", 0);
    l1Tree_p->SetBranchStatus("jetEt", 1);
    l1Tree_p->SetBranchStatus("jetBx", 1);

    //handle dtree branches
    const Int_t DsizeMax = 50000;
    Int_t Dsize;
    Float_t Dpt[DsizeMax];
    Float_t Dmass[DsizeMax];
    Float_t Dy[DsizeMax];

    dTree_p->SetBranchStatus("*", 0);
    dTree_p->SetBranchStatus("Dsize", 1);
    dTree_p->SetBranchStatus("Dpt", 1);
    dTree_p->SetBranchStatus("Dmass", 1);
    dTree_p->SetBranchStatus("Dy", 1);

    dTree_p->SetBranchAddress("Dsize", &Dsize);
    dTree_p->SetBranchAddress("Dpt", Dpt);
    dTree_p->SetBranchAddress("Dmass", Dmass);
    dTree_p->SetBranchAddress("Dy", Dy);

    //handle trkTree branches
    std::vector<float>* trkPt_p = nullptr;
    std::vector<float>* trkEta_p = nullptr;
    std::vector<bool>* highPurity_p = nullptr;

    trkTree_p->SetBranchStatus("*", 0);
    trkTree_p->SetBranchStatus("trkPt", 1);
    trkTree_p->SetBranchStatus("trkEta", 1);
    trkTree_p->SetBranchStatus("highPurity", 1);

    trkTree_p->SetBranchAddress("trkPt", &trkPt_p);
    trkTree_p->SetBranchAddress("trkEta", &trkEta_p);
    trkTree_p->SetBranchAddress("highPurity", &highPurity_p);
    
    //Process entries in file
    const ULong64_t nEntries = l1Tree_p->GetEntries();
    for(ULong64_t entry = 0; entry < nEntries; ++entry){
      if(currEntry%nPrint == 0) std::cout << "  Entry " << currEntry << "/" << totalEntries << std::endl;
      ++currEntry;

      pfTree_p->GetEntry(entry);
      hltTree_p->GetEntry(entry);      
      dTree_p->GetEntry(entry);
      trkTree_p->GetEntry(entry);      
      l1Reader.Next();

      bool goodEvent = HLT_HIUPC_ZDC1nOR_MinPixelCluster400_MaxPixelCluster10000_v8;
      goodEvent = goodEvent || HLT_HIUPC_ZDC1nOR_SinglePixelTrackLowPt_MaxPixelCluster400_v8;
      goodEvent = goodEvent || HLT_HIUPC_ZDC1nOR_SinglePixelTrack_MaxPixelTrack_v8;
      
      if(!goodEvent) continue;

      etr->calcRapidityGapsInput(pfE_p, pfPt_p, pfEta_p, pfId_p);
      bool rapidityGapPlus = etr->rapidityGapPlus();
      bool rapidityGapMinus = etr->rapidityGapMinus();
      if(!rapidityGapPlus && !rapidityGapMinus) continue;
      
      //Find the leading dpt
      Float_t leadingDPt = -1.0;      
      for(Int_t dI = 0; dI < Dsize; ++dI){
	//Apply pt cuts
	if(Dpt[dI] < leadingDPt) continue;
	if(Dpt[dI] < ptBins[0]) continue;
	if(Dpt[dI] >= ptBins[nPtBins]) continue;

	//Apply y cut
	if(Dy[dI] > 2.0) continue;
	if(Dy[dI] < -2.0) continue;

	//Minimal mass cut
	if(Dmass[dI] > 2.05) continue;
	if(Dmass[dI] < 1.6) continue;
	
	leadingDPt = Dpt[dI];
      }

      //Find the leading l1 jet pT
      Float_t leadingJetEt = -1.0;
      for(unsigned int lI = 0; lI < (*jetEt).size(); ++lI){
	if(leadingJetEt > (*jetEt)[lI]) continue;
	if((*jetBx)[lI] != 0) continue;

	leadingJetEt = (*jetEt)[lI];
      }

      Int_t nTrkHP = 0;
      for(unsigned int tI = 0; tI < trkPt_p->size(); ++tI){
	if(trkPt_p->at(tI) < trkPtMin) continue;
	if(TMath::Abs(trkEta_p->at(tI)) > trkAbsEtaMax) continue;
	if(!highPurity_p->at(tI)) continue;

	++nTrkHP;
      }

      Int_t nTrkHPForFill = nTrkHP;
      if(nTrkHP >= nTrkMax) nTrkHPForFill = nTrkMax - 1;
      
      //fill histograms
      if(leadingDPt > 0.0){
	dPtDenom_h->Fill(leadingDPt);
	nTrkWithDDenom_h->Fill(nTrkHPForFill);

	for(Int_t jI = 0; jI < nJtPtThresh; ++jI){
	  if(leadingJetEt >= jtPtThresh[jI]){
	    dPtNum_h[jI]->Fill(leadingDPt);
	    nTrkWithDNum_h[jI]->Fill(nTrkHPForFill);
	  }
	}
      }
    }
    
    inFile_p->Close();
    delete inFile_p;
  }

  //Clean and close
  outFile_p->cd();

  dPtDenom_h->Write("", TObject::kOverwrite);
  for(Int_t jI = 0; jI < nJtPtThresh; ++jI){
    dPtNum_h[jI]->Write("", TObject::kOverwrite);
  }

  nTrkWithDDenom_h->Write("", TObject::kOverwrite);
  for(Int_t jI = 0; jI < nJtPtThresh; ++jI){
    nTrkWithDNum_h[jI]->Write("", TObject::kOverwrite);
  }

  config_p->Write("config", TObject::kOverwrite); 
  delete config_p;
  
  outFile_p->Close();
  delete outFile_p;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2 && argc != 3){
    std::cout << "Usage: ./bin/dTurnOn.exe <inFileName> <saveTag-optional='NoTag'>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 2) retVal += dTurnOn(argv[1]);
  else if(argc == 3) retVal += dTurnOn(argv[1], argv[2]);
  return retVal;
}
