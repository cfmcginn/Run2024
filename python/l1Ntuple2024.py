# L1Ntuple for ZDC studies with 2023 data
# Hannah Bossi, <hannah.bossi@cern.ch>
# 9/17/2023
import FWCore.ParameterSet.Config as cms
import os
import sys

from Configuration.Eras.Era_Run3_pp_on_PbPb_cff import Run3_pp_on_PbPb
process = cms.Process('RAW2DIGI', Run3_pp_on_PbPb)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.RawToDigi_Data_cff') # mapper for raw prime
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')


#Replace GT for running over 2024
#GT        = "132X_dataRun3_HLT_v2"
GT        = "141X_dataRun3_Prompt_Candidate_2024_10_08_09_42_50"
process.GlobalTag.globaltag = GT


# ==================================================================
# ==================== modification needed for 2023 data ===========
#We no longer want to grab the emap from txt file

#from CondCore.CondDB.CondDB_cfi import *
#process.es_pool = cms.ESSource("PoolDBESSource",
#    timetype = cms.string('runnumber'),
#    toGet = cms.VPSet(
#        cms.PSet(
#            record = cms.string("HcalElectronicsMapRcd"),
#            tag = cms.string("HcalElectronicsMap_2021_v2.0_data")
#        )
#    ),
#    connect = cms.string('frontier://FrontierProd/CMS_CONDITIONS'),
#        authenticationMethod = cms.untracked.uint32(1)
#    )
#
#process.es_prefer = cms.ESPrefer('HcalTextCalibrations', 'es_ascii')
#process.es_ascii = cms.ESSource(
#    'HcalTextCalibrations',
#    input = cms.VPSet(
#        cms.PSet(
#
#            object = cms.string('ElectronicsMap'),
#            #Replacing old emap file input
##            file = cms.FileInPath("emap_2023_newZDC_v3.txt")
#            file = cms.FileInPath("emap_2023_newZDC.txt")
#             )
#        )
#    )
# =======================================================================

# To change the number of events, change this part
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1000),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)

# Input source

# in case of dat files - read it like this
# replacing 2023 file reading with snippet below
#filedir = '/eos/cms/store/t0streamer/Data/PhysicsHIPhysicsRawPrime14/000/375/820'
#print(filedir)
#infile    = cms.untracked.vstring()
#for f in reversed(os.listdir(filedir)):
#    if f[-4:] == '.dat' :
#        infile.append('file:'+filedir+'/'+f)
#print(infile)
#
#
#process.source = cms.Source("NewEventStreamFileReader",
#                            fileNames = infile,
#)


process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        '/store/hidata/HIRun2023A/HIForward0/RAW/v1/000/375/697/00000/3cc42004-7d46-467d-89db-d04005b11227.root'
    )
)



