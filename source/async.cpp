#include "async.h"
#include "mtbulk.h"

#include <vector>
#include <iostream>
#include <boost/asio.hpp>


std::vector<std::string> split(const std::string &str, char separator)
{
    std::vector<std::string> retValue;

    std::string::size_type start = 0;
    std::string::size_type stop  = str.find_first_of(separator);

    while(stop != std::string::npos)
    {
        retValue.push_back(str.substr(start, stop - start));

        start = stop + 1;
        stop  = str.find_first_of(separator, start);
    }

    retValue.push_back(str.substr(start));

    return retValue;
}


namespace async {

struct handle
{
    boost::asio::io_service service;
    MTWorker wrk;
    BulkController ctrl;

    handle(std::size_t commandsCount) : service(), wrk(), ctrl(commandsCount, wrk){}
};

void receive(handle_t handle, const char *data, std::size_t size)
{
    auto context  = static_cast<async::handle *>(handle);

//    while (!context->service.stopped()) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(50));
//    }

    context->service.reset();

    context->service.post([&](){
        auto commands = split(std::string(data, size), '\n');

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for(const auto &cmd : commands)
        {
            context->ctrl.addString(cmd);
        }
    });

    context->service.run();
}

handle_t connect(std::size_t bulk)
{
    auto retVal = new handle(bulk);


    return static_cast<handle_t>(retVal);
}

void disconnect(handle_t handle)
{
    auto context  = static_cast<async::handle *>(handle);

    delete context;
}

}
