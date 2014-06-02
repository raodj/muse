#include "FSMAsyncHelper.h"
#include "FSHelper.h"

FSMAsyncHelper::FSMAsyncHelper(const QModelIndex &parent,
                               FSHelper *helper, const FSEntry &dir) :
    modelIndex(parent), fsHelper(helper), dir(dir) {
    Q_ASSERT(helper != NULL);
    // Nothing else to be done for now.
}

void
FSMAsyncHelper::run() {
    fsHelper->getEntries(dir);
    emit entriesLoaded(modelIndex);
}
