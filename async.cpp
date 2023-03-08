#include "async.h"

#include "BulkPrinters.h"
#include "TestCommandParser.h"

#include <algorithm>

namespace async
{
    class BulkHandler
    {
    public:
        BulkHandler()
        {
            cbp = ConsoleBulkPrinter::create();
            fbp = FileBulkPrinter::create();
        }

        std::shared_ptr<Bulk> add_bulk(std::shared_ptr<Bulk> bulk)
        {
            bulks.push_back(bulk);
            cbp.get()->subscribe(bulk.get());
            fbp.get()->subscribe(bulk.get());
            return bulk;
        }

        void remove_bulk(std::shared_ptr<Bulk> bulk)
        {
            bulks.erase(std::remove(bulks.begin(), bulks.end(), bulk), bulks.end());
        }

    private:
        std::vector<std::shared_ptr<Bulk>> bulks;
        std::shared_ptr<ConsoleBulkPrinter> cbp;
        std::shared_ptr<FileBulkPrinter> fbp;
    };

    BulkHandler bulk_handler;

    handle_t connect(std::size_t bulk_size)
    {
        auto bulk_ptr = bulk_handler.add_bulk(std::make_shared<Bulk>(bulk_size));
        return (void*)std::move(bulk_ptr.get());
    }

    void receive(handle_t handle, const char* data, std::size_t size)
    {
        Bulk* bulk_ptr = static_cast<Bulk*>(handle);
        std::shared_ptr<TestCommandParser> cmdParser = std::make_shared<TestCommandParser>(bulk_ptr->get_size(), data, size);
        cmdParser->parse(*bulk_ptr);
    }

    void disconnect(handle_t handle)
    {
        Bulk* bulk_ptr = static_cast<Bulk*>(handle);
        bulk_handler.remove_bulk(std::make_shared<Bulk>(*bulk_ptr));
    }
}
