//*********************************************************************************
// Generated by SmartRF Studio version 2.13.1 (build#180)
// The applied template is compatible with CC13x0 SDK version 2.10.xx.xx or newer.
// Device: CC1310 Rev. B (2.1)
//
//*********************************************************************************


//*********************************************************************************
// Parameter summary
// RX Address0: 0xAA
// RX Address1: 0xBB
// RX Address Mode: No address check
// Frequency: 868.00000 MHz
// Data Format: Serial mode disable 
// Deviation: 25.000 kHz
// Packet Length Config: Variable 
// Max Packet Length: 255 
// Packet Length: 20
// Packet Data: 255 
// RX Filter BW: 98 kHz
// Symbol Rate: 50.00000 kBaud
// Sync Word Length: 32 Bits 
// TX Power: 14 dBm (requires define CCFG_FORCE_VDDR_HH = 1 in ccfg.c, see CC13xx/CC26xx Technical Reference Manual)
// Whitening: No whitening 

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include <ti/drivers/rf/RF.h>
#include DeviceFamily_constructPath(rf_patches/rf_patch_cpe_genfsk.h)
#include DeviceFamily_constructPath(rf_patches/rf_patch_rfe_genfsk.h)
#include "smartrf_settings.h"


// TI-RTOS RF Mode Object
RF_Mode RF_prop =
{
    .rfMode = RF_MODE_PROPRIETARY_SUB_1,
    .cpePatchFxn = &rf_patch_cpe_genfsk,
    .mcePatchFxn = 0,
    .rfePatchFxn = &rf_patch_rfe_genfsk
};


// Overrides for CMD_PROP_RADIO_DIV_SETUP
uint32_t pOverrides[] =
{
    // override_use_patch_prop_genfsk.xml
    // PHY: Use MCE ROM bank 4, RFE RAM patch
    MCE_RFE_OVERRIDE(0,4,0,1,0,0),
    // override_synth_prop_863_930_div5.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x0A480583,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x7AB80603,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_phy_rx_aaf_bw_0xd.xml
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_agc_reflevel_0x1a.xml
    // Tx: Configure PA ramping setting (0x41). Rx: Set AGC reference level to 0x1A.
    HW_REG_OVERRIDE(0x6088,0x411A),
    // Tx: Configure PA ramping setting
    HW_REG_OVERRIDE(0x608C,0x8213),
    // override_phy_rx_rssi_offset_5db.xml
    // Rx: Set RSSI offset to adjust reported RSSI by +5 dB (default: 0), trimmed for external bias and differential configuration
    (uint32_t)0x00FB88A3,
#if (CCFG_FORCE_VDDR_HH)
    // TX power override
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
#endif
    (uint32_t)0xFFFFFFFF,
};


// CMD_NOP
// No Operation Command
rfc_CMD_NOP_t RF_cmdNop =
{
     .commandNo                = CMD_NOP,
     .status                   = 0x0000,
     .pNextOp                  = 0, // Set this to (uint8_t*)&RF_cmdPropCs in the application
     .startTime                = 0x00000000,
     .startTrigger.triggerType = TRIG_ABSTIME, // Trigs at an absolute time
     .startTrigger.bEnaCmd     = 0x0,
     .startTrigger.triggerNo   = 0x0,
     .startTrigger.pastTrig    = 0x0,
     .condition.rule           = COND_ALWAYS, // Always run next command (except in case of Abort)
     .condition.nSkip          = 0x0,
};


// CMD_PROP_RADIO_DIV_SETUP
// Proprietary Mode Radio Setup Command for All Frequency Bands
rfc_CMD_PROP_RADIO_DIV_SETUP_t RF_cmdPropRadioDivSetup =
{
    .commandNo = 0x3807,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .modulation.modType = 0x1,
    .modulation.deviation = 0x64,
    .symbolRate.preScale = 0xF,
    .symbolRate.rateWord = 0x8000,
    .symbolRate.decimMode = 0x0,
    .rxBw = 0x24,
    .preamConf.nPreamBytes = 0x4,
    .preamConf.preamMode = 0x0,
    .formatConf.nSwBits = 0x20,
    .formatConf.bBitReversal = 0x0,
    .formatConf.bMsbFirst = 0x1,
    .formatConf.fecMode = 0x0,
    .formatConf.whitenMode = 0x0,
    .config.frontEndMode = 0x0,
    .config.biasMode = 0x1,
    .config.analogCfgMode = 0x0,
    .config.bNoFsPowerUp = 0x0,
    .txPower = 0xA73F,
    .pRegOverride = pOverrides,
    .centerFreq = 0x0364,
    .intFreq = 0x8000,
    .loDivider = 0x05
};


