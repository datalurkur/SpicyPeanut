#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

enum RemoteCommand
{
    SuspendSampling = 0,
    ResumeSampling,
    CalibratePHMid,
    CalibratePHLow,
    CalibratePHHigh,
    CalibrateECDry,
    CalibrateECLow,
    CalibrateECHigh,
    ResetProbes,
    ReadPH,
    ReadEC
};

const unsigned int kRemoteCommandPort = 5009;
const unsigned int kRemoteCommandSize = 1;

#endif