process.options = cms.untracked.PSet(
#    FailPath = cms.untracked.vstring(),
    IgnoreCompletely = cms.untracked.vstring(),
    Rethrow = cms.untracked.vstring(),
#    SkipEvent = cms.untracked.vstring(),
    accelerators = cms.untracked.vstring('*'),
    allowUnscheduled = cms.obsolete.untracked.bool,
    canDeleteEarly = cms.untracked.vstring(),
    deleteNonConsumedUnscheduledModules = cms.untracked.bool(True),
    dumpOptions = cms.untracked.bool(False),
    emptyRunLumiMode = cms.obsolete.untracked.string,
    eventSetup = cms.untracked.PSet(
        forceNumberOfConcurrentIOVs = cms.untracked.PSet(
            allowAnyLabel_=cms.required.untracked.uint32
        ),
        numberOfConcurrentIOVs = cms.untracked.uint32(0)
    ),
    fileMode = cms.untracked.string('FULLMERGE'),
    forceEventSetupCacheClearOnNewRun = cms.untracked.bool(False),
    holdsReferencesToDeleteEarly = cms.untracked.VPSet(),
    makeTriggerResults = cms.obsolete.untracked.bool,
    modulesToIgnoreForDeleteEarly = cms.untracked.vstring(),
    numberOfConcurrentLuminosityBlocks = cms.untracked.uint32(0),
    numberOfConcurrentRuns = cms.untracked.uint32(1),
    numberOfStreams = cms.untracked.uint32(0),
    numberOfThreads = cms.untracked.uint32(1),
    printDependencies = cms.untracked.bool(False),
    sizeOfStackForThreadsInKB = cms.optional.untracked.uint32,
    throwIfIllegalParameter = cms.untracked.bool(True),
    wantSummary = cms.untracked.bool(False)
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('l1Ntuple nevts:100'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition
from Configuration.Applications.ConfigBuilder import MassReplaceInputTag

# Additional output definition

# Path and EndPath definitions
process.raw2digi_step = cms.Path(process.RawToDigi)
process.endjob_step = cms.EndPath(process.endOfProcess)

# Schedule definition
process.schedule = cms.Schedule(process.raw2digi_step, process.endjob_step)


from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

# customisation of the process.

# Automatic addition of the customisation function from L1Trigger.Configuration.customiseReEmul
from L1Trigger.Configuration.customiseReEmul import L1TReEmulFromRAW 

#call to customisation function L1TReEmulFromRAW imported from L1Trigger.Configuration.customiseReEmul
process = L1TReEmulFromRAW(process)

# Automatic addition of the customisation function from L1Trigger.L1TNtuples.customiseL1Ntuple
from L1Trigger.L1TNtuples.customiseL1Ntuple import L1NtupleRAWEMU 

#call to customisation function L1NtupleRAWEMU imported from L1Trigger.L1TNtuples.customiseL1Ntuple
process = L1NtupleRAWEMU(process)

# Automatic addition of the customisation function from L1Trigger.Configuration.customiseSettings
from L1Trigger.Configuration.customiseSettings import L1TSettingsToCaloParams_2018_v1_4_1 

#call to customisation function L1TSettingsToCaloParams_2018_v1_4_1 imported from L1Trigger.Configuration.customiseSettings
process = L1TSettingsToCaloParams_2018_v1_4_1(process)

# Automatic addition of the customisation function from L1Trigger.Configuration.customiseUtils
from L1Trigger.Configuration.customiseUtils import L1TGlobalMenuXML 

#call to customisation function L1TGlobalMenuXML imported from L1Trigger.Configuration.customiseUtils
process = L1TGlobalMenuXML(process)

# End of customisation functions


# Customisation from command line

# Add early deletion of temporary data products to reduce peak memory need
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
# End adding early deletion



# =====================================================================
# add in the zdc analyzer - this writes the digi information to a tree
process.load('HeavyIonsAnalysis.ZDCAnalysis.QWZDC2018Producer_cfi')
process.load('HeavyIonsAnalysis.ZDCAnalysis.QWZDC2018RecHit_cfi')
process.load('HeavyIonsAnalysis.ZDCAnalysis.zdcanalyzer_cfi')

process.zdcanalyzer.doZDCRecHit = False
process.zdcanalyzer.doZDCDigi = True
process.zdcanalyzer.zdcRecHitSrc = cms.InputTag("QWzdcreco")
process.zdcanalyzer.zdcDigiSrc = cms.InputTag("hcalDigis", "ZDC")
process.zdcanalyzer.calZDCDigi = False
process.zdcanalyzer.verbose = False
process.zdcanalyzer_step = cms.Path(process.zdcanalyzer)
process.schedule.append(process.zdcanalyzer_step)
#=======================================================================


# ======================================================================
# ======================== add in the emulator =========================
# unpacked etsums
process.l1UpgradeTree.sumZDCTag = cms.untracked.InputTag("gtStage2Digis","EtSumZDC") 
process.l1UpgradeTree.sumZDCToken = cms.untracked.InputTag("gtStage2Digis","EtSumZDC")

# l1 emulator sums
process.l1UpgradeEmuTree.sumZDCTag = cms.untracked.InputTag("etSumZdcProducer")
process.l1UpgradeEmuTree.sumZDCToken = cms.untracked.InputTag("etSumZdcProducer")

# do not change these settings
process.etSumZdcProducer = cms.EDProducer('L1TZDCProducer',
                                          hcalTPDigis = cms.InputTag("simHcalTriggerPrimitiveDigis"),
#                                          hcalTPDigis = cms.InputTag("hcalDigis"),
                                          bxFirst = cms.int32(-2),
                                          bxLast = cms.int32(3)
)

#Via Hannah, for the simHcal collection
process.simHcalTriggerPrimitiveDigis.inputLabel = cms.VInputTag("hcalDigis", "hcalDigis:ZDC")
process.simHcalTriggerPrimitiveDigis.inputUpgradeLabel = cms.VInputTag("hcalDigis", "hcalDigis:ZDC")


process.etSumZdc = cms.Path(process.etSumZdcProducer)
process.schedule.append(process.etSumZdc)
#======================================================================


# input tag replacement needed for raw prime data
#MassReplaceInputTag(process, new="rawPrimeDataRepacker", old="rawDataCollector")

#input tag needed for regular HI data
MassReplaceInputTag(process, new="rawDataRepacker", old="rawDataCollector")
