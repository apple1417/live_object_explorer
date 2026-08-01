#include "pch.h"
#include "refs.h"
#include "refs_searcher.h"

#ifdef __clang__
// Sqlite, detecting `_MSC_VER`, tries to use `__int64`, which is an MSVC extension
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif

#include "sqlite3.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if !defined(NDEBUG) && defined(_MSC_VER)
// NOLINTNEXTLINE(cppcoreguielines-macro-usage)
#define BREAKPOINT() __debugbreak()
#else
#define BREAKPOINT()
#endif

using namespace unrealsdk::unreal;

namespace live_object_explorer::refs {

uint32_t num_threads = []() {
    uint32_t num = std::thread::hardware_concurrency();
    if (num == 0) {
        // May return 0 if not supported.
        // Currently steam hardware survey says 6 and 8 cores are ~25% each, so lets go with the
        // better of the two.
        // NOLINTNEXTLINE(readability-magic-numbers)
        num = 8;
    }
    return num;
}();

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
        BREAKPOINT();
        sqlite3_close(new_db);
        return nullptr;
    }
    return std::shared_ptr<sqlite3>{new_db, [](sqlite3* db_to_close) {
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

/**
 * @brief Wipes and creates a new database.
 *
 * @return True if sucessfully created, false on any error.
 */
bool create_new_db(void) {
    database = open_db("");

    auto exec = [](const char* query) {
        char* error = nullptr;
        auto ret = sqlite3_exec(database.get(), query, nullptr, nullptr, &error);
        if (ret != SQLITE_OK) {
            LOG(ERROR, "Sqlite exec failed: {}", sqlite3_errstr(ret));
            BREAKPOINT();
            if (error != nullptr) {
                LOG(ERROR, "{}", error);
                sqlite3_free(error);
            }
            return false;
        }
        return true;
    };

    // Keep foreign_keys in a seperate statement to be safe
    return exec("PRAGMA foreign_keys = ON") && exec(R"==(
        CREATE TABLE Objects (
            Pointer     INTEGER NOT NULL UNIQUE,
            Name        TEXT,
            PRIMARY KEY(Pointer)
        ) STRICT;

        CREATE TABLE Refs (
            FromPointer INTEGER NOT NULL,
            ToPointer   INTEGER NOT NULL,
            FOREIGN KEY(FromPointer) REFERENCES Objects(Pointer),
            FOREIGN KEY(ToPointer) REFERENCES Objects(Pointer),
            UNIQUE(FromPointer, ToPointer)
        ) STRICT;
    )==");
}

/**
 * @brief Prepares a (long term) sqlite query.
 *
 * @param query The query to prepare.
 * @return A pointer to the prepared statement, or null on error.
 */
std::shared_ptr<sqlite3_stmt> prepare_statement(std::string_view query) {
    sqlite3_stmt* raw_statement = nullptr;
    auto res = sqlite3_prepare_v3(database.get(), query.data(), static_cast<int>(query.size() + 1),
                                  SQLITE_PREPARE_PERSISTENT, &raw_statement, nullptr);
    if (res != SQLITE_OK) {
        LOG(ERROR, "Failed to prepare statement: {}", sqlite3_errmsg(database.get()));
        BREAKPOINT();
        return {nullptr};
    }
    return {raw_statement, sqlite3_finalize};
};

// We need one prepared statement per thread. Rather than actually deal with statements, I'd prefer
// lambdas, with raii cleanup - so make some factory functions that return a lambda.

/**
 * @brief Creates a lambda to insert an object name.
 *
 * @return The lambda, or null on failure.
 */
std::function<void(UObject*)> create_insert_object_lambda(void) {
    auto upsert_object_statement = prepare_statement(R"==(
        INSERT INTO
            Objects (Pointer, Name)
        VALUES
            (:pointer, :name)
        ON CONFLICT(Pointer) DO UPDATE SET
            Name = :name
    )==");

    if (upsert_object_statement == nullptr) {
        return nullptr;
    }

    return [upsert_object_statement](UObject* obj) {
        if (upsert_object_statement == nullptr || obj == nullptr) {
            return;
        }
        sqlite3_reset(upsert_object_statement.get());

        auto pointer = static_cast<sqlite_int64>(reinterpret_cast<intptr_t>(obj));
        auto name = obj->get_path_name();

        auto res = sqlite3_bind_int64(upsert_object_statement.get(), 1, pointer);
        if (res != SQLITE_OK) {
            LOG(ERROR, "Failed to bind 'object' in 'upsert object' query: {}", sqlite3_errstr(res));
            BREAKPOINT();
            return;
        }

        static_assert(sizeof(wchar_t) == sizeof(char16_t));
        res = sqlite3_bind_text16(upsert_object_statement.get(), 2, name.c_str(),
                                  static_cast<int>(name.size() * sizeof(wchar_t)),
                                  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                                  SQLITE_TRANSIENT);
        if (res != SQLITE_OK) {
            LOG(ERROR, "Failed to bind 'name' in 'upsert object' query: {}", sqlite3_errstr(res));
            BREAKPOINT();
            return;
        }

        res = sqlite3_step(upsert_object_statement.get());
        if (res != SQLITE_DONE) {
            LOG(ERROR, "Failed to step 'upsert object' query: {}", sqlite3_errmsg(database.get()));
            BREAKPOINT();
            return;
        }
    };
}

/**
 * @brief Creates a lambda to insert an object reference.
 *
 * @return The lambda, or null on failure.
 */
refs_callback create_insert_ref_lambda(void) {
    // We know the from object must already have been inserted, so only need to add the to object
    auto insert_object_statement = prepare_statement(R"==(
        INSERT OR IGNORE INTO
            Objects (Pointer)
        VALUES
            (:to)
        ON CONFLICT(Pointer) DO NOTHING
    )==");
    if (insert_object_statement == nullptr) {
        return nullptr;
    }
    auto insert_ref_statement = prepare_statement(R"==(
        INSERT INTO
            Refs (FromPointer, ToPointer)
        VALUES
            (:from, :to)
        ON CONFLICT(FromPointer, ToPointer) DO NOTHING
    )==");
    if (insert_ref_statement == nullptr) {
        return nullptr;
    }

    return [insert_object_statement, insert_ref_statement](UObject* from_obj, UObject* to_obj) {
        if (insert_object_statement == nullptr || insert_ref_statement == nullptr
            || from_obj == nullptr || to_obj == nullptr) {
            return;
        }
        sqlite3_reset(insert_object_statement.get());
        sqlite3_reset(insert_ref_statement.get());

        auto from_pointer = static_cast<sqlite_int64>(reinterpret_cast<intptr_t>(from_obj));
        auto to_pointer = static_cast<sqlite_int64>(reinterpret_cast<intptr_t>(to_obj));

        auto res = sqlite3_bind_int64(insert_ref_statement.get(), 1, from_pointer);
        if (res != SQLITE_OK) {
            LOG(ERROR, "Failed to bind 'from object' in 'insert ref' query: {}",
                sqlite3_errstr(res));
            BREAKPOINT();
            return;
        }

        res = sqlite3_bind_int64(insert_object_statement.get(), 1, to_pointer);
        if (res != SQLITE_OK) {
            LOG(ERROR, "Failed to bind 'to object' in 'insert object' query: {}",
                sqlite3_errstr(res));
            BREAKPOINT();
            return;
        }
        res = sqlite3_bind_int64(insert_ref_statement.get(), 2, to_pointer);
        if (res != SQLITE_OK) {
            LOG(ERROR, "Failed to bind 'to object' in 'insert ref' query: {}", sqlite3_errstr(res));
            BREAKPOINT();
            return;
        }

        res = sqlite3_step(insert_object_statement.get());
        if (res != SQLITE_DONE) {
            LOG(ERROR, "Failed to step 'insert object' query: {}", sqlite3_errmsg(database.get()));
            BREAKPOINT();
            return;
        }

        res = sqlite3_step(insert_ref_statement.get());
        if (res != SQLITE_DONE) {
            LOG(ERROR, "Failed to step 'insert ref' query: {}", sqlite3_errmsg(database.get()));
            BREAKPOINT();
            return;
        }
    };
}

}  // namespace

bool has_snapshot(void) {
    return database != nullptr;
}

void take_snapshot(void) {
    if (!create_new_db()) {
        database = nullptr;
        return;
    }

    // Some misc setup before we stop the world

    std::vector<std::thread> threads{};
    threads.reserve(num_threads);

    auto gobjects = unrealsdk::gobjects();

    // Stop the world. While checking refs we can't let the game (or anything else) mess with them.
    const unrealsdk::utils::ThreadSuspender suspend{};

    // Chunk gobjects per thread
    auto num_objects = gobjects.size();
    // Round up to make sure we don't miss anything, the last thread will do less
    auto objects_per_thread = ((num_objects - 1) / num_threads) + 1;

    for (size_t start_idx = 0; start_idx < num_objects; start_idx += objects_per_thread) {
        auto end_idx = std::min(start_idx + objects_per_thread, num_objects);
        threads.emplace_back([start_idx, end_idx, &gobjects]() {
            // Need to create a seperate prepared starement/lambda on each thread
            auto insert_object = create_insert_object_lambda();
            if (insert_object == nullptr) {
                return;
            }
            auto insert_ref = create_insert_ref_lambda();
            if (insert_ref == nullptr) {
                return;
            }

            // Iterate through all objects
            for (auto i = start_idx; i < end_idx; i++) {
                UObject* obj = nullptr;
                try {
                    obj = gobjects.obj_at(i);
                } catch (const std::out_of_range&) {
                    continue;
                }
                if (obj == nullptr) {
                    continue;
                }

                insert_object(obj);
                search_for_refs(obj, insert_ref);
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
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
        database = open_db("");
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
        BREAKPOINT();
        return;
    }
    const RaiiLambda raii2{[&]() {
        auto ret = sqlite3_backup_finish(backup);
        if (ret != SQLITE_OK) {
            LOG(ERROR, "Failed to free backup object: {}", sqlite3_errstr(ret));
            BREAKPOINT();
        }
    }};

    auto ret = sqlite3_backup_step(backup, -1);
    if (ret != SQLITE_DONE) {
        LOG(ERROR, "Failed to export database: {}", sqlite3_errmsg(database.get()));
        BREAKPOINT();
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
        BREAKPOINT();
        return;
    }
    const RaiiLambda raii{[&]() {
        auto ret = sqlite3_backup_finish(backup);
        if (ret != SQLITE_OK) {
            LOG(ERROR, "Failed to free backup object: {}", sqlite3_errstr(ret));
            BREAKPOINT();
        }
    }};

    auto ret = sqlite3_backup_step(backup, -1);
    if (ret != SQLITE_DONE) {
        LOG(ERROR, "Failed to export database: {}", sqlite3_errmsg(export_db.get()));
        BREAKPOINT();
        return;
    }
}

}  // namespace live_object_explorer::refs
