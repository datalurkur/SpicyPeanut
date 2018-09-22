#ifndef _GLOBALSTATE_H_
#define _GLOBALSTATE_H_

class GlobalState
{
public:
    GlobalState();

    bool getDataCollectionEnabled();
    void setDataCollectionEnabled(bool enabled);

private:
    bool _dataCollectionEnabled;
};

#endif