// CMD_FS
// Frequency Synthesizer Programming Command
rfc_CMD_FS_t RF_cmdFs =
{
    .commandNo = 0x0803,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .frequency = 0x0364,
    .fractFreq = 0x0000,
    .synthConf.bTxMode = 0x0,
    .synthConf.refFreq = 0x0,
    .__dummy0 = 0x00,
    .__dummy1 = 0x00,
    .__dummy2 = 0x00,
    .__dummy3 = 0x0000
};


// CMD_PROP_TX
// Proprietary Mode Transmit Command
rfc_CMD_PROP_TX_t RF_cmdPropTx =
{
    .commandNo = 0x3801,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bVarLen = 0x1,
    .pktLen = 0x1E,
    .syncWord = 0x930B51DE,
    .pPkt = 0 // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
};


// CMD_PROP_RX
// Proprietary Mode Receive Command
rfc_CMD_PROP_RX_t RF_cmdPropRx =
{
    .commandNo = 0x3802,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bRepeatOk = 0x0,
    .pktConf.bRepeatNok = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bVarLen = 0x1,
    .pktConf.bChkAddress = 0x0,
    .pktConf.endType = 0x0,
    .pktConf.filterOp = 0x0,
    .rxConf.bAutoFlushIgnored = 0x0,
    .rxConf.bAutoFlushCrcErr = 0x0,
    .rxConf.bIncludeHdr = 0x1,
    .rxConf.bIncludeCrc = 0x0,
    .rxConf.bAppendRssi = 0x0,
    .rxConf.bAppendTimestamp = 0x0,
    .rxConf.bAppendStatus = 0x1,
    .syncWord = 0x930B51DE,
    .maxPktLen = 0xFF,
    .address0 = 0xAA,
    .address1 = 0xBB,
    .endTrigger.triggerType = 0x1,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 0x00000000,
    .pQueue = 0, // INSERT APPLICABLE POINTER: (dataQueue_t*)&xxx
    .pOutput = 0 // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
};


