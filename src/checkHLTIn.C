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

int checkHLTIn(std::string inFileName)
{
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

  std::vector<std::string> hltBranches;
  std::vector<Int_t> hltVal;
  std::map<std::string, Int_t> branchFireCounter;
  
  //pre-parse files for totalnEntries, convenient counting
  ULong64_t totalEntries = 0;
  std::cout << "Processing " << fileList.size() << " files..." << std::endl;
  for(unsigned fI = 0; fI < fileList.size(); ++fI){
    //    std::cout << " " << fI << "/" << fileList.size() << ": '" << fileList[fI] << "'" << std::endl;

    TFile* inFile_p = new TFile(fileList[fI].c_str(), "READ");
    TTree* hltTree_p = (TTree*)inFile_p->Get("hltanalysis/HltTree");

    if(fI == 0){
      TObjArray* listOfBranches = hltTree_p->GetListOfBranches();

      for(Int_t entry = 0; entry < listOfBranches->GetEntries(); ++entry){
	std::string branchName = listOfBranches->At(entry)->GetName();
	if(branchName.substr(0,4).find("HLT_") == std::string::npos) continue;
	if(branchName.find("Prescale") != std::string::npos) continue;

	hltBranches.push_back(branchName);
      }
    }
    
    totalEntries += hltTree_p->GetEntries();
    
    inFile_p->Close();
    delete inFile_p;
  }

  std::cout << "HLT BRANCHES (N=" << hltBranches.size() << "): " << std::endl;
  for(unsigned int hI = 0; hI < hltBranches.size(); ++hI){
    std::cout << " " << hI << ": " << hltBranches[hI] << std::endl;

    hltVal.push_back(0);
    branchFireCounter[hltBranches[hI]] = 0;
  }

  std::cout << " Corresponds to " << totalEntries << " events..." << std::endl;
  //Setup some cout params
  const ULong64_t nPrint = 10000;
  ULong64_t currEntry = 0;
  
  for(unsigned fI = 0; fI < fileList.size(); ++fI){
    //grab file, l1 and dtrees
    TFile* inFile_p = new TFile(fileList[fI].c_str(), "READ");
    TTree* hltTree_p = (TTree*)inFile_p->Get("hltanalysis/HltTree");

    hltTree_p->SetBranchStatus("*", 0);
    for(unsigned int hI = 0; hI < hltBranches.size(); ++hI){
      hltTree_p->SetBranchStatus(hltBranches[hI].c_str(), 1);
      hltTree_p->SetBranchAddress(hltBranches[hI].c_str(), &(hltVal[hI]));
    }
    
    //Process entries in file
    const ULong64_t nEntries = hltTree_p->GetEntries();
    for(ULong64_t entry = 0; entry < nEntries; ++entry){
      if(currEntry%nPrint == 0) std::cout << "  Entry " << currEntry << "/" << totalEntries << std::endl;

      hltTree_p->GetEntry(entry);      

      for(unsigned int hI = 0; hI < hltBranches.size(); ++hI){
	if(hltVal[hI] == 1) ++(branchFireCounter[hltBranches[hI]]);
      }
      
      ++currEntry;
    }
    
    inFile_p->Close();
    delete inFile_p;
  }

  std::cout << "Trigger analysis complete." << std::endl;

  for(unsigned int hI = 0; hI < hltBranches.size(); ++hI){
    std::cout << " " << hI << ": " << hltBranches[hI] << ", " << branchFireCounter[hltBranches[hI]] << "/" << totalEntries << ", " << ((Double_t)branchFireCounter[hltBranches[hI]])/((Double_t)totalEntries) << std::endl;
  }	  
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/checkHLTIn.exe <inFileName>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 2) retVal += checkHLTIn(argv[1]);
  return retVal;
}
