#ifndef DIVIDEANDCONQUER_REQUESTFETCHER_H
#define DIVIDEANDCONQUER_REQUESTFETCHER_H


class RequestFetcher {
    int requestSize;
public:
    RequestFetcher(int requestSize);

    int* operator()();
};


#endif //DIVIDEANDCONQUER_REQUESTFETCHER_H
