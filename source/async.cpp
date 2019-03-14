#include "async.h"
#include "mtbulk.h"

#include <vector>
#include <iostream>
#include <boost/asio.hpp>
#include <algorithm>


std::vector<std::string> split(const std::string &str, char separator)
{
    std::vector<std::string> retValue;

    auto start = str.begin();
    decltype (str.begin()) stop;

    do
    {
        while (*start == separator) {
            ++start;
        }

        stop = std::find(start, str.end(), separator);

        auto subString = std::string(start, stop);

        if (!subString.empty())
        {
            retValue.push_back(subString);
        }

        start = stop;
    }
    while(stop != str.end());

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

    context->service.reset();

    context->service.post([&](){
        auto commands = split(std::string(data, size), '\n');

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
