// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "mod_diagnostics.h"
#include "mods.h"
#include <fstream>
#include <openssl/evp.h>
namespace Eden::ModDiagnostics {
inline void InventoryFile(const std::filesystem::path& file) {
    std::error_code error;
    const auto size = std::filesystem::file_size(file, error);
    if (error) return;
    std::string text = file.string() + " bytes=" + std::to_string(size);
    if (size <= 64 * 1024 * 1024) {
        std::ifstream input(file, std::ios::binary);
        auto* digest = EVP_MD_CTX_new();
        bool ok = input.good() && digest && EVP_DigestInit_ex(digest, EVP_sha256(), nullptr) == 1;
        char buffer[32768];
        while (ok && input.read(buffer, sizeof(buffer)))
            ok = EVP_DigestUpdate(digest, buffer, sizeof(buffer)) == 1;
        if (ok && input.gcount()) ok = EVP_DigestUpdate(digest, buffer, input.gcount()) == 1;
        unsigned char bytes[EVP_MAX_MD_SIZE]; unsigned count = 0;
        ok = ok && !input.bad() && EVP_DigestFinal_ex(digest, bytes, &count) == 1;
        if (digest) EVP_MD_CTX_free(digest);
        text += ok ? " sha256=" : " sha256=unavailable";
        if (ok) for (unsigned i = 0; i < count; ++i) {
            static constexpr char hex[] = "0123456789abcdef";
            text += hex[bytes[i] >> 4]; text += hex[bytes[i] & 15];
        }
    } else text += " sha256=skipped-over-64MiB";
    Write("framework-file", text);
}
inline void Inventory(const std::string& global_mods, const std::string& game_folder,
                      const std::string& user, uint64_t title) {
    Write("inventory-scope", "External emulator patches only; frameworks bundled into the game are not listed here. SD pack folders are presence checks, not confirmation ARCropolis enabled or loaded them.");
    Write("inventory-paths", "global_mods=" + global_mods + " game_folder=" + game_folder +
        " sd_mods=" + user + "/sdmc/ultimate/mods");
    unsigned count = 0;
    for (const auto& root : {Mods::TitleFolder(global_mods, title),
                            game_folder.empty() ? std::string{} : game_folder + "/mods"}) {
        if (root.empty()) continue;
        for (const auto& mod : Mods::ListFolder(root)) {
            if (!Mods::IsFolder(mod)) continue;
            for (const char* part : {"exefs", "romfs/skyline/plugins"})
                for (const auto& file : Mods::ListFolder(mod.path() / part)) {
                    if (++count > 256) { Write("inventory", "framework limit reached", true); return; }
                    InventoryFile(file.path());
                }
        }
    }
    count = 0;
    for (const auto& pack : Mods::ListFolder(user + "/sdmc/ultimate/mods")) {
        if (++count > 256) { Write("inventory", "character directory limit reached"); break; }
        Write("character-pack", pack.path().filename().string());
    }
    Write("character-pack-count", std::to_string(count));
    Write("inventory-complete", "ARCropolis logs: " + user + "/sdmc/ultimate/arcropolis/logs", true);
}
}
