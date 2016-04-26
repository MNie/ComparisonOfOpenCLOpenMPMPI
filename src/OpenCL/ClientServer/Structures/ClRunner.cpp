#include "ClRunner.h"

ClRunner::ClRunner(cl::Context Context, KernelParameters Parameters)
        : Context(Context), Parameters(Parameters) {

    cl_int error;
    this->CommandQueue = cl::CommandQueue(this->Context, 0, &error);
    if(error != CL_SUCCESS) printf("Error at creating command queue : %d\n", error);

    LocalMemoryBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, 2*Parameters.arraySize * sizeof(int), NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating local memory buffer : %d\n", error);

    InBuffer = cl::Buffer(Context, CL_MEM_READ_ONLY, sizeof(KernelParameters), NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating output buffer : %d\n", error);

    const auto sizeOfOutput = Parameters.arraySize * sizeof(int);
    OutBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeOfOutput, NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating output buffer : %d\n", error);

    CommandQueue.enqueueWriteBuffer(InBuffer, CL_FALSE, 0, sizeof(KernelParameters), &Parameters, NULL, &previousCommandEvent);
}

ClRunner::~ClRunner() {
    CommandQueue.finish();
}

void ClRunner::operator()(cl::make_kernel<cl::Buffer &, cl::Buffer &, cl::Buffer &> Kernel, void (CL_CALLBACK *pfn_notify)(cl_event event, cl_int command_exec_status, void * user_data), void *pVoid) {
    auto output = (int*)_mm_malloc(sizeof(int) * Parameters.arraySize, ALLOC_ALIGN);
    auto callbackArgs = new RunnerArgs();
    callbackArgs->PassedData = pVoid;
    callbackArgs->that = this;
    callbackArgs->Callback = pfn_notify;
    callbackArgs->OutputData = output;
    callbackArgs->CommandQueue = &CommandQueue;
    callbackArgs->OutputBuffer = &OutBuffer;
    callbackArgs->ElementsNumber = Parameters.arraySize;
    cl::Event event;
    event = Kernel(cl::EnqueueArgs(CommandQueue, previousCommandEvent, cl::NDRange(1), cl::NDRange(1)), LocalMemoryBuffer,  InBuffer, OutBuffer);

    event.setCallback(CL_SUCCESS, [](cl_event event_in, cl_int command_exec_status, void * user_data) {
        cl::Event event;
        ((RunnerArgs*)user_data)->CommandQueue->enqueueReadBuffer(*(((RunnerArgs*)user_data)->OutputBuffer), CL_FALSE, 0, (((RunnerArgs*)user_data)->ElementsNumber)*sizeof(int), ((RunnerArgs*)user_data)->OutputData, NULL, &event);
        event.setCallback(CL_SUCCESS, [](cl_event event, cl_int command_exec_status, void * user_data) {
            ((RunnerArgs*)user_data)->Callback(event, command_exec_status, user_data);

            delete user_data;
        }, user_data);
    }, callbackArgs);
}

void ClRunner::SetParameters(KernelParameters parameters) {

}

void ClRunner::WriteTo(int *area, int count) {

    cl::Event event;
    CommandQueue.enqueueWriteBuffer(OutBuffer, CL_TRUE, 0, count*sizeof(int), area, nullptr, &previousCommandEvent);
}

void ClRunner::ReadTo(int *area) {
    cl::Event event;
    CommandQueue.enqueueReadBuffer(OutBuffer, CL_FALSE, 0, Parameters.arraySize*sizeof(int), area, nullptr, &event);
    event.setCallback(CL_SUCCESS, [](cl_event event, cl_int command_exec_status, void * user_data) {
    }, area);
}

ClRunner::ClRunner() {

}
