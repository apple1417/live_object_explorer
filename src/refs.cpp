#include "pch.h"
#include "refs.h"

namespace live_object_explorer::refs {

namespace {

bool fake_has_snapshot = false;

}

bool has_snapshot(void) {
    return fake_has_snapshot;
}

void take_snapshot(void) {
    fake_has_snapshot = true;
}

void import_db(void) {}

void export_db(void) {}

}  // namespace live_object_explorer::refs