// CMD_PROP_CS
// Carrier Sense Command
rfc_CMD_PROP_CS_t RF_cmdPropCs =
{
    .commandNo                = CMD_PROP_CS,
    .status                   = 0x0000,
    .pNextOp                  = 0, // Set this to (uint8_t*)&RF_cmdCountBranch in the application
    .startTime                = 0x00000000,
    .startTrigger.triggerType = TRIG_NOW,
    .startTrigger.bEnaCmd     = 0x0,
    .startTrigger.triggerNo   = 0x0,
    .startTrigger.pastTrig    = 0x0,
    .condition.rule           = COND_SKIP_ON_FALSE, // Run next command if this command returned TRUE,
                                                    // skip a number of commands (.condition.nSkip - 1) if it returned FALSE
                                                    // End causes for the CMD_PROP_CS command:
                                                    // Observed channel state Busy with csConf.busyOp = 1:                            PROP_DONE_BUSY        TRUE
                                                    // 0bserved channel state Idle with csConf.idleOp = 1:                            PROP_DONE_IDLE        FALSE
                                                    // Timeout trigger observed with channel state Busy:                              PROP_DONE_BUSY        TRUE
                                                    // Timeout trigger observed with channel state Idle:                              PROP_DONE_IDLE        FALSE
                                                    // Timeout trigger observed with channel state Invalid and csConf.timeoutRes = 0: PROP_DONE_BUSYTIMEOUT TRUE
                                                    // Timeout trigger observed with channel state Invalid and csConf.timeoutRes = 1: PROP_DONE_IDLETIMEOUT FALSE
                                                    // Received CMD_STOP after command started:                                       PROP_DONE_STOPPED     FALSE
    .condition.nSkip          = 0x2, // Number of skips + 1 if the rule involves skipping. 0: Same, 1: Next, 2: Skip next
    .csFsConf.bFsOffIdle      = 0x0, // Keep synthesizer running if command ends with channel Idle
    .csFsConf.bFsOffBusy      = 0x0, // Keep synthesizer running if command ends with Busy
    .__dummy0                 = 0x00,
    .csConf.bEnaRssi          = 0x1, // Enable RSSI as a criterion
    .csConf.bEnaCorr          = 0x0, // Disable correlation (PQT) as a criterion
    .csConf.operation         = 0x0, // Busy if either RSSI or correlation indicates Busy
    .csConf.busyOp            = 0x1, // End carrier sense on channel Busy
    .csConf.idleOp            = 0x0, // Continue on channel Idle
    .csConf.timeoutRes        = 0x0, // Timeout with channel state Invalid treated as Busy
    .rssiThr                  = 0x0, // Set the RSSI threshold in the application
    .numRssiIdle              = 0x0, // Number of consecutive RSSI measurements - 1 below the threshold
                                     // needed before the channel is declared Idle
    .numRssiBusy              = 0x0, // Number of consecutive RSSI measurements -1 above the threshold
                                     // needed before the channel is declared Busy
    .corrPeriod               = 0x0000, // N/A since .csConf.bEnaCorr = 0
    .corrConfig.numCorrInv    = 0x0, // N/A since .csConf.bEnaCorr = 0
    .corrConfig.numCorrBusy   = 0x0, // N/A since .csConf.bEnaCorr = 0
    .csEndTrigger.triggerType = TRIG_REL_START, // Trigs at a time relative to the command started
    .csEndTrigger.bEnaCmd     = 0x0,
    .csEndTrigger.triggerNo   = 0x0,
    .csEndTrigger.pastTrig    = 0x0,
    .csEndTime                = 0x00000000, // Set the CS end time in the application
};


// CMD_COUNT_BRANCH
// Counter Command with Branch of Command Chain
rfc_CMD_COUNT_BRANCH_t RF_cmdCountBranch =
{
 .commandNo                = CMD_COUNT_BRANCH,
     .status                   = 0x0000,
     .pNextOp                  = 0, // Set this to (uint8_t*)&RF_cmdPropTx in the application
     .startTime                = 0x00000000,
     .startTrigger.triggerType = TRIG_NOW, // Triggers immediately
     .startTrigger.bEnaCmd     = 0x0,
     .startTrigger.triggerNo   = 0x0,
     .startTrigger.pastTrig    = 0x0,
     .condition.rule           = COND_STOP_ON_FALSE, // Run next command if this command returned TRUE, stop if it returned FALSE
                                                     // End causes for the CMD_COUNT_BRANCH command:
                                                     // Finished operation with counter = 0 when being started: DONE_OK         TRUE
                                                     // Finished operation with counter > 0 after decrementing: DONE_OK         TRUE
                                                     // Finished operation with counter = 0 after decrementing: DONE_COUNTDOWN  FALSE
     .condition.nSkip          = 0x0,
     .counter                  = 0, // On start, the radio CPU decrements the value, and the end status of the operation differs if the result is zero
                                    // This number is set in the application (CS_RETRIES_WHEN_BUSY) and determines how many times the CMD_PROP_CS should run
                                    // in the case where the channel i Busy
     .pNextOpIfOk              = 0, // Set this to (uint8_t*)&RF_cmdPropCs in the application
};

// CMD_TX_TEST
rfc_CMD_TX_TEST_t RF_cmdTxTest =
{
    .commandNo = 0x0808,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .config.bUseCw = 0x0,
    .config.bFsOff = 0x1,
    .config.whitenMode = 0x2,
    .__dummy0 = 0x00,
    .txWord = 0xABCD,
    .__dummy1 = 0x00,
    .endTrigger.triggerType = 0x1,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .syncWord = 0x930B51DE,
    .endTime = 0x00000000,
};

