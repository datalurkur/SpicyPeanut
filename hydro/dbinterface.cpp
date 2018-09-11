#include "dbinterface.h"
#include "log.h"

#include <fstream>

std::shared_ptr<DBInterface> DBInterface::Instance = nullptr;

void DBInterface::Init()
{
    if (Instance != nullptr) { return; }
    Instance = std::make_shared<DBInterface>();
}

DBInterface::DBInterface(): _connectionString(""), _session(nullptr)
{
    readConnectionString();
    connect();
}

DBInterface::~DBInterface()
{
    if (_session != nullptr)
    {
        _session->close();
    }
}

void DBInterface::readConnectionString()
{
    std::ifstream cStrInput(kConnectionStringPath, std::ios::in);
    if (!cStrInput.is_open())
    {
        LogError("Failed to open connection string file, no data will be logged");
    }

    cStrInput.seekg(0, std::ios::end);
    _connectionString.reserve((int)cStrInput.tellg());
    cStrInput.seekg(0, std::ios::beg);

    _connectionString.assign((std::istreambuf_iterator<char>(cStrInput)), std::istreambuf_iterator<char>());
    if (_connectionString.length() == 0)
    {
        LogError("Failed to read connection string data, no samples will be logged");
    }
}

bool DBInterface::connect()
{
    if (_connectionString.length() == 0) { return false; }
    try
    {
        if (_session != nullptr)
        {
            _session->close();
        }
        _session = std::make_shared<soci::session>(soci::odbc, _connectionString);
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
    for (int i = 0; i < kRetries; ++i)
    {
        try
        {
            (*_session) << "insert into sample (typeid, sampledata) values (:tp, :mm)", soci::use((int)type, "tp"), soci::use(measurement, "mm");
            break;
        }
        catch (std::exception const & e)
        {
            LogError("Failed to log data - " << e.what());
        }
        LogInfo("Attempting to reconnect and log (retry " << (i+1) << ")");
        connect();
    }
}