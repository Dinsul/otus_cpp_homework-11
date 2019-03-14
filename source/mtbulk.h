#pragma once

#include <list>
#include <string>
#include <ctime>
#include <memory>

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#define LOG_THREADS_NUM 2


/*!
 * \brief Bulk булька
 */
class Bulk : public std::list<std::string>
{
    time_t _beginTime;                  //!< Время записи первого элемента

public:
    time_t beginTime() const;
    void refreshTime();
};

/*!
 * \brief ImpWorker интерфейс для выполнения работы.
 */
struct ImpWorker
{
    virtual ~ImpWorker();
    virtual void operator ()(const Bulk &bulk) = 0;
    virtual size_t bulksPrinted() = 0;
};

struct Worker : public ImpWorker
{
    void operator ()(const Bulk &bulk) override;
    size_t bulksPrinted() override {return 0;}
};

struct MTWorker : public ImpWorker
{
private:
    bool isRun;
    size_t _printCommandsCounter;
    size_t _printBulkCounter;

    std::array<size_t, LOG_THREADS_NUM> _logCommandsCouter;
    std::array<size_t, LOG_THREADS_NUM> _logBulksCouter;

    std::condition_variable _cv;
    std::mutex              _cvMutex;

    std::queue<Bulk> _bulksToLog;
    std::queue<Bulk> _bulksToPrint;

    void printHelper(size_t &commandsCounter, size_t &bulkCounter);
    void logHelper(size_t &commandsCounter, size_t &bulkCounter);

public:
    MTWorker();
    ~MTWorker() override;

    void operator ()(const Bulk &bulk) override;

    size_t bulksPrinted() override;
};

/*!
 * \brief The BulkController
 * класс содержащий логику работы с бульками и логику заполнения булек
 */
class BulkController
{
    Bulk        _bulk;

    ImpWorker  &_worker;
    size_t      _bulkSize;
    size_t      _currentNumber;
    size_t      _stackSize;
    std::string _signShiftUp;
    std::string _signShiftDown;

    size_t      _bulksCounter;
    size_t      _commandsCounter;
    size_t      _linesCounter;

    void sendBulk();

public:
    /*!
     * \brief BulkController конструктор
     * \param bulk указатель на бульку
     * \param commandsCount колличество команд в бульке
     */
    BulkController(size_t bulkSize, ImpWorker &worker);

    ~BulkController();

    /*!
     * \brief addString добавляет строку, если булька заполнена, выполняет соответствующие действия
     * \param str новая строка
     */
    void addString(const std::string &str);

    /*!
     * \brief signShiftUp возвращает подпись начала блока
     * \return сторока обозначающая начало блока
     */
    std::string signShiftUp() const;

    /*!
     * \brief signShiftDown возвращает подпись конца блока
     * \return сторока обозначающая конец блока
     */
    std::string signShiftDown() const;

    /*!
     * \brief setSignShiftUp метод установки подписи начала блока
     * \param signShiftUp сторока обозначающая начало блока
     */
    void setSignShiftUp(const std::string &signShiftUp);

    /*!
     * \brief setSignShiftDown метод установки подписи конца блока
     * \param signShiftDown сторока обозначающая конец блока
     */
    void setSignShiftDown(const std::string &signShiftDown);

    void waiteWorker();
};
