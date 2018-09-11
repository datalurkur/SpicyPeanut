#ifndef _DBINTERFACE_H_
#define _DBINTERFACE_H_

#include "soci.h"
#include "backends/odbc/soci-odbc.h"

#include <map>
#include <memory>

class DBInterface
{
public:
    const int kRetries = 3;

    // NOTE: These match the ids in the database.  Because I'm lazy.
    enum SampleType
    {
        temperature = 1,
        humidity,
        ph,
        ec
    };

    static std::shared_ptr<DBInterface> Instance;

    static void Init();

private:
    const std::string kConnectionStringPath = "connectionString.dat";

public:
    DBInterface();
    virtual ~DBInterface();

    void logSample(SampleType type, double measurement);

private:
    void readConnectionString();
    bool connect();

private:
    std::string _connectionString;
    std::shared_ptr<soci::session> _session;
};

#endif