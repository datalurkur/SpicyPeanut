#include "dbinterface.h"
#include "log.h"

#include <fstream>

std::shared_ptr<DBInterface> DBInterface::Instance = nullptr;

void DBInterface::Init()
{
    if (Instance != nullptr) { return; }
    Instance = std::make_shared<DBInterface>();
}

DBInterface::DBInterface(): _session(nullptr)
{
    connect();
}

DBInterface::~DBInterface()
{
    if (_session != nullptr)
    {
        _session->close();
    }
}

bool DBInterface::connect()
{
    std::ifstream cStrInput(kConnectionStringPath, std::ios::in);
    if (!cStrInput.is_open())
    {
        LogError("Failed to open connection string file, no data will be logged");
        return false;
    }

    std::string connectionString;
    cStrInput.seekg(0, std::ios::end);
    connectionString.reserve((int)cStrInput.tellg());
    cStrInput.seekg(0, std::ios::beg);

    connectionString.assign((std::istreambuf_iterator<char>(cStrInput)), std::istreambuf_iterator<char>());
    if (connectionString.length() == 0)
    {
        LogError("Failed to read connection string data, no samples will be logged");
        return false;
    }

    try
    {
        _session = std::make_shared<soci::session>(soci::odbc, connectionString);
    }
    catch (std::exception const & e)
    {
        LogError("Failed to connect to database: " << e.what());
        return false;
    }

    return true;
}

void DBInterface::logSample(DBInterface::SampleType type, double measurement)
{
    if (_session == nullptr) { return; }
    try
    {
        (*_session) << "insert into sample (typeid, sampledata) values (:tp, :mm)", soci::use((int)type, "tp"), soci::use(measurement, "mm");
    }
    catch (std::exception const & e)
    {
        LogError("Failed to log sample data: " << e.what());
    }
}