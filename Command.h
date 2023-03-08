#pragma once

enum BracketType
{
    open = '{',
    close = '}'
};

enum CommandType
{
    command,
    bracket
};

class Command
{
public:
    Command(const std::string& cmd)
    {
        if (cmd[0] == BracketType::open)
        {
            type = CommandType::bracket;
            bracket = BracketType::open;
        }
        else if (cmd[0] == BracketType::close)
        {
            type = CommandType::bracket;
            bracket = BracketType::close;
        }
        else
        {
            type = CommandType::command;
            data = cmd;
        }
    }
    auto get_type() const
    {
        return type;
    }

    auto get_data() const
    {
        return data;
    }

    auto get_bracket() const
    {
        return bracket;
    }

private:
    CommandType type;
    std::string data;
    BracketType bracket;
};