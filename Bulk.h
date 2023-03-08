#pragma once
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <chrono>

using chronoTime = std::chrono::time_point<std::chrono::system_clock>;
using BulkData = std::vector<std::string>;
using BulkDataTime = std::pair<BulkData, chronoTime>;

// Observer or Subscriber
class BulkPrinter
{
public:
    virtual ~BulkPrinter() = default;

    virtual void update(BulkDataTime data_time) = 0;
};

// Observable or Publisher
class Observable
{
public:
    virtual ~Observable() = default;

    // 1. Show ownership explicitly.
    virtual void subscribe(const std::shared_ptr<BulkPrinter>& obs) = 0;
};

class Bulk : public Observable
{
public:

    Bulk(size_t _size)
        : size(_size)
    {
        index = 0;
    }

    void subscribe(const std::shared_ptr<BulkPrinter>& obs) override
    {
        m_subs.emplace_back(obs);
    }

    void add_elem(const std::string& elem, const chronoTime& time, bool newElem)
    {
        if (m_data.empty())
        {
            m_time = time;
        }
        m_data.emplace_back(elem);
        if (newElem)
            ++index;
    }

    void ready(bool isReady)
    {
        if (isReady)
        {
            notify(std::make_pair(m_data, m_time));
            m_data.clear();
            index = 0;
        }
    }

    bool is_ready() const
    {
        return index == size;
    }

    size_t get_size() const
    {
        return size;
    }

private:

    void notify(const BulkDataTime& data_time)
    {
        auto iter = m_subs.begin();
        while (iter != m_subs.end())
        {
            auto ptr = iter->lock();
            if (ptr)
            {  // Если наблюдатель ещё существует, то оповещаем его
                ptr->update(data_time);
                ++iter;
            }
            else
            {  // Иначе - удаляем из списка наблюдателей.
                m_subs.erase(iter++);
            }
        }
    }

    // 1. Save weak_ptr
    // 2. Use std::list
    std::list<std::weak_ptr<BulkPrinter>> m_subs;

    BulkData m_data;
    chronoTime m_time;
    const size_t size;
    size_t index;
};