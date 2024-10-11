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

int dTurnOnPlot(std::vector<std::string> inFileNames)
{
  //Create histograms and output file
  TFile* inFile_p = new TFile(inFileNames[0].c_str(), "READ");
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

  const Double_t trkPtMin = config_p->GetValue("TRKPTMIN", -1.0); 
  const Double_t trkAbsEtaMax = config_p->GetValue("TRKABSETAMAX", -1001.0);
  
  if(trkPtMin < 0.0){
    std::cout << "TrkPtMin value invalid. check. return 1" << std::endl;
    return 1;
  }
  if(trkAbsEtaMax < -1000.0){
    std::cout << "TrkAbsEtaMax value invalid. check. return 1" << std::endl;
    return 1;
  }
  
  
  inFile_p->Close();
  delete inFile_p;

  std::vector<std::string> savetagPerFile;
  std::vector<std::vector<TGraphAsymmErrors*> > graphPerFile_p;
  for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
    graphPerFile_p.push_back({});
  }
  //Define xlow, xhigh for the turnons to be used globally
  Double_t xLow = 100000;
  Double_t xHigh = -1.0;

  //define some colors and markers
  std::vector<Style_t> markerlist_open = {24, 25, 26, 27, 28, 30, 32, 42, 46, 44};
  std::vector<Color_t> colorlist_middle = {kGreen+2, kRed-3, kAzure-3, kOrange-3, kMagenta-5, kCyan-2, kYellow+2, kBlue-5, kPink+2, kViolet+7};
    
  //define some sizes
  int font = 42;
  double size = 0.03;
  
  for(unsigned int fI = 0; fI < inFileNames.size(); ++fI){
    TFile* inFile_p = new TFile(inFileNames[fI].c_str(), "READ");
    TEnv* config_p = (TEnv*)inFile_p->Get("config");
    
    std::string saveTag = config_p->GetValue("SAVETAG", "");
    savetagPerFile.push_back(saveTag);
    
    if(saveTag.size() == 0){
      std::cout << "Given infile doesnt have a SAVETAG in config - check. return 1" << std::endl;
    }

    //Grab histograms
    TH1D* denom_p = (TH1D*)inFile_p->Get("dPtDenom_h");
    std::vector<TH1D*> num_p;
    TH1D* denomTrk_p = (TH1D*)inFile_p->Get("nTrkWithDDenom_h");
    std::vector<TH1D*> numTrk_p;
    std::vector<TGraphAsymmErrors*> graph_p;
    
    std::cout << "Producing turn-ons for nJtPtThresh: " << nJtPtThresh << std::endl;
    for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
      std::cout << " " << jI << ": " << jtPtThresh[jI] << std::endl;
      
      std::string histStr = Form("dPtNum_Jt%d_h", (Int_t)jtPtThresh[jI]);
      num_p.push_back((TH1D*)inFile_p->Get(histStr.c_str()));
      
      histStr = Form("nTrkWithDNum_Jt%d_h", (Int_t)jtPtThresh[jI]);
      numTrk_p.push_back((TH1D*)inFile_p->Get(histStr.c_str()));
    }
    
    //D pT turnons
    {
      //Create canvas and TGraphs
      TCanvas* canv_p = new TCanvas("canv", "canv", 900, 900);
      canv_p->SetTopMargin(0.03);
      canv_p->SetRightMargin(0.03);
      canv_p->SetBottomMargin(0.14);
      canv_p->SetLeftMargin(0.14);
      
      xLow = denom_p->GetXaxis()->GetBinLowEdge(1);
      xHigh = denom_p->GetXaxis()->GetBinLowEdge(denom_p->GetXaxis()->GetNbins()+1);
      
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
      label_p->SetTextFont(font);
      label_p->SetTextSize(size);
      
      const Float_t xPos = 0.6;
      const Float_t yPos = 0.5;
      label_p->DrawLatex(xPos, yPos+0.05, saveTag.c_str());
    
      TLegend* leg_p = new TLegend(xPos, yPos, xPos+0.3, yPos - 0.05*(Double_t)nJtPtThresh);
      leg_p->SetBorderSize(0);
      leg_p->SetFillColor(0);
      leg_p->SetFillStyle(0);
      
      for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
	graph_p.push_back(new TGraphAsymmErrors());
	graphPerFile_p[jI].push_back(new TGraphAsymmErrors());
      
	graph_p[jI]->BayesDivide(num_p[jI], denom_p);
	graphPerFile_p[jI][fI]->BayesDivide(num_p[jI], denom_p);
	
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
      delete label_p;
      
      delete temp_p;
      for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
	delete graph_p[jI];
      }
    }

    //nTrk overlay with D
    {
      //Create canvas and TGraphs
      TCanvas* canv_p = new TCanvas("canv", "canv", 900, 900);
      canv_p->SetTopMargin(0.03);
      canv_p->SetRightMargin(0.03);
      canv_p->SetBottomMargin(0.14);
      canv_p->SetLeftMargin(0.14);
      
      denomTrk_p->SetLineWidth(1.5);
      denomTrk_p->SetLineColor(1);    
      denomTrk_p->DrawCopy("HIST E1");
      
      TLine* line_p = new TLine();
      line_p->SetLineColor(kRed);
      line_p->SetLineStyle(2);
      line_p->DrawLine(30, 0.0, 30, denomTrk_p->GetMaximum()*0.5);
      line_p->DrawLine(100, 0.0, 100, denomTrk_p->GetMaximum()*0.5);
      delete line_p;
      
      TLatex* label_p = new TLatex();
      label_p->SetNDC();
      label_p->SetTextFont(font);
      label_p->SetTextSize(size);
      
      const Float_t xPos = 0.25;
      const Float_t xPos2 = 0.65;
      const Float_t yPos = 0.8;
      label_p->DrawLatex(xPos, yPos+0.05, saveTag.c_str());
      label_p->DrawLatex(xPos2, yPos, "High Purity");
      label_p->DrawLatex(xPos2, yPos-0.05, Form("Trk p_{T} > %.1f", trkPtMin));
      label_p->DrawLatex(xPos2, yPos-0.1, Form("Trk |#eta| < %.1f", trkAbsEtaMax));
      
      TLegend* leg_p = new TLegend(xPos, yPos, xPos+0.3, yPos - 0.05*(Double_t)(nJtPtThresh+1));
      leg_p->SetBorderSize(0);
      leg_p->SetFillColor(0);
      leg_p->SetFillStyle(0);
      
      leg_p->AddEntry(denomTrk_p, "All D", "L");
      
      for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
	numTrk_p[jI]->SetMarkerColor(colorlist_middle[jI]);
	numTrk_p[jI]->SetLineColor(colorlist_middle[jI]);
	numTrk_p[jI]->SetMarkerStyle(markerlist_open[jI]);
	numTrk_p[jI]->SetMarkerSize(1.5);
	
	std::string legStr = Form("L1 Jet E_{T} >= %d GeV", (Int_t)jtPtThresh[jI]);
	leg_p->AddEntry(numTrk_p[jI], legStr.c_str(), "P L");
	numTrk_p[jI]->DrawCopy("HIST E1 P SAME");
      }
      
      leg_p->Draw("SAME");
      
      gStyle->SetOptStat(0);
      gPad->SetTicks();
      
      gSystem->mkdir("pdfDir");
      std::string saveName = "pdfDir/nTrkWithD_" + saveTag + ".pdf";
      canv_p->SaveAs(saveName.c_str());
      delete canv_p;
      
      delete leg_p;    
      delete label_p;
    }

    inFile_p->Close();
    delete inFile_p;    
  }

  //Turn-on per jt pt threshold
  for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
    TCanvas* canv_p = new TCanvas("canv", "canv", 900, 900);
    canv_p->SetTopMargin(0.03);
    canv_p->SetRightMargin(0.03);
    canv_p->SetBottomMargin(0.14);
    canv_p->SetLeftMargin(0.14);

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
    label_p->SetTextFont(font);
    label_p->SetTextSize(size);
    
    const Float_t xPos = 0.6;
    const Float_t yPos = 0.5;
    std::string labelStr = Form("L1 Jet E_{T} >= %d GeV", (Int_t)jtPtThresh[jI]);
    label_p->DrawLatex(xPos, yPos+0.05, labelStr.c_str());
    
    TLegend* leg_p = new TLegend(xPos, yPos, xPos+0.3, yPos - 0.05*(Double_t)nJtPtThresh);
    leg_p->SetBorderSize(0);
    leg_p->SetFillColor(0);
    leg_p->SetFillStyle(0);
    
    for(unsigned int fI = 0; fI < inFileNames.size(); ++fI){
      graphPerFile_p[jI][fI]->SetMarkerColor(colorlist_middle[fI]);
      graphPerFile_p[jI][fI]->SetLineColor(colorlist_middle[fI]);
      graphPerFile_p[jI][fI]->SetMarkerStyle(markerlist_open[fI]);
      graphPerFile_p[jI][fI]->SetMarkerSize(1.5);
      
      leg_p->AddEntry(graphPerFile_p[jI][fI], savetagPerFile[fI].c_str(), "P L");

      std::cout << "Printing jI, fI: " << jI << ", " << fI << std::endl;
      graphPerFile_p[jI][fI]->Print("ALL");
      
      graphPerFile_p[jI][fI]->Draw("P");
    }
    
    leg_p->Draw("SAME");
    
    gStyle->SetOptStat(0);
    gPad->SetTicks();
    
    gSystem->mkdir("pdfDir");
    
    std::string saveName = Form("pdfDir/dTurnOn_JtPtThresh%.1f", jtPtThresh[jI]);
    while(saveName.find(".") != std::string::npos){saveName.replace(saveName.find("."), 1, "p");}
    saveName = saveName + ".pdf";
    canv_p->SaveAs(saveName.c_str());
    delete canv_p;

    delete leg_p;
    delete label_p;
    
    delete temp_p;
  }

  
  //Cleanup graphs
  for(unsigned int jI = 0; jI < jtPtThresh.size(); ++jI){
    for(unsigned int fI = 0; fI < inFileNames.size(); ++fI){
      delete graphPerFile_p[jI][fI];
    }
  }
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc < 2){
    std::cout << "Usage: ./bin/dTurnOnPlot.exe <inFileName1> <inFileName2>.... etc. return 1" << std::endl;
    return 1;
  }

  std::vector<std::string> fileList;
  for(int aI = 1; aI < argc; ++aI){
    fileList.push_back(argv[aI]);
  }
  
  int retVal = 0;
  retVal += dTurnOnPlot(fileList);
  return retVal;
}
