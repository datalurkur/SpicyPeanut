#include "globalstate.h"

GlobalState::GlobalState(): _dataCollectionEnabled(true) { }

bool GlobalState::getDataCollectionEnabled() { return _dataCollectionEnabled; }
void GlobalState::setDataCollectionEnabled(bool enabled) { _dataCollectionEnabled = enabled; }