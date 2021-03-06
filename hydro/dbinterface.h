#ifndef _DBINTERFACE_H_
#define _DBINTERFACE_H_

#include "soci.h"
#include "odbc/soci-odbc.h"

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
        ec,
        tds
    };

    static std::shared_ptr<DBInterface> Instance;

    static void Init();

private:
#if _WINDOWS_BUILD
    const std::string kConnectionStringPath = "connectionString.dat";
#else
    const std::string kConnectionStringPath = "/etc/spicyd.conf";
#endif

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
