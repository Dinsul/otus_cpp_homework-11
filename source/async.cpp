#include "async.h"
#include "mtbulk.h"

#include <vector>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <algorithm>


void split(BulkController &ctrl, std::string &str, char separator)
{
    auto start = str.begin();
    auto stop  = str.begin();

    do
    {
        if (*stop == separator){
            start = ++stop;
        }
        else {
            start = stop;
        }

        stop = std::find(start, str.end(), separator);

        if (stop != str.end())
        {
            ctrl.addString(std::string(start, stop));
        }

    }
    while(stop != str.end());

    str = std::string(start, str.end());
}


namespace async {

struct handle
{
    boost::asio::io_service service;
    MTWorker wrk;
    BulkController ctrl;

    std::string buffer;

    handle(std::size_t commandsCount) : service(), wrk(), ctrl(commandsCount, wrk), buffer(){}
};

void receive(handle_t handle, const char *data, std::size_t size)
{
    auto context  = static_cast<async::handle *>(handle);

    context->buffer.append(std::string(data, size));

    context->service.reset();
    context->service.post(boost::bind(&split, std::ref(context->ctrl), std::ref(context->buffer), '\n'));

    if (!size && !context->buffer.empty()){
        context->ctrl.addString(context->buffer);
    }
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
    receive(handle, "", 0);

    delete context;
}

}
