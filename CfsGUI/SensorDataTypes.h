#pragma once

struct SensorDataRecord
{
    double timestamp_ms;
    double Fx, Fy, Fz, Mx, My, Mz;
};

typedef void (CALLBACK* FUNC_Initialize)();
typedef void (CALLBACK* FUNC_Finalize)();
typedef bool (CALLBACK* FUNC_PortOpen)(int);
typedef void (CALLBACK* FUNC_PortClose)(int);
typedef bool (CALLBACK* FUNC_SetSerialMode)(int, bool);
typedef bool (CALLBACK* FUNC_GetSerialData)(int, double*, char*);
typedef bool (CALLBACK* FUNC_GetLatestData)(int, double*, char*);
typedef bool (CALLBACK* FUNC_GetSensorLimit)(int, double*);
typedef bool (CALLBACK* FUNC_GetSensorInfo)(int, char*);

#define WM_THREAD_FINISHED (WM_APP + 1)