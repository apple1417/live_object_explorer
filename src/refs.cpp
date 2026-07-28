#include "pch.h"
#include "refs.h"

#ifdef __clang__
// Sqlite, detecting `_MSC_VER`, tries to use `__int64`, which is an MSVC extension
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif

#include "sqlite3.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace live_object_explorer::refs {

namespace {

template <typename F>
struct RaiiLambda {
    F func;

    [[nodiscard]] RaiiLambda(F&& func) : func(std::move(func)) {}
    ~RaiiLambda() { func(); }

    RaiiLambda(const RaiiLambda&) = delete;
    RaiiLambda& operator=(const RaiiLambda&) = delete;
    RaiiLambda(RaiiLambda&&) = delete;
    RaiiLambda& operator=(RaiiLambda&&) = delete;
};

std::shared_ptr<sqlite3> database{};

/**
 * @brief Opens a new database connection.
 *
 * @param filename The filename to open.
 * @return The database, or null on error.
 */
std::shared_ptr<sqlite3> open_db(const char* filename) {
    sqlite3* new_db = nullptr;
    auto res = sqlite3_open(filename, &new_db);
    if (res != SQLITE_OK) {
        LOG(ERROR, "Failed to open database at {}: {}", filename, sqlite3_errstr(res));
        sqlite3_close(new_db);
        return nullptr;
    }
    return std::shared_ptr<sqlite3>{new_db, [](sqlite3* db_to_close) {
                                        LOG(INFO, "closing");
                                        auto res = sqlite3_close(db_to_close);
                                        if (res != SQLITE_OK) {
                                            LOG(ERROR, "Failed to close database: {}",
                                                sqlite3_errstr(res));
                                        }
                                    }};
}

/**
 * @brief Gets the path to the local db, for import/exports.
 *
 * @return The path to the local db.
 */
std::filesystem::path get_local_db_path(void) {
    return unrealsdk::utils::get_this_dll().parent_path() / "live_object_explorer_refs.sqlite3";
}

}  // namespace

bool has_snapshot(void) {
    return database != nullptr;
}

void take_snapshot(void) {
    database = open_db(":memory:");

    // TODO: probably want a helper
    sqlite3_exec(database.get(), "PRAGMA foreign_keys = ON", nullptr, nullptr, nullptr);

    char* error = nullptr;
    auto ret = sqlite3_exec(database.get(), R"==(
        CREATE TABLE Objects (
            Pointer     INTEGER NOT NULL UNIQUE,
            Name        TEXT NOT NULL,
            PRIMARY KEY(Pointer)
        ) STRICT;

        CREATE TABLE Refs (
            FromPointer INTEGER NOT NULL UNIQUE,
            ToPointer   INTEGER NOT NULL UNIQUE,
            FOREIGN KEY(FromPointer) REFERENCES Objects(Pointer),
            FOREIGN KEY(ToPointer) REFERENCES Objects(Pointer),
            UNIQUE(FromPointer, ToPointer)
        ) STRICT;
    )==",
                            nullptr, nullptr, &error);
    if (ret != SQLITE_OK) {
        LOG(ERROR, "Sqlite exec failed: {}", sqlite3_errstr(ret));
        if (error != nullptr) {
            LOG(ERROR, "{}", error);
        }
    }
}

void import_db(void) {
    if (!std::filesystem::exists(get_local_db_path())) {
        return;
    }

    // If we don't already have a db, create a new one
    bool wipe_db_on_exit = false;
    if (database == nullptr) {
        wipe_db_on_exit = true;
        database = open_db(":memory:");
        if (database == nullptr) {
            return;
        }
    }
    // If we made a new db, and we get an error, then delete it
    const RaiiLambda raii1{[&]() {
        if (wipe_db_on_exit) {
            database = nullptr;
        }
    }};

    auto import_db = open_db(get_local_db_path().string().c_str());
    if (import_db == nullptr) {
        return;
    }

    auto backup = sqlite3_backup_init(database.get(), "main", import_db.get(), "main");
    if (backup == nullptr) {
        LOG(ERROR, "Failed to create backup object: {}", sqlite3_errmsg(database.get()));
        return;
    }
    const RaiiLambda raii2{[&]() {
        auto ret = sqlite3_backup_finish(backup);
        if (ret != SQLITE_OK) {
            LOG(ERROR, "Failed to free backup object: {}", sqlite3_errstr(ret));
        }
    }};

    auto ret = sqlite3_backup_step(backup, -1);
    if (ret != SQLITE_DONE) {
        LOG(ERROR, "Failed to export database: {}", sqlite3_errmsg(database.get()));
    }

    // Successfully loaded, don't wipe
    wipe_db_on_exit = false;
}

void export_db(void) {
    if (database == nullptr) {
        return;
    }

    auto export_db = open_db(get_local_db_path().string().c_str());
    if (export_db == nullptr) {
        return;
    }

    auto backup = sqlite3_backup_init(export_db.get(), "main", database.get(), "main");
    if (backup == nullptr) {
        LOG(ERROR, "Failed to create backup object: {}", sqlite3_errmsg(export_db.get()));
        return;
    }
    const RaiiLambda raii{[&]() {
        auto ret = sqlite3_backup_finish(backup);
        if (ret != SQLITE_OK) {
            LOG(ERROR, "Failed to free backup object: {}", sqlite3_errstr(ret));
        }
    }};

    auto ret = sqlite3_backup_step(backup, -1);
    if (ret != SQLITE_DONE) {
        LOG(ERROR, "Failed to export database: {}", sqlite3_errmsg(export_db.get()));
        return;
    }
}

}  // namespace live_object_explorer::refs
