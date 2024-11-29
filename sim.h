#ifndef SIM_H
#define SIM_H

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

enum PipelineRegister {
    FE,
    DE,
    RN,
    RR,
    DI,
    IS,
    EX,
    WB,
    RT
};

struct CycleInfo {
    int start;
    int finish;
};

struct RenameMapElement
{
    int RegisterIndex;
    int RobValue;
    bool Valid;
};

#endif
