#pragma once
#include "CommandParser.h"
#include <vector>
#include <thread>

class TestCommandParser : public CommandParser
{
public:
    TestCommandParser(size_t _bulk_size, const char* _data, size_t size)
    {
        bulk_size = _bulk_size;

        std::string str = "";
        for (size_t i = 0; i < size; i++)
        {
            if (_data[i] != '\n')
            {
                str += _data[i];
            }
            else
            {
                input(str);
            }
        }
    }

    void parse(Bulk& bulk) override
    {
        while (pos<data.size())
        {
            while (!bulk.is_ready())
            {
                auto str = data[pos];
                parse_cmd(str, bulk);
                ++pos;

                if (pos == data.size())
                    break;
            }
            ready(bulk);
        }
    }

private:
    void input(std::string& str) override
    {
        if (!str.empty())
        {
            data.push_back(str);
            str.clear();
        }
    }

    std::vector<std::string> data;
    size_t pos = 0;
};