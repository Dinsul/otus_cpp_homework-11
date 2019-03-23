#include "bulkmt.h"

#include <iostream>
#include <fstream>

#include <stdexcept>
#include <functional>

inline bool fexists(const std::string& filename) {
  std::ifstream ifile(filename.c_str());
  return ifile.operator bool();
}

std::string BulkController::signShiftDown() const
{
    return _signShiftDown;
}

void BulkController::setSignShiftDown(const std::string &signShiftDown)
{
    _signShiftDown = signShiftDown;
}

void BulkController::waiteWorker()
{
    while (_bulksCounter != _worker.bulksPrinted())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::string BulkController::signShiftUp() const
{
    return _signShiftUp;
}

void BulkController::setSignShiftUp(const std::string &signShiftUp)
{
    _signShiftUp = signShiftUp;
}

void BulkController::sendBulk()
{
    if (!_bulk.empty())
    {
        _worker(_bulk);

        ++_bulksCounter;
        _bulk.clear();
        _currentNumber = 0;
    }
}

BulkController::BulkController(size_t bulkSize, ImpWorker &worker)
    : _bulk(), _worker(worker),
      _bulkSize(bulkSize), _currentNumber(0),
      _stackSize(0), _signShiftUp("{"), _signShiftDown("}"),
      _bulksCounter(0),
      _commandsCounter(0),
      _linesCounter(0)
{}

BulkController::~BulkController()
{
    sendBulk();

    waiteWorker();

    std::cout << "MainThread\n"
              << "\tCommands: " << _commandsCounter << "\n"
              << "\tLines:    " << _linesCounter << "\n"
              << "\tBulks:    " << _bulksCounter << std::endl;
}

void BulkController::addString(const std::string &str)
{
    ++_linesCounter;

    if (str == _signShiftUp)
    {
        if (_stackSize == 0)
        {
            sendBulk();
        }

        ++_stackSize;
    }
    else if (str == _signShiftDown)
    {
        if (_stackSize == 0)
        {
            throw std::out_of_range("stack breakdown below");
        }

        --_stackSize;

        if (_stackSize == 0)
        {
            sendBulk();
        }
    }
    else
    {
        if (_bulk.empty()) {
            _bulk.refreshTime();
        }

        _bulk.emplace_back(str);

        ++_currentNumber;
        ++_commandsCounter;

        if (_stackSize == 0 && _currentNumber == _bulkSize)
        {
            sendBulk();
        }
    }
}

void Worker::operator ()(const Bulk &bulk)
{
    for (const auto &cmd : bulk)
    {
        std::cout << cmd << " ";
    }

    std::cout << std::endl;
}

ImpWorker::~ImpWorker() {}

void MTWorker::printHelper(size_t &commandsCounter, size_t &bulkCounter)
{
    std::unique_lock<std::mutex> lk(_cvMutex);
    _cv.wait(lk, [this](){ return !_bulksToPrint.empty() || !isRun; });

    if (isRun)
    {
        auto bulk = _bulksToPrint.front();
        _bulksToPrint.pop();

        std::cout << "bulk: ";

        for (auto cmd : bulk)
        {
            std::cout << cmd << " ";
            ++commandsCounter;
        }

        std::cout << std::endl;

        lk.unlock();

        ++bulkCounter;

        std::thread(&MTWorker::printHelper, this, std::ref(commandsCounter), std::ref(bulkCounter)).detach();
    }
    else
    {
        lk.unlock();
    }
}

void MTWorker::logHelper(size_t &commandsCounter, size_t &bulkCounter)
{
    std::unique_lock<std::mutex> lk(_cvMutex);
    _cv.wait(lk, [this](){ return !_bulksToLog.empty() || !isRun; });

    if (isRun)
    {
        auto bulk = _bulksToLog.front();
        _bulksToLog.pop();

        unsigned int counter = 0;
        char buffer[128];
        std::string fileName;

        do{
            sprintf(buffer, "bulk_%.2ld_%li_%.4d.log", _id, bulk.beginTime(), ++counter);
            fileName = buffer;
        }while (fexists(fileName));

        std::ofstream logFile;

        logFile.open(fileName, std::ios_base::app);

        lk.unlock();


        logFile << "bulk: ";

        for (auto cmd : bulk)
        {
            logFile << cmd << " ";
            ++commandsCounter;
        }

        logFile << std::endl;

        logFile.close();

        ++bulkCounter;

        std::thread(&MTWorker::logHelper, this, std::ref(commandsCounter), std::ref(bulkCounter)).detach();
    }
    else
    {
        lk.unlock();
    }
}

size_t MTWorker::get_id()
{
    static size_t counter;
    return  ++counter;
}

MTWorker::MTWorker() :
    _id(MTWorker::get_id()),
    isRun(true),
    _printCommandsCounter(0),
    _printBulkCounter(0),
    _logCommandsCouter({0}),
    _logBulksCouter({0})
{

    for (size_t i = 0; i < LOG_THREADS_NUM; ++i)
    {
        std::thread(&MTWorker::logHelper, this, std::ref(_logCommandsCouter[i]), std::ref(_logBulksCouter[i])).detach();
    }

    std::thread(&MTWorker::printHelper, this, std::ref(_printCommandsCounter), std::ref(_printBulkCounter)).detach();
}

MTWorker::~MTWorker()
{
    isRun = false;

    _cv.notify_all();

    std::cout << "PrintThread\n"
              << "\tCommands: " << _printCommandsCounter << "\n"
              << "\tBulks:    " << _printBulkCounter << std::endl;

    for (size_t i = 0; i < LOG_THREADS_NUM; ++i)
    {
        std::cout << "LogThread #" << i + 1 << "\n"
                  << "\tCommands: " << _logCommandsCouter[i] << "\n"
                  << "\tBulks:    " << _logBulksCouter[i] << std::endl;
    }
}

void MTWorker::operator ()(const Bulk &bulk)
{
    {
        std::lock_guard<std::mutex> lk(_cvMutex);

        _bulksToLog.emplace(bulk);
        _bulksToPrint.emplace(bulk);
    }

    _cv.notify_all();
}

size_t MTWorker::bulksPrinted()
{
    return _printBulkCounter;
}

time_t Bulk::beginTime() const
{
    return _beginTime;
}

void Bulk::refreshTime()
{
    _beginTime = time(nullptr);
}
