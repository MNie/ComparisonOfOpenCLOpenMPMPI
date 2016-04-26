#ifndef DIVIDEANDCONQUER_REQUESTPROCESSOR_H
#define DIVIDEANDCONQUER_REQUESTPROCESSOR_H

#include "KernelParameters.h"
#include "RequestFetcher.h"
#include "ProcessorParameters.h"
#include "ClRunner.h"
#include "../../../Libraries/Timer.h"

class RequestProcessor {
    ClRunner* runners;
    ProcessorParameters parameters;
    RequestFetcher requestFetcher;
    bool* statuses;
    Timer reqMedianTimer;
    Timer mainTimer;
public:
    RequestProcessor(ProcessorParameters parameters, RequestFetcher fetcher);


    virtual ~RequestProcessor();

    void StartProcessing();

    void DispatchRequest(int *request);

    int getAvailableRunner() const;

    int checkForAvailability() const;

    bool static IsArraySorted(const void *user_data);
};


#endif //DIVIDEANDCONQUER_REQUESTPROCESSOR_H
