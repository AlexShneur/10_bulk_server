#pragma once
#include <stack>

class BracketStorage
{
public:
    void push(BracketType bracket)
    {
        brackets.push(bracket);
    }

    void pop()
    {
        if (!empty())
            brackets.pop();
    }

    bool empty()
    {
        return brackets.empty();
    }
private:
    std::stack<BracketType> brackets;
};
