#include <unistd.h>
#include <stdint.h>
#include <climits>
#include "../../../Libraries/Timer.h"
#include "RequestProcessor.h"
#include "ClRunner.h"
#include "ProcessorParameters.h"

void RequestProcessor::StartProcessing() {
    mainTimer.Start();
    for (int i = 0; i < parameters.kernelParameters.arrayCounter; ++i) {
        auto request = requestFetcher();
        DispatchRequest(request);
    }
    mainTimer.Stop();
}

RequestProcessor::RequestProcessor(ProcessorParameters parameters, RequestFetcher fetcher)
        : runners (new ClRunner[parameters.numberOfRunners]),
          statuses (new bool[parameters.numberOfRunners]),
          parameters(parameters),
          requestFetcher(fetcher),
          reqMedianTimer(Timer(Timer::Mode::Median)),
          mainTimer(Timer(Timer::Mode::Single))
{
    for (int i = 0; i < parameters.numberOfRunners; ++i) {
        runners[i] = ClRunner(parameters.environment.context, parameters.kernelParameters);
        statuses[i] = true;
    }
}

struct UserDataForCallback {
    int runnerIndex;
    bool* statuses;
    Timer* medianTimer;
    Timer reqTimer;

    UserDataForCallback() : reqTimer(Timer(Timer::Mode::Single)){};
};

bool RequestProcessor::IsArraySorted(const void *user_data) {
    auto isSorted = true;
    auto previous = INT_MIN;
    for (int j = 0; j < ((RunnerArgs*)user_data)->ElementsNumber; ++j) {
        isSorted = isSorted && (previous < ((int*)((RunnerArgs*)user_data)->OutputData)[j]);
        previous = ((int*)((RunnerArgs*)user_data)->OutputData)[j];
    }
    return isSorted;
}

void RequestProcessor::DispatchRequest(int *request) {
    auto user_data = new UserDataForCallback;
    user_data->statuses = statuses;
    user_data->medianTimer = &reqMedianTimer;
    user_data->reqTimer.Start();
    int selectedRunnerIndex = getAvailableRunner();
    user_data->runnerIndex = selectedRunnerIndex;
    ClRunner& selectedRunner = runners[selectedRunnerIndex];
    selectedRunner.WriteTo(request, parameters.kernelParameters.arraySize);
    selectedRunner(parameters.environment.kernel, [](cl_event e, cl_int i, void* user_data) {
        ((UserDataForCallback*)(((RunnerArgs*)user_data)->PassedData))->reqTimer.Stop();
        ((UserDataForCallback*)(((RunnerArgs*)user_data)->PassedData))->statuses[((UserDataForCallback*)(((RunnerArgs*)user_data)->PassedData))->runnerIndex] = true;
        ((UserDataForCallback*)(((RunnerArgs*)user_data)->PassedData))->medianTimer->PushTime(((UserDataForCallback*)(((RunnerArgs*)user_data)->PassedData))->reqTimer.Get());
        _mm_free(((RunnerArgs*)user_data)->OutputData);
        delete (((RunnerArgs*)user_data)->PassedData);
    }, user_data);
}


int RequestProcessor::getAvailableRunner() const {
    int selected = -1;
    while(selected == -1) {
        selected = checkForAvailability();
    }
    return selected;
}

int RequestProcessor::checkForAvailability() const {
    for (int i = 0; i < parameters.numberOfRunners; ++i) {
        if(statuses[i]) {
            statuses[i] = false;
            return i;
        }
    }
    return -1;
}

RequestProcessor::~RequestProcessor() {
    delete [] runners;
    printf("OpenCL,%d,%d,%lu,%lu",parameters.numberOfRunners, parameters.kernelParameters.arraySize, reqMedianTimer.Get(), reqMedianTimer.GetAvg());
    delete [] statuses;
}
