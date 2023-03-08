#pragma once
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Bulk.h"
#include "WaitingQueue.h"

#include <random>

using BulkDataQueue = WaitingQueue<BulkDataTime>;

class ConsoleBulkPrinter : public BulkPrinter, public std::enable_shared_from_this<BulkPrinter>
{
public:
    // Объект должен быть создан в динамической памяти.
    static std::shared_ptr<ConsoleBulkPrinter> create()
    {
        auto ptr = std::shared_ptr<ConsoleBulkPrinter>{ new ConsoleBulkPrinter{} };
        return ptr;
    }

    void update(BulkDataTime data_time) override
    {
        m_queue.push(data_time);
    }

    void subscribe(Bulk* bulk)
    {
        bulk->subscribe(shared_from_this());
    }

    ~ConsoleBulkPrinter()
    {
        while (!m_queue.empty());   //ожидаем завершения вывода данных

        m_queue.stop();
        join_threads();
    }

private:
    // Чтобы пользователи не создавали объект в стеке, сделаем конструктор закрытым.
    ConsoleBulkPrinter()
    {
        for (size_t i = 0; i < LOG_THREAD_COUNT; ++i)
        {
            auto log_thread = std::thread(&ConsoleBulkPrinter::print_bulk, this,
                std::ref(m_queue));
            thread_pool.emplace_back(std::move(log_thread));
        }
    }

    void print_bulk(BulkDataQueue& queue)
    {
        std::pair<BulkData, chronoTime> data_time;
        while (queue.pop(data_time))
        {
            BulkData& data = data_time.first;
            if (!data.empty())
            {
                std::cout << "bulk: ";
                for (auto it = begin(data); it != end(data); ++it)
                {
                    if (it == (end(data) - 1))
                        std::cout << *it << std::endl;
                    else
                        std::cout << *it << ", ";
                }
            }
        }
    }

    void join_threads()
    {
        for (size_t i = 0; i < LOG_THREAD_COUNT; ++i)
        {
            if (thread_pool[i].joinable())
                thread_pool[i].join();
        }
    }

    BulkDataQueue m_queue;
    std::vector<std::thread> thread_pool;
    const size_t LOG_THREAD_COUNT = 1;
};

class FileBulkPrinter : public BulkPrinter, public std::enable_shared_from_this<BulkPrinter>
{
public:
    // Объект должен быть создан в динамической памяти.
    static std::shared_ptr<FileBulkPrinter> create()
    {
        auto ptr = std::shared_ptr<FileBulkPrinter>{ new FileBulkPrinter{} };
        return ptr;
    }

    void update(BulkDataTime data_time) override
    {
        m_queue.push(data_time);
    }

    void subscribe(Bulk* bulk)
    {
        bulk->subscribe(shared_from_this());
    }

    ~FileBulkPrinter()
    {
        while (!m_queue.empty());   //ожидаем завершения вывода данных

        m_queue.stop();
        join_threads();
    }

private:
    // Чтобы пользователи не создавали объект в стеке, сделаем конструктор закрытым.
    FileBulkPrinter()
    {
        for (size_t i = 0; i < FILE_THREAD_COUNT; ++i)
        {
            auto file_thread = std::thread(&FileBulkPrinter::print_bulk, this,
                std::ref(m_queue));
            thread_pool.emplace_back(std::move(file_thread));
        }
    }

    void print_bulk(BulkDataQueue& queue)
    {
        std::pair<BulkData, chronoTime> data_time;
        while (queue.pop(data_time))
        {
            BulkData& data = data_time.first;
            if (!data.empty())
            {
                auto time_t = std::chrono::system_clock::to_time_t(data_time.second);
                std::mt19937 rng(dev());
                std::uniform_int_distribution<std::mt19937::result_type> dist6(1, UINT32_MAX);
                std::ofstream file("bulk" + std::to_string(time_t) +"_id"+ thread_id_string(std::this_thread::get_id()) +"_"+ std::to_string(dist6(rng)) + ".log");
                file << "bulk: ";
                for (auto it = begin(data); it != end(data); ++it)
                {
                    if (it == (end(data) - 1))
                        file << *it << std::endl;
                    else
                        file << *it << ", ";
                }
                file.close();
            }
        }
    }

    std::string thread_id_string(const std::thread::id& id) 
    {
        std::stringstream ss;
        ss << id;
        return ss.str();
    }

    void join_threads()
    {
        for (size_t i = 0; i < FILE_THREAD_COUNT; ++i)
        {
            if (thread_pool[i].joinable())
                thread_pool[i].join();
        }
    }

    BulkDataQueue m_queue;
    std::vector<std::thread> thread_pool;
    const size_t FILE_THREAD_COUNT = 2;

    std::random_device dev;
};