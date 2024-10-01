//c and cpp
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//root
#include "TCanvas.h"
#include "TColor.h"
#include "TEnv.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TPad.h"
#include "TStyle.h"
#include "TSystem.h"

int dTurnOnPlot(std::string inFileName)
{
  //Create histograms and output file
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  TEnv* config_p = (TEnv*)inFile_p->Get("config");

  Int_t nJtPtThresh = config_p->GetValue("NJTPTTHRESH", -1);
  if(nJtPtThresh < 0){
    std::cout << "Issue getting nJtPtThresh from config, less than 0 - check. return 1" << std::endl;
    return 1;
  }

  std::vector<double> jtPtThresh;
  for(Int_t jI = 0; jI < nJtPtThresh; ++jI){
    std::string valStr = "JTPTTHRESH." + std::to_string(jI);
    double tempPtThresh = config_p->GetValue(valStr.c_str(), -1.0);
    if(tempPtThresh < 0.0){
      std::cout << "Issue getting " << valStr << " from inFile config. return 1" << std::endl;
      return 1;
    }
    jtPtThresh.push_back(tempPtThresh);
  }

  std::string saveTag = config_p->GetValue("SAVETAG", "");
  if(saveTag.size() == 0){
    std::cout << "Given infile doesnt have a SAVETAG in config - check. return 1" << std::endl;
  }
  
  //Grab histograms
  TH1D* denom_p = (TH1D*)inFile_p->Get("dPtDenom_h");
  std::vector<TH1D*> num_p;
  std::vector<TGraphAsymmErrors*> graph_p;

  //define some colors and markers
  std::vector<Style_t> markerlist_open = {24, 25, 26, 27, 28, 30, 32, 42, 46, 44};
  std::vector<Color_t> colorlist_middle = {kGreen+2, kRed-3, kAzure-3, kOrange-3, kMagenta-5, kCyan-2, kYellow+2, kBlue-5, kPink+2, kViolet+7};
  
  std::cout << "Producing turn-ons for nJtPtThresh: " << nJtPtThresh << std::endl;
  for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
    std::cout << " " << jI << ": " << jtPtThresh[jI] << std::endl;

    std::string histStr = Form("dPtNum_Jt%d_h", (Int_t)jtPtThresh[jI]);
    num_p.push_back((TH1D*)inFile_p->Get(histStr.c_str()));
  }
  
  //Create canvas and TGraphs
  TCanvas* canv_p = new TCanvas("canv", "canv", 900, 900);
  canv_p->SetTopMargin(0.03);
  canv_p->SetRightMargin(0.03);
  canv_p->SetBottomMargin(0.14);
  canv_p->SetLeftMargin(0.14);

  Double_t xLow = denom_p->GetXaxis()->GetBinLowEdge(1);
  Double_t xHigh = denom_p->GetXaxis()->GetBinLowEdge(denom_p->GetXaxis()->GetNbins()+1);
  
  TH1D* temp_p = new TH1D("temp_h", ";D p_{T} (GeV);Efficiency", 1, xLow, xHigh);
  temp_p->SetMaximum(1.1);
  temp_p->SetMinimum(0.0);

  temp_p->DrawCopy();

  TLine* line_p = new TLine();
  line_p->SetLineStyle(2);
  line_p->DrawLine(xLow, 1.0, xHigh, 1.0);
  delete line_p;

  TLatex* label_p = new TLatex();
  label_p->SetNDC();

  const Float_t xPos = 0.6;
  const Float_t yPos = 0.5;
  label_p->DrawLatex(xPos, yPos+0.05, saveTag.c_str());
  
  TLegend* leg_p = new TLegend(xPos, yPos, xPos+0.3, yPos - 0.05*(Double_t)nJtPtThresh);
  leg_p->SetBorderSize(0);
  leg_p->SetFillColor(0);
  leg_p->SetFillStyle(0);
  
  for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
    graph_p.push_back(new TGraphAsymmErrors());

    graph_p[jI]->BayesDivide(num_p[jI], denom_p);

    graph_p[jI]->SetMarkerColor(colorlist_middle[jI]);
    graph_p[jI]->SetLineColor(colorlist_middle[jI]);
    graph_p[jI]->SetMarkerStyle(markerlist_open[jI]);
    graph_p[jI]->SetMarkerSize(1.5);

    std::string legStr = Form("L1 Jet E_{T} >= %d GeV", (Int_t)jtPtThresh[jI]);
    leg_p->AddEntry(graph_p[jI], legStr.c_str(), "P L");
    graph_p[jI]->Draw("P");
  }

  leg_p->Draw("SAME");
  
  gStyle->SetOptStat(0);
  gPad->SetTicks();

  gSystem->mkdir("pdfDir");
  std::string saveName = "pdfDir/dTurnOn_" + saveTag + ".pdf";
  canv_p->SaveAs(saveName.c_str());
  delete canv_p;

  delete leg_p;

  delete temp_p;
  for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
    delete graph_p[jI];
  }
  
  inFile_p->Close();
  delete inFile_p;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/dTurnOnPlot.exe <inFileName>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += dTurnOnPlot(argv[1]);
  return retVal;
}
